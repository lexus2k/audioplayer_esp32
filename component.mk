#
#    This file is part of I2S audio player for ESP32.
#    Copyright (C) 2019  Alexey Dynda.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# This is Makefile for ESP32 IDF

COMPONENT_ADD_INCLUDEDIRS := ./include
COMPONENT_SRCDIRS := ./src
CPPFLAGS += -DUSE_VGM_DECODER
# CPPFLAGS += -DUSE_GME_DECODER

# COMPONENT_DEPENDS := spibus
