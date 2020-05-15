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

#pragma once

#include "nixie_audio_defs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdint.h>
#include <string.h>

enum class EAudioChannels: uint8_t
{
    LEFT_ONLY = 0x01,
    RIGHT_ONLY = 0x02,
    BOTH = 0x01 | 0x02,
};

class AudioDecoder;

class AudioPlayer
{
public:
    AudioPlayer(uint32_t frequency = 16000);
    ~AudioPlayer();

    /**
     * Starts playing of specified melody.
     * You should call update() method in the loop until it returns false.
     */
    void play(const NixieMelody* melody);

    /** Interrupts playing of current melody */
    void stop();

    /** Changes volume for the melody */
    void set_volume( float volume );

    /** Initializes ESP I2S hardware */
    void begin(EAudioChannels channels = EAudioChannels::BOTH);

    /** Frees ESP hardware */
    void end();

    /** Set callback to be used on play completion */
    void set_on_play_complete( void (*cb)() = nullptr ) { m_on_play_complete = cb; }

    /** Returns true if melody is playing */
    bool is_playing();

    /**
     * Return false, when nothing is played
     */
    bool update();

    /**
     * Set prebuffering for melody decoder in milliseconds. update() method must be called
     * at least each  prebuffering_ms milliseconds.
     */
    void set_prebuffering(uint32_t prebuffering_ms);

private:
    AudioDecoder* m_decoder = nullptr;
    void (*m_on_play_complete)() = nullptr;
    float m_volume = 1.0f;
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

    void play_notes(const NixieMelody *melody);
    void play_vgm(const NixieMelody *melody);
    void play_nsf(const NixieMelody *melody);
};
