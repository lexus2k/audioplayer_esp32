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

#include "nixie_audio_defs.h"
#include "audio_decoder.h"
#include <stdint.h>

class AudioNotesDecoder: public AudioDecoder
{
public:
    AudioNotesDecoder() = default;
    virtual ~AudioNotesDecoder() = default;

    void set_format(uint32_t rate, uint8_t bps);
    void set_melody( const NixieMelody* melody );

    int decode(uint8_t* buffer, int max_size) override;

private:
    const NixieMelody* m_melody = nullptr;
    uint32_t m_rate = 16000;
    uint8_t m_bps = 16;
    const uint8_t *m_position = nullptr;
    uint16_t m_note_samples_left = 0;
    uint16_t m_played_period = 0;
    uint16_t m_samples_per_period = 0;
    uint16_t m_pause_left = 0;
    uint16_t m_last_index = 0;

    bool read_note_data();
    void next_note();
};
