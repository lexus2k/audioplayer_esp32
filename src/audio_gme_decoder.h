/*
    This file is part of I2S audio player for ESP32.
    Copyright (C) 2019  Alexey Dynda.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "audio_decoder.h"
#include <stdint.h>

#ifdef USE_GME_DECODER

#include "gme.h"

class AudioGmeDecoder: public AudioDecoder
{
public:
    AudioGmeDecoder() = default;
    virtual ~AudioGmeDecoder();

    void set_melody( const uint8_t *buffer, int size );
    void set_format(uint32_t rate, uint8_t bps);
    int decode(uint8_t* buffer, int max_size) override;
private:
    Music_Emu* m_emu = nullptr;
    uint32_t m_rate = 16000;
    uint8_t m_bps = 16;
};

#endif
