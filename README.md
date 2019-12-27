# Remote Simulator

[![Travis build](https://travis-ci.org/arcadien/remoteSimulator.svg?branch=master)](https://travis-ci.org/arcadien/remoteSimulator)
[![codecov](https://codecov.io/gh/arcadien/remoteSimulator/branch/master/graph/badge.svg)](https://codecov.io/gh/arcadien/remoteSimulator)

This firmware allow to trigger the emission of a 433Mhz signal when an event
occurs. The send signal is a on/off signal, so that this firmware is not
suitable for temperature or other discrete data. It is suited for sound,
light or movment detection. The trigger sensor can be active low or high.
Two different definitions and two pins are used to select one mode or another.
The firmware also monitors the power level, so that a low battery level will
make the LED signal to quickly blink each 8 seconds.
mode.

Technical details:
- Written for ATTiny85
- Uses CMake or PlatformIO for build
- The AVR watchdog is used for deep sleep management.
- Battery level is evaluated comparing Vcc with internal 1V1 reference
- SoftwareSerial libray is useable for debugging or send serial data on PB0

```
                  +-\/-+
 Reset      PB5  1|    |8  Vcc
 RF433 TX - PB3  2|    |7  PB2 - Trigger pin (active low) if USE_LOW_INT0
 LED +pin - PB4  3|    |6  PB1 - Trigger pin (active high) if USE_HIGH_PCINT1
            GND  4|    |5  PB0 - Serial TX (trace)
 ``` 

The main source code has been slightly adapted from a version found on [Onlinux.fr](http://blog.onlinux.fr/detecteur-de-choc-tx-433mhz-pilotes-avec-attiny85/)

# Compile the code

The code can be compiled a plain AVR gcc toolchain, using the script in `tools/scripts`. 
It also can be compiled using the [PlatformIO](https://platformio.org) tool.

## Using CMake

Be sure to clone the repository with its submodules. Then create a build folder in the repo, `cd` to it and type `../tools/scripts/init-avr-linux.sh ..`. After that, the code is ready for compilation. Type `make -j` and gather the .hex files.

## Using PlatformIO

Cloning submodules is not mandatory, you don't even need to install the AVR toolchain. But PlatformIO itself must be installed, using `pip3 install platformIO`.
Once installed, type `pip3 install platformio`, then `pio run`.

# Links
We rely on other open source projects, which are:
* [CMake](https://cmake.org/)
* [CMake for AVR library](https://github.com/mkleemann/cmake-avr)
* [Arduino Core](https://github.com/arduino/ArduinoCore-avr)
* [ATTiny Arduino support](https://github.com/damellis/attiny)
* [RC-Switch Arduino library](https://github.com/sui77/rc-switch)
