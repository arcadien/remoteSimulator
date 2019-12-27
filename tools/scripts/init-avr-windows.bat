@echo off
rem 
rem  This file is part of the Hack distribution (https://github.com/arcadien/Hack)
rem 
rem  Copyright (c) 2019 Aurélien Labrosse
rem 
rem  This program is free software: you can redistribute it and/or modify
rem  it under the terms of the GNU General Public License as published by
rem  the Free Software Foundation, version 3.
rem 
rem  This program is distributed in the hope that it will be useful, but
rem  WITHOUT ANY WARRANTY; without even the implied warranty of
rem  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
rem  General Public License for more details.
rem 
rem  You should have received a copy of the GNU General Public License
rem  along with this program. If not, see <http://www.gnu.org/licenses/>.
rem 
rem 
rem To use this script, Arduino IDE must installed in C:/Program Files (x86)/Arduino/.
rem If not, change the path of Arduino below. 
rem To change these settings, update MCU and BOARD below.
rem 
rem Sample call:
rem
rem ```
rem mkdir build
rem cd build
rem ..\tools\Script\init-avr-windows.bat ..
rem
rem ```

set MCU=attiny85
set BOARD=tinyX5
set CLOCK=1000000UL

cmake  %1                                                              ^
-DCMAKE_TOOLCHAIN_FILE:PATH=%1/third_party/cmake-avr/generic-gcc-avr.cmake ^
-G "Unix Makefiles"                                                    ^
-DMCU_SPEED:STRING=%CLOCK%                                             ^
-DAVR_MCU:STRING=%MCU%                                                 ^
-DBOARD_VARIANT:STRING=%BOARD%                                         ^
-DCMAKE_BUILD_TYPE=Release                                             ^
-DARDUINO_ROOT:PATH="C:/Program Files (x86)/Arduino/"                  ^
-DCROSS_FROM_WINDOWS:BOOL=True                                         ^
-DCMAKE_MAKE_PROGRAM:PATH="make"                                       ^
-DSWITCH_FAMILY='a'                                                    ^ 
-DSWITCH_GROUP=1                                                       ^
-DSWITCH_NUMBER=1
