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

#include "audio_notes_decoder.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const uint16_t s_amplitude[16] =
{
    0x0000,0x1000,0x8000,0x8000,0xEFFF,0xFFFF,0xFFFF,0xFFFF,
    0xFFFF,0xEFFF,0x8000,0x8000,0x1000,0x0000,0x0000,0x0000,
};

enum
{
    STATE_READ_NOTE = 0,
    STATE_PLAY_NOTE,
    STATE_MAKE_PAUSE,
    STATE_STOP,
};

void AudioNotesDecoder::set_melody( const NixieMelody* melody )
{
    m_melody = melody;
    m_position = melody->notes;
    m_note_samples_left = 0;
    m_pause_left = 0;
    m_state = STATE_READ_NOTE;
}

void AudioNotesDecoder::set_format(uint32_t rate, uint8_t bps)
{
    m_rate = rate;
    m_bps = bps;
}

int AudioNotesDecoder::decode(uint8_t* origin_buffer, int max_size)
{
    if (m_melody == nullptr)
    {
        return -1;
    }
    uint8_t* buffer = origin_buffer;
    int remaining = max_size;
    while ( remaining > 0 && m_state != STATE_STOP )
    {
        switch (m_state)
        {
            case STATE_READ_NOTE:
                m_state = read_note_data() == false ? STATE_STOP: STATE_PLAY_NOTE;
                break;
            case STATE_PLAY_NOTE:
                if ( m_note_samples_left == 0 )
                {
                    m_state = STATE_MAKE_PAUSE;
                }
                if ( m_samples_per_period )
                {
                    uint16_t remainder = m_played_period % m_samples_per_period;
                    m_last_index = (16 * remainder / m_samples_per_period);
                }
                // RIGHT ???
                *reinterpret_cast<uint16_t*>(buffer) = s_amplitude[m_last_index];
                remaining -= (m_bps / 8);
                buffer += (m_bps / 8);
                // LEFT ???
                *reinterpret_cast<uint16_t*>(buffer) = s_amplitude[m_last_index];
                remaining -= (m_bps / 8);
                buffer += (m_bps / 8);

                m_note_samples_left--;
                m_played_period++;
                break;
            case STATE_MAKE_PAUSE:
                if ( m_pause_left == 0 )
                {
                    next_note();
                    m_state = STATE_READ_NOTE;
                    break;
                }
                *reinterpret_cast<uint16_t*>(buffer) = s_amplitude[m_last_index];
                remaining -= (m_bps / 8);
                buffer += (m_bps / 8);
                *reinterpret_cast<uint16_t*>(buffer) = s_amplitude[m_last_index];
                remaining -= (m_bps / 8);
                buffer += (m_bps / 8);

                m_pause_left--;
                break;
            default:
                break;
        }
    }
    return buffer - origin_buffer;
}

bool AudioNotesDecoder::read_note_data()
{
    bool result = false;
    m_samples_per_period = 0;
    switch ( m_melody->type )
    {
        case MELODY_TYPE_PROGMEM_TEMPO:
        {
            SNixieTempoNote note = *reinterpret_cast<const SNixieTempoNote*>(m_position);
            if ( note.freq == NOTE_STOP )
            {
                break;
            }
            m_note_samples_left = m_rate / note.tempo;
            m_samples_per_period = (note.freq > NOTE_LAST_CMD) ? (m_rate / note.freq) : 100000;
            m_pause_left = 0;
            result = true;
            if ( m_melody->pause < 0 )
            {
                m_pause_left = (m_note_samples_left  * (-m_melody->pause) / 32);
            }
            else if ( m_melody->pause > 0)
            {
                m_pause_left = m_rate * m_melody->pause / 1000;
            }
            break;
        }
        case MELODY_TYPE_PROGMEM_SAMPLING:
        {
            SNixieSamplingNote note = *reinterpret_cast<const SNixieSamplingNote*>(m_position);
            if ( note.freq == NOTE_STOP )
            {
                break;
            }
            m_note_samples_left = (note.duration * m_rate) / 1000;
            m_samples_per_period = (note.freq > NOTE_LAST_CMD) ? (m_rate / note.freq) : 100000;
            m_pause_left = 0;
            result = true;
            if ( m_melody->pause < 0 )
            {
                m_pause_left = m_note_samples_left * (-m_melody->pause) / 32;
            }
            else if ( m_melody->pause > 0)
            {
                m_pause_left = m_rate * m_melody->pause / 1000;
            }
            break;
        }
        default:
            break;
    }
    if ( result )
    {
        m_played_period = m_samples_per_period * m_last_index / 16;
        if ( m_melody->pause > 0 )
        {
            m_pause_left = m_rate * m_melody->pause / 1000;
        }
    }
    return result;
}

void AudioNotesDecoder::next_note()
{
    switch ( m_melody->type )
    {
        case MELODY_TYPE_PROGMEM_TEMPO:
            m_position += sizeof(SNixieTempoNote);
            break;
        case MELODY_TYPE_PROGMEM_SAMPLING:
            m_position += sizeof(SNixieSamplingNote);
            break;
        default:
            break;
    }
}

