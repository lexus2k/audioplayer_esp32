/*
    This file is part of I2S audio player for ESP32.
    Copyright (C) 2019-2020  Alexey Dynda.

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

AudioVgmDecoder::~AudioVgmDecoder()
{
    if ( m_started )
    {
        m_vgm.close();
        m_started = false;
    }
}

void AudioVgmDecoder::set_melody( const uint8_t *buffer, int size )
{
    m_vgm.setSampleFrequency(m_rate);
    m_vgm.open( buffer, size );
    m_started = true;
}

void AudioVgmDecoder::set_format(uint32_t rate, uint8_t bps)
{
    m_rate = rate;
    m_bps = bps;
}

void AudioVgmDecoder::set_volume( float volume )
{
    m_vgm.setVolume( static_cast<uint16_t>(volume * 64) );
}

void AudioVgmDecoder::set_duration(uint32_t duration_ms)
{
    m_vgm.setMaxDuration( duration_ms );
}

void AudioVgmDecoder::set_track(int track)
{
    m_vgm.setTrack( track );
}

int AudioVgmDecoder::decode(uint8_t* origin_buffer, int max_size)
{
    int size = m_vgm.decodePcm(origin_buffer, max_size);
    if ( size < 0 ) return 0;
    if ( size == 0 )
    {
        m_vgm.close();
        m_started = false;
    }
    return size;
}

#endif
