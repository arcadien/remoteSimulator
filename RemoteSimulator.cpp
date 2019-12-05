//
// Original work from:
// http://blog.onlinux.fr/detecteur-de-choc-tx-433mhz-pilotes-avec-attiny85/
//

//                           +-\/-+
//          Ain0 (D 5) PB5  1|    |8  Vcc
//          Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1 -
// LED +pin Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1 - Shock sensor pin
//                     GND  4|    |5  PB0 (D 0) pwm0 - RF433 tx pin
//
//
#include <avr/sleep.h>
#include <avr/wdt.h>

#define RCSwitchDisableReceiving 1
#include <RCSwitch.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// Led will flash if battery level is lower than this value
// in mV.
#define LOW_BATTERY_LEVEL (3 * 1150)

// wdt is set to 8s, 8x225=1800 seconds = 30 minutes
#define WDT_COUNT 225

volatile boolean f_wdt = 0;
volatile byte count = WDT_COUNT;
volatile boolean f_int = 0;
volatile boolean lowBattery = 0;
volatile boolean switchState = HIGH;

const byte TX_PIN = PB0; // Pin number for the 433mhz OOK transmitter
const byte LED_PIN = PB4;
const byte WAKEUP_PIN = PB1; // Use pin 1 as wake up pin

RCSwitch mySwitch = RCSwitch();

/*
 * Blink led at LED_PIN `blinkCount` times
 */
void blink(int blinkCount) {
  pinMode(LED_PIN, OUTPUT);
  for (byte i = blinkCount; i > 0; i--) {
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }

  pinMode(LED_PIN, INPUT); // reduce power
}

void lowBatteryWarning() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  delay(1); // mS
  digitalWrite(LED_PIN, LOW);
  pinMode(LED_PIN, INPUT);
}

uint16_t readVcc(void) {
  uint16_t result;
  // Read 1.1V reference against Vcc
  ADMUX = (0 << REFS0) | (12 << MUX0);
  delay(2);              // Wait for Vref to settle
  ADCSRA |= (1 << ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC)) {}
  result = ADCW;
  // Back-calculate AVcc in mV
  return 1018500L / result;
}

// set system into the sleep state
// system wakes up when watchdog times out
void system_sleep() {
  cbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  sleep_mode();      // System sleeps here
  sleep_disable();   // System continues execution here when watchdog timed out
  sbi(ADCSRA, ADEN); // switch Analog to Digitalconverter ON
}

/*
 * Setup hardware watchdog
 *
 * Values for ii:
 * 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
 * 6=1 sec,7=2 sec, 8=4 sec, 9=8sec
 */
void setup_watchdog(int ii) {
  byte bb;
  int ww;
  if (ii > 9)
    ii = 9;
  bb = ii & 7;
  if (ii > 7)
    bb |= (1 << 5);
  bb |= (1 << WDCE);
  ww = bb;
  MCUSR &= ~(1 << WDRF);
  WDTCR |= (1 << WDCE) | (1 << WDE);
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  if (count >= WDT_COUNT) {
    f_wdt = true; // set WDTl flag
    count = 0;
  }
  count++;
}

ISR(PCINT0_vect) {
  f_int = true; // set INT flag
}

void setup() {
  setup_watchdog(9);
  pinMode(TX_PIN, OUTPUT);
  pinMode(WAKEUP_PIN, INPUT); // Set the pin to input
  mySwitch.enableTransmit(TX_PIN);

  mySwitch.switchOn('e', 3, 2); // Test Zibase signal id E10

  lowBattery =
      !(readVcc() >= LOW_BATTERY_LEVEL); // Initialize battery level value

  PCMSK |= bit(PCINT1); // set pin change interrupt PB1
  GIFR |= bit(PCIF);    // clear any outstanding interrupts
  GIMSK |= bit(PCIE);   // enable pin change interrupts
  sei();
}

void loop() {
  system_sleep();

  if (lowBattery)        // lowBattery is at setup().
    lowBatteryWarning(); // Flash for 1ms every 8s
                         // http://www.gammon.com.au/power

  if (f_int) {
    blink(2);
    cli();

    // Only send message if pin is high
    if (PINB & 0x2) {
      mySwitch.switchOn('e', 3, 2); // Zibase signal id E10
    }

    f_int = false; // Reset INT Flag
    sei();
  } else if (f_wdt) {
    blink(5);
    cli();
    lowBattery = !(readVcc() >= LOW_BATTERY_LEVEL);
    f_wdt = false;
    sei();
  }
}
