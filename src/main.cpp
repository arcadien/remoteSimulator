/*
 * RemoteSimulator
 *
 * This firmware allow to trigger the emission of a 433Mhz signal when an event
 * occurs. The send signal is a on/off signal, so that this firmware is not
 * suitable for temperature or other discrete data. It is suited for sound,
 * light or movment detection. The trigger sensor can be active low or high.
 * Two different definitions are used to select one mode or another.
 * The firmware also monitors the power level, so that a low battery level will
 * make the LED signal to blink each 8 seconds, when the MCU wakes up from sleep
 * mode.
 *
 * Technical details:
 * - The AVR watchdog is used for deep sleep management.
 * - Battery level is evaluated comparing Vcc with internal 1V1 reference
 * - SoftwareSerial libray is useable for debugging or send serial data on PB0
 *
 *  Original work from:
 *  http://blog.onlinux.fr/detecteur-de-choc-tx-433mhz-pilotes-avec-attiny85/
 *
 *
 *                  +-\/-+
 * Reset      PB5  1|    |8  Vcc
 * RF433 TX - PB3  2|    |7  PB2 - Trigger pin (active low) if USE_LOW_INT0
 * LED +pin - PB4  3|    |6  PB1 - Trigger pin (active high) if USE_HIGH_PCINT1
 *            GND  4|    |5  PB0 - Serial TX (trace)
 */
#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <x10rf.h>
#include <RCSwitch.h>
#include <TinySoftwareSerial.h>

#if !defined(USE_HIGH_PCINT1) and !defined(USE_LOW_INT0)
#define USE_LOW_INT0
#endif

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#ifndef bit_is_set
#define bit_is_set(sfr, bit) (_SFR_BYTE(sfr) & _BV(bit))
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifdef USE_DOUBLE_TRIGGER_FOR_OFF
bool f_is_first_trigger = true;
volatile bool f_second_trigger_overtime = false;
#endif

// How many watchdog interrupt before battery sensing
#define WDT_COUNT_BEFORE_CURRENT_SENSING 8 // each 64 seconds (3600 / 8)
volatile uint16_t current_wdt_count = 0;

volatile boolean f_wdt = 0;
volatile boolean f_int = 0;
volatile boolean lowBattery = 0;

// real 1v1 ref value, in mv
const uint32_t REF_1V1 = 1048;

const byte TX_PIN = PB3; // Pin number for the 433mhz OOK transmitter
const byte LED_PIN = PB4;
const byte WAKEUP_PIN = PB2;

RCSwitch mySwitch = RCSwitch();

#define COMMAND_REPEAT_COUNT 3
#define rfxSensorID 0x0E 
x10rf x10 = x10rf(TX_PIN, 0, COMMAND_REPEAT_COUNT);

/*
 * Blink led at LED_PIN `blinkCount` times
 */
void blink(int blinkCount) {
  pinMode(LED_PIN, OUTPUT);
  for (byte i = blinkCount; i > 0; i--) {
    digitalWrite(LED_PIN, HIGH);
    delayMicroseconds(100000);
    digitalWrite(LED_PIN, LOW);
    delayMicroseconds(100000);
  }
}


void lowBatteryWarning() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  delayMicroseconds(10000);

  pinMode(LED_PIN, INPUT);
}

long readVcc(void) {
  uint16_t result;

  power_adc_enable();

  ADMUX = 0;
  sbi(ADMUX, MUX3);
  sbi(ADMUX, MUX2);
  sbi(ADCSRA, ADEN);
  delayMicroseconds(2000);

  uint16_t accumulator = 0;

  // we will discard 10 first reads,
  // then mean 16 reads
  for (uint8_t i = 0; i < (10 + 16); ++i) {
    sbi(ADCSRA, ADSC);
    while (bit_is_set(ADCSRA, ADSC)) {}
    if (i > 9) {
      accumulator += ADC;
    }
  }
  result = (accumulator / 16);

  // Back-calculate AVcc in mV
  long millivolts = ((uint32_t)(1024 * REF_1V1)) / result;

  Serial.print("Current Voltage:");
  Serial.print(millivolts);
  Serial.println("mV");

  // switch Analog to Digitalconverter OFF
  cbi(ADCSRA, ADEN);
  power_adc_disable();

  return millivolts;
}

void arm_interrupt() {
  cli();
#ifdef USE_HIGH_PCINT1
  // PB1 high using PCINT1
  sbi(GIMSK, PCIE);
  sbi(PCMSK, PCINT1);
#endif
#ifdef USE_LOW_INT0
  // PB2 low using INT0
  sbi(GIMSK, INT0);
  cbi(MCUCR, ISC00);
  cbi(MCUCR, ISC01);
#endif
  sei();
}

// set system into the sleep state
// system wakes up when watchdog times out
void system_sleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sei();
  sleep_mode();
  sleep_disable();
}

void setup_watchdog1s() {
  wdt_reset();
  WDTCR = 0;
  WDTCR |= (1 << WDE) | (1 << WDIE) | (1 << WDP2) | (1 << WDP1); // 1s timeout
}

