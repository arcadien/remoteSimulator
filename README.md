# Remote Simulator

[![Travis build](https://travis-ci.org/arcadien/remoteSimulator.svg?branch=master)](https://travis-ci.org/arcadien/remoteSimulator)
[![codecov](https://codecov.io/gh/arcadien/remoteSimulator/branch/master/graph/badge.svg)](https://codecov.io/gh/arcadien/remoteSimulator)

A remote simulator designed for ATTiny85.
It provides CMake setup and Git submodules to allow compilation outside Arduino IDE.

The goal of this project is to hack a IR detector light, giving it a way to transmit detection event.

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
