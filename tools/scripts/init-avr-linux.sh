#
# This file is part of the Hack distribution (https://github.com/arcadien/Hack)
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

export MCU=attiny85
export BOARD=tinyX5
export CLOCK=1000000UL


cmake  $1 \
-G "Unix Makefiles" \
-DMCU_SPEED=$CLOCK \
-DAVR_MCU=$MCU \
-DBOARD_VARIANT=$BOARD \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_TOOLCHAIN_FILE=$1/third_party/cmake-avr/generic-gcc-avr.cmake \
-DSWITCH_FAMILY='a' \
-DSWITCH_GROUP=1 \
-DSWITCH_NUMBER=1