void setup_watchdog8s() {
  wdt_reset();
  WDTCR = 0;
  WDTCR |= (1 << WDE) | (1 << WDIE) | (1 << WDP3) | (1 << WDP0); // 8s timeout
}

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  if (current_wdt_count >= WDT_COUNT_BEFORE_CURRENT_SENSING) {
    f_wdt = true; // set WDTl flag
    current_wdt_count = 0;
  }
  current_wdt_count++;
#ifdef USE_DOUBLE_TRIGGER_FOR_OFF
  f_second_trigger_overtime = true;
#endif
}

ISR(BADISR_vect) { blink(20); }

#ifdef USE_HIGH_PCINT1
ISR(PCINT0_vect) {
  if (bit_is_set(PINB, PB1)) {
    cbi(GIMSK, PCIE);
    f_int = true;
  }
}
#endif

#ifdef USE_LOW_INT0
ISR(INT0_vect) {
  cbi(GIMSK, INT0);
  f_int = true;
}
#endif

void printConfigurationInformations() {
  Serial.println("REMOTESIMULATOR");
  Serial.print("GIT: ");
  Serial.println(TOSTRING(GIT_TAG));
  Serial.print("SWITCH_FAMILY: ");
  Serial.println(*TOSTRING(SWITCH_FAMILY));
  Serial.print("SWITCH_GROUP: ");
  Serial.println(SWITCH_GROUP);
  Serial.print("SWITCH_NUMBER: ");
  Serial.println(SWITCH_NUMBER);
  Serial.print("LOW BATTERY VOLTAGE: ");
  Serial.println(LOW_BATTERY_VOLTAGE);
  Serial.print("USE DOUBLE TRIGGER FOR OFF: ");
  Serial.println(*TOSTRING(USE_DOUBLE_TRIGGER_FOR_OFF));
  Serial.print("rfxSensorID:");
  Serial.println(rfxSensorID);

  delayMicroseconds(100000);
}

void setup() {
  setup_watchdog8s();

  pinMode(TX_PIN, OUTPUT);
  pinMode(WAKEUP_PIN, INPUT);
  mySwitch.enableTransmit(TX_PIN);

  // power_timer0_disable();
  // power_timer1_disable();
  // power_usi_disable();

  // UART support, TX only. See
  // https://github.com/SpenceKonde/ATTinyCore/blob/master/avr/extras/ATtiny_x5.md
  Serial.begin(9600);
  ACSR &= ~(1 << ACIE);
  ACSR |= ~(1 << ACD);

  printConfigurationInformations();

  lowBattery =
      !(readVcc() >= LOW_BATTERY_VOLTAGE); // Initialize battery level value

  blink(2);
}

void EmitOnCommand() {
  Serial.println("Emit On");
  blink(1);
  digitalWrite(LED_PIN, HIGH);
  mySwitch.switchOn(*TOSTRING(SWITCH_FAMILY), SWITCH_GROUP, SWITCH_NUMBER);
  digitalWrite(LED_PIN, LOW);
  delayMicroseconds(10000);
}

void EmitOffCommand() {
  Serial.println("Emit off");
  blink(2);
  digitalWrite(LED_PIN, HIGH);
  mySwitch.switchOff(*TOSTRING(SWITCH_FAMILY), SWITCH_GROUP, SWITCH_NUMBER);
  digitalWrite(LED_PIN, LOW);
  delayMicroseconds(10000);
}

/*!
 * If USE_HIGH_PCINT1, ensure PB3 is high otherwise do not trigger any action
 */
void loop() {

  wdt_reset();

  if (lowBattery) {
    // Flash for 1ms every 8s
    // http://www.gammon.com.au/power
    lowBatteryWarning();
    Serial.println("Low battery");
  }

#ifdef USE_DOUBLE_TRIGGER_FOR_OFF

  // 1s watchdog timed out before a second
  // trigger occurs, send a "ON" command
  if (!f_is_first_trigger && f_second_trigger_overtime) {
    Serial.println("overtime");
    setup_watchdog8s();
    f_is_first_trigger = true;
    f_second_trigger_overtime = false;
    EmitOnCommand();
  }

#endif

  if (f_int) {
    f_int = false;

#ifdef USE_DOUBLE_TRIGGER_FOR_OFF

    if (f_is_first_trigger) {
      Serial.println("First trigger");
      setup_watchdog1s();
      delay(100);
      f_is_first_trigger = false;
    } else {
      Serial.println("Second trigger");
      setup_watchdog8s();
      f_is_first_trigger = true;
      f_second_trigger_overtime = false;
      EmitOffCommand();
    }
#else
    EmitOnCommand();
    setup_watchdog8s();
#endif

  } else if (f_wdt) {
    Serial.println("Current sensing");
    long rawBattery = readVcc();
    
    lowBattery = (rawBattery <= LOW_BATTERY_VOLTAGE);
    digitalWrite(LED_PIN, HIGH);
    x10.RFXmeter(rfxSensorID, 0, rawBattery);
    digitalWrite(LED_PIN, LOW);
    f_wdt = false;
    setup_watchdog8s();

  } else {
    setup_watchdog8s();
  }
  
  arm_interrupt();
  system_sleep();
  Serial.println("Wake up");
}
