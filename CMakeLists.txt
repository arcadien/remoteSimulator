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

cmake_minimum_required(VERSION 3.10)

project(RemoteSimulator VERSION 0.1.0)

set(PROJECT_NAME "remoteSimulator")

set(CMAKE_CXX_STANDARD 11)

set(SWITCH_FAMILY "e" CACHE STRING  "Zibase(c) signal family" )
set(SWITCH_GROUP  3   CACHE INTEGER "Zibase(c) signal group"  )
set(SWITCH_NUMBER 2   CACHE INTEGER "Zibase(c) signal number" )


set( CMAKE_EXPORT_COMPILE_COMMANDS ON ) 
if( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json)
endif()

include_directories("${SOURCES_DIR}/include/")

if(AVR)
  include(tools/cmake/avr.cmake)
else()
  message(FATAL "This project only works with AVR toolchain")
endif()
