# Remote Simulator

[![Travis build](https://travis-ci.org/arcadien/remoteSimulator.svg?branch=master)](https://travis-ci.org/arcadien/remoteSimulator)

This firmware allow to trigger the emission of a 433Mhz signal when a 'trigger event' occurs. 
The sent signal is a `on` or `off` signal, so that this firmware is not suitable for temperature or other discrete data. 
It is suited for something like sound, light or motion detection.
The trigger sensor can be active low or high, two different build options and two pins are used to select one mode or another.
The firmware also monitors the power level.

Technical details:
- Written for ATTiny85
- Uses PlatformIO for build
- The AVR watchdog is used for deep sleep management
- Battery level is evaluated comparing Vcc with internal 1V1 reference, and low battery voltage is a build parameter (platformio.ini)
- SoftwareSerial libray is useable for debugging or send serial data (9600 bauds/s) on `PB0`
- Double trigger for 'off' signal is a build parameter. It allows to clap one time for `on` and two times for `off`. The delay between two triggers is 1 second.
- Detail of on/off signals are build parameters (using [RC-Switch](https://github.com/sui77/rc-switch) `Type C Intertechno`)
- The battery voltage is sent at regular interval (1 hour by default)
- The battery voltage is sent using [X10](https://en.wikipedia.org/wiki/X10_(industry_standard)) protocol
- A low battery level will make the LED to quickly blink each 8 seconds.

Sample output trace:
```
REMOTESIMULATOR
GIT: 9b555aba0e02795dadab3060a91e6916c52096a9
SWITCH_FAMILY: a
SWITCH_GROUP: 1
SWITCH_NUMBER: 1
LOW BATTERY VOLTAGE: 2300
USE DOUBLE TRIGGER FOR OFF: 1
Current Voltage:5339mV
Wake up
[...]
```

```
                  +-\/-+
 Reset      PB5  1|    |8  Vcc
 RF433 TX - PB3  2|    |7  PB2 - Trigger pin (active low) if USE_LOW_INT0
 LED +pin - PB4  3|    |6  PB1 - Trigger pin (active high) if USE_HIGH_PCINT1
            GND  4|    |5  PB0 - Serial TX (trace)
 ``` 


The main source code has been slightly adapted from a version found on [Onlinux.fr](http://blog.onlinux.fr/detecteur-de-choc-tx-433mhz-pilotes-avec-attiny85/)

# Compile the code

## Using PlatformIO

PlatformIO itself must be installed, using `pip3 install platformIO`.
Once installed, type `pip3 install platformio`, then `pio run`.

# Links
We rely on other open source projects, which are:
* [X10 protocol library for Arduino](https://github.com/pyrou/X10RF-Arduino)
* [RC-Switch Arduino library](https://github.com/sui77/rc-switch)
