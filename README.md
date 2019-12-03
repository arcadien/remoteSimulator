# Remote Simulator

[![Travis build](https://travis-ci.org/arcadien/remoteSimulator.svg?branch=master)](https://travis-ci.org/arcadien/remoteSimulator)
[![codecov](https://codecov.io/gh/arcadien/remoteSimulator/branch/master/graph/badge.svg)](https://codecov.io/gh/arcadien/remoteSimulator)

A remote simulator designed for ATTiny85.
It provides CMake setup and Git submodules to allow compilation outside Arduino IDE.

The goal of this project is to hack a IR detector light, giving it a way to transmit detection event.

The main source code has been slightly adapted from a version found on [Onlinux.fr](http://blog.onlinux.fr/detecteur-de-choc-tx-433mhz-pilotes-avec-attiny85/)

# Links
We rely on other open source projects, which are:
* [CMake](https://cmake.org/)
* [CMake for AVR library](https://github.com/mkleemann/cmake-avr)
* [Arduino Core](https://github.com/arduino/ArduinoCore-avr)
* [ATTiny Arduino support](https://github.com/damellis/attiny)
* [RC-Switch Arduino library](https://github.com/sui77/rc-switch)
