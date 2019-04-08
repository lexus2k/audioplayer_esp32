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

#include "audio_gme_decoder.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef USE_GME_DECODER

AudioGmeDecoder::~AudioGmeDecoder()
{
}

void AudioGmeDecoder::set_melody( const uint8_t *buffer, int size )
{
//    gme_open_file( filename, &m_emu, 44100 );
    if (m_emu != nullptr)
    {
        gme_delete( m_emu );
        m_emu = nullptr;
    }
    gme_open_data( buffer, size, &m_emu, m_rate );
    gme_start_track( m_emu, 0 );
}

void AudioGmeDecoder::set_format(uint32_t rate, uint8_t bps)
{
    m_rate = rate;
    m_bps = bps;
}

int AudioGmeDecoder::decode(uint8_t* origin_buffer, int max_size)
{
    gme_err_t err = gme_play( m_emu, max_size, (short int*)origin_buffer ) ;
    if (err != nullptr)
    {
        printf( err );
        return 0;
    }
    return max_size;
}

#endif
