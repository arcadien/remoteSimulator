#
#
# Copyright (c) 2019 Aurélien Labrosse
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#


#
# This Cmake scripts perform project
# configuration for AVR chip target
#
# uploadtool is: AVR_UPLOADTOOL
# programmer is: AVR_PROGRAMMER
# upload port is: AVR_UPLOADTOOL_PORT
# uploadtool options are: AVR_UPLOADTOOL_OPTIONS
# MCU is set to: AVR_MCU
# MCU speed (Hz) is set to: MCU_SPEED
# H_FUSE is set to: AVR_H_FUSE
# L_FUSE is set to: AVR_L_FUSE

# ############################################################################
# compiler options for all build types
# ############################################################################
add_definitions("-DF_CPU=${MCU_SPEED}")
add_definitions("-std=gnu++11")
add_definitions("-fpack-struct")
add_definitions("-fshort-enums")
add_definitions("-Wall")
add_definitions("-funsigned-char")
add_definitions("-funsigned-bitfields")
add_definitions("-ffunction-sections")
add_definitions("-DARDUINO=101")

# Do not use on library
#add_definitions("-flto")

set(AVR_BINARIES "${ARDUINO_ROOT}/hardware/tools/avr/bin")
message("-- Using AVR toolchain binaries in ${AVR_BINARIES}")

find_program(AVR_CC avr-gcc HINTS ${AVR_BINARIES})
find_program(AVR_CXX avr-g++ HINTS ${AVR_BINARIES})
find_program(AVR_OBJCOPY avr-objcopy HINTS ${AVR_BINARIES})
find_program(AVR_SIZE_TOOL avr-size HINTS ${AVR_BINARIES})
find_program(AVR_OBJDUMP avr-objdump HINTS ${AVR_BINARIES})

# ############################################################################
# status messages
# ############################################################################
message(STATUS "Current C compiler is: ${AVR_CC}")
message(STATUS "Current C++ compiler is: ${AVR_CXX}")
message(STATUS "Current OBJCOPY is: ${AVR_OBJCOPY}")
message(STATUS "Current AVR_SIZE_TOOL is: ${AVR_SIZE_TOOL}")
message(STATUS "Current AVR_OBJDUMP is: ${AVR_OBJDUMP}")

message(STATUS "Current uploadtool is: ${AVR_UPLOADTOOL}")
message(STATUS "Current programmer is: ${AVR_PROGRAMMER}")
message(STATUS "Current upload port is: ${AVR_UPLOADTOOL_PORT}")
message(STATUS "Current uploadtool options are: ${AVR_UPLOADTOOL_OPTIONS}")
message(STATUS "Current MCU is set to: ${AVR_MCU}")
message(STATUS "Current MCU speed (Hz) is set to: ${MCU_SPEED}")
message(STATUS "Current H_FUSE is set to: ${AVR_H_FUSE}")
message(STATUS "Current L_FUSE is set to: ${AVR_L_FUSE}")

if(NOT BOARD_VARIANT)
  set(BOARD_VARIANT "standard")
endif()

message(STATUS "Current board variant is set to : ${BOARD_VARIANT}")

if(CMAKE_BUILD_TYPE MATCHES Release)
  set(CMAKE_C_FLAGS_RELEASE "-Os")
  set(CMAKE_CXX_FLAGS_RELEASE "-Os")
endif(CMAKE_BUILD_TYPE MATCHES Release)

if(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  set(CMAKE_C_FLAGS_RELWITHDEBINFO
      "-Os -save-temps -g -gdwarf-3 -gstrict-dwarf")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
      "-Os -save-temps -g -gdwarf-3 -gstrict-dwarf")
endif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS_DEBUG "-O0 -save-temps -g -gdwarf-3 -gstrict-dwarf")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -save-temps -g -gdwarf-3 -gstrict-dwarf")
endif(CMAKE_BUILD_TYPE MATCHES Debug)

set(ARDUINO_ROOT "${CMAKE_SOURCE_DIR}/third_party/arduino-core")
if(CROSS_FROM_WINDOWS)
  set(ARDUINO_SRC "${ARDUINO_ROOT}/hardware/arduino/avr/cores/arduino/")
  include_directories("${ARDUINO_ROOT}/hardware/arduino/avr/variants/${BOARD_VARIANT}")
  include_directories("third_party/attiny/variants/${BOARD_VARIANT}")
  include_directories("${ARDUINO_ROOT}/hardware/tools/avr/avr/include")
else()
  set(ARDUINO_SRC "${ARDUINO_ROOT}/cores/arduino/")
  include_directories("${ARDUINO_ROOT}/variants/${BOARD_VARIANT}")
  include_directories("third_party/attiny/variants/${BOARD_VARIANT}")
endif()
file(GLOB ARDUINO_SRC_CPP "${ARDUINO_SRC}/*.cpp")
file(GLOB ARDUINO_SRC_C   "${ARDUINO_SRC}/*.c")
file(GLOB ARDUINO_INCLUDE "${ARDUINO_SRC}/*.h")

include_directories("${ARDUINO_SRC}")

add_avr_library(arduino
  STATIC
  ${ARDUINO_SRC_CPP}
  ${ARDUINO_SRC_C}
  ${ARDUINO_INCLUDE})

set(RC_SWITCH "${CMAKE_SOURCE_DIR}/third_party/rc-switch")
include_directories(${RC_SWITCH})
add_avr_library(rc-switch STATIC "${RC_SWITCH}/RCSwitch.cpp")
avr_target_link_libraries(rc-switch arduino)

add_avr_executable(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/RemoteSimulator.cpp")

avr_target_link_libraries(${PROJECT_NAME} arduino)
avr_target_link_libraries(${PROJECT_NAME} rc-switch)
