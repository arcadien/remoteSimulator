//
// Original work from:
// http://blog.onlinux.fr/detecteur-de-choc-tx-433mhz-pilotes-avec-attiny85/
//

//                        +-\/-+
//            (D 5) PB5  1|    |8  Vcc
// RF433 TX - (D 3) PB3  2|    |7  PB2 (D 2) - Trigger pin (active low) if USE_LOW_INT0
// LED +pin - (D 4) PB4  3|    |6  PB1 (D 1) - Trigger pin (active high) if USE_HIGH_PCINT1
//                  GND  4|    |5  PB0 (D 0) - Serial TX (trace)

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

volatile boolean f_wdt = 0;
volatile byte count = WDT_COUNT;
volatile boolean f_int = 0;
volatile boolean lowBattery = 0;

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
  // Read 1.1V reference against Vcc
  ADMUX = (0 << REFS0) | (12 << MUX0);
  _delay_ms(2);
  ADCSRA |= (1 << ADSC);
  while (bit_is_set(ADCSRA, ADSC)) {}
  result = ADCW;
  // Back-calculate AVcc in mV
  uint16_t millivolts = 1018500L / result;

  Serial.print("Current Voltage:");
  float volts = millivolts / 1000;
  Serial.print(volts);
  Serial.println("V");

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
    f_int = false;
    blink(1);
    mySwitch.switchOn('e', 3, 2);
    _delay_ms(100);

  } else if (f_wdt) {
    lowBattery = (readVcc() <= LOW_BATTERY_LEVEL);
    f_wdt = false;
  }
  setup_watchdog();
  arm_interrupt();
  system_sleep();
}
