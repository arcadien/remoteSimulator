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

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

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

// Led will flash if battery level is lower than this value
// in mV.
#define LOW_BATTERY_LEVEL (2 * 1150)

// wdt is set to 8s, 8x225=1800 seconds = 30 minutes
#define WDT_COUNT 225

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

volatile boolean f_wdt = 0;
volatile byte count = WDT_COUNT;
volatile boolean f_int = 0;
volatile boolean lowBattery = 0;

// real 1v1 ref value, in mv
const uint32_t REF_1V1 = 1048;

const byte TX_PIN = PB3; // Pin number for the 433mhz OOK transmitter
const byte LED_PIN = PB4;
const byte WAKEUP_PIN = PB2;

RCSwitch mySwitch = RCSwitch();

/*
 * Blink led at LED_PIN `blinkCount` times
 */
void blink(int blinkCount) {
  pinMode(LED_PIN, OUTPUT);
  for (byte i = blinkCount; i > 0; i--) {
    digitalWrite(LED_PIN, HIGH);
    _delay_ms(100);
    digitalWrite(LED_PIN, LOW);
    _delay_ms(100);
  }

  pinMode(LED_PIN, INPUT); // reduce power
}

void lowBatteryWarning() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  _delay_ms(10);

  pinMode(LED_PIN, INPUT);
}

uint16_t readVcc(void) {
  uint16_t result;

  // Use internal 1.1v reference
  ADMUX = 0;
  sbi(ADMUX, MUX3);
  sbi(ADMUX, MUX2);
  _delay_ms(2);

  uint16_t accumulator = 0;
  for (uint8_t i = 16; i > 0; --i) {
    sbi(ADCSRA, ADSC);
    while (bit_is_set(ADCSRA, ADSC)) {}
    accumulator += ADC;
  }
  result = (accumulator >> 4);

  // Back-calculate AVcc in mV
  uint16_t millivolts = ((uint32_t)1024 * REF_1V1) / result;

  Serial.print("Current Voltage:");
  Serial.print(millivolts);
  Serial.println("mV");

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
  // switch Analog to Digitalconverter OFF
  cbi(ADCSRA, ADEN);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sei();
  sleep_mode();
  // switch Analog to Digitalconverter ON
  sbi(ADCSRA, ADEN);
}

void setup_watchdog() {
  WDTCR |= (1 << WDE) | (1 << WDIE) | (1 << WDP3) | (1 << WDP0); // 8s timeout
}

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  if (count >= WDT_COUNT) {
    f_wdt = true; // set WDTl flag
    count = 0;
  }
  count++;
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

void setup() {

  pinMode(TX_PIN, OUTPUT);
  pinMode(WAKEUP_PIN, INPUT);
  mySwitch.enableTransmit(TX_PIN);

  // UART support, TX only
  // see
  // https://github.com/SpenceKonde/ATTinyCore/blob/master/avr/extras/ATtiny_x5.md
  Serial.begin(9600);
  ACSR &= ~(1 << ACIE);
  ACSR |= ~(1 << ACD);

  Serial.println("REMOTESIMULATOR");
  Serial.print("GIT: ");
  Serial.println(TOSTRING(GIT_TAG));

  Serial.print("SWITCH_FAMILY: ");
  Serial.println(*TOSTRING(SWITCH_FAMILY));
  Serial.print("SWITCH_GROUP: ");
  Serial.println(SWITCH_GROUP);
  Serial.print("SWITCH_NUMBER: ");
  Serial.println(SWITCH_NUMBER);

  _delay_ms(100);

  lowBattery =
      !(readVcc() >= LOW_BATTERY_LEVEL); // Initialize battery level value

  blink(2);
  setup_watchdog();
}

/*!
 * If USE_HIGH_PCINT1, ensure PB3 is high otherwise do not trigger any action
 */
void loop() {
  if (lowBattery)        // lowBattery is at setup().
    lowBatteryWarning(); // Flash for 1ms every 8s
                         // http://www.gammon.com.au/power

  if (f_int) {
    Serial.println("Emit signal");
    f_int = false;
    blink(1);
    mySwitch.switchOn(*TOSTRING(SWITCH_FAMILY), SWITCH_GROUP, SWITCH_NUMBER);
    _delay_ms(100);

  } else if (f_wdt) {
    lowBattery = (readVcc() <= LOW_BATTERY_LEVEL);
    f_wdt = false;
  }
  setup_watchdog();
  arm_interrupt();
  system_sleep();
}
