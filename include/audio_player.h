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

#include "audio_notes_decoder.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include <stdint.h>
#include <string.h>

class AudioPlayer
{
public:
    AudioPlayer(uint32_t frequency = 16000);
    ~AudioPlayer();

    void play(const NixieMelody* melody);
    void play_vgm(const uint8_t *buffer, int size);
    void begin();
    void end();
    void set_on_play_complete( void (*cb)() = nullptr ) { m_on_play_complete = cb; }
    /**
     * Return false, when nothing is played
     */
    bool update();
    void set_prebuffering(int prebuffering_ms);

private:
    AudioDecoder* m_decoder = nullptr;
    void (*m_on_play_complete)() = nullptr;
    uint32_t m_frequency = 16000;
    uint8_t* m_buffer = nullptr;
    uint32_t m_i2s_buffer_size = 512;
    uint32_t m_decoder_buffer_size = 2048;
    uint8_t* m_write_pos = nullptr;
    uint8_t* m_player_pos = nullptr;
    int m_size;
    SemaphoreHandle_t m_mutex;

    int reset_player();
    int decode_data();
    int play_data();
};
