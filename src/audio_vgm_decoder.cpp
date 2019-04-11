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

#include "audio_vgm_decoder.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef USE_VGM_DECODER

extern "C" int vgm_play_start(const uint8_t *data, int size);
extern "C" int vgm_play_data(void *outBuffer, int size);
extern "C" int vgm_play_stop(void);
extern "C" int vgm_set_format(uint32_t frequency);
extern "C" int vgm_set_volume(float volume);

AudioVgmDecoder::~AudioVgmDecoder()
{
    if ( m_started )
    {
        vgm_play_stop();
        m_started = false;
    }
}

void AudioVgmDecoder::set_melody( const uint8_t *buffer, int size )
{
    if ( m_started )
    {
        vgm_play_stop();
        m_started = false;
    }
    vgm_set_volume( 2.5f );
    vgm_set_format(m_rate);
    vgm_play_start(buffer, size);
    m_started = true;
}

void AudioVgmDecoder::set_format(uint32_t rate, uint8_t bps)
{
    m_rate = rate;
    m_bps = bps;
}

int AudioVgmDecoder::decode(uint8_t* origin_buffer, int max_size)
{
    int size = vgm_play_data(origin_buffer, max_size);
    if ( size == 0 )
    {
        vgm_play_stop();
        m_started = false;
    }
    return size;
}

#endif
