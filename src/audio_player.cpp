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

#include "audio_player.h"
#ifdef USE_GME_DECODER
#include "audio_gme_decoder.h"
#elif defined(USE_VGM_DECODER)
#include "audio_vgm_decoder.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_adc_cal.h"
#include <stdio.h>
#include <string.h>

//#define AUDIO_PLAYER_DEBUG

static int write_i2s_data(uint8_t *buffer, int len)
{
    size_t written = 0;
    if ( buffer == nullptr )
    {
        i2s_zero_dma_buffer( I2S_NUM_0 );
    }
    else
    {
        esp_err_t err = i2s_write(I2S_NUM_0, buffer, len, &written, 0); //portMAX_DELAY);
        if (err != ESP_OK)
        {
            return -1;
        }
#ifdef I2S_DEBUG
        printf("i2c: %u\n", written);
#endif
    }
    return written;
}

AudioPlayer::AudioPlayer(uint32_t frequency)
   : m_frequency( frequency )
{
    m_mutex = xSemaphoreCreateMutex();
    set_prebuffering( 20 );
}

AudioPlayer::~AudioPlayer()
{
    vSemaphoreDelete(m_mutex);
}

void AudioPlayer::begin()
{
    xSemaphoreTake( m_mutex, portMAX_DELAY );
    i2s_config_t i2s_config{};
    i2s_config.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN);
    i2s_config.sample_rate = m_frequency;
    i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    i2s_config.channel_format = I2S_CHANNEL_FMT_ALL_RIGHT;
    i2s_config.communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_I2S_MSB);
    i2s_config.intr_alloc_flags = 0; //ESP_INTR_FLAG_LEVEL1;
    i2s_config.dma_buf_count = 8;
    i2s_config.dma_buf_len = m_i2s_buffer_size / 8;
    i2s_config.use_apll = false;
    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK)
    {
        printf("error: %i\n", err);
    }
//    i2s_set_pin(I2S_NUM_0, NULL);
    i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN);
    i2s_set_sample_rates(I2S_NUM_0, m_frequency);
    i2s_zero_dma_buffer( I2S_NUM_0 );
    xSemaphoreGive( m_mutex );
}

void AudioPlayer::end()
{
    xSemaphoreTake( m_mutex, portMAX_DELAY );
    if (m_buffer != nullptr )
    {
        free(m_buffer);
        m_buffer = nullptr;
    }
    i2s_driver_uninstall(I2S_NUM_0);
    xSemaphoreGive( m_mutex );
}

void AudioPlayer::set_prebuffering(int prebuffering_ms)
{
    uint32_t bytes = m_frequency * prebuffering_ms / 1000 * 2 * 2;
    int i=5;
    while ( bytes > (1<<i) ) i++;
    // At least we need i2s buffer to hold audio data for preburreing_ms period
    m_i2s_buffer_size = 1<<i;
    // We do not need large buffer for decoder
    m_decoder_buffer_size = m_i2s_buffer_size / 2;
}

void AudioPlayer::play(const NixieMelody* melody)
{
    xSemaphoreTake( m_mutex, portMAX_DELAY );
    if (m_decoder != nullptr)
    {
        delete m_decoder;
    }
    AudioNotesDecoder* decoder = new AudioNotesDecoder();
    // TODO get format from m_output
    decoder->set_format(m_frequency, 16);
    decoder->set_melody( melody );
    m_decoder = decoder;
    reset_player();
    xSemaphoreGive( m_mutex );
}

void AudioPlayer::play_vgm(const uint8_t *buffer, int size)
{
    xSemaphoreTake( m_mutex, portMAX_DELAY );
    if (m_decoder != nullptr)
    {
        delete m_decoder;
    }
#if   defined(USE_GME_DECODER)
    AudioGmeDecoder* decoder = new AudioGmeDecoder();
#elif defined(USE_VGM_DECODER)
    AudioVgmDecoder* decoder = new AudioVgmDecoder();
#endif
    m_decoder = decoder;
    if (m_decoder == nullptr )
    {
        return;
    }
    // TODO get format from m_output
    decoder->set_format( m_frequency, 16 );
    decoder->set_melody( buffer, size );
    reset_player();
    xSemaphoreGive( m_mutex );
}

int AudioPlayer::reset_player()
{
    if (m_buffer != nullptr)
    {
        free( m_buffer );
        m_buffer = nullptr;
    }
    m_buffer = static_cast<uint8_t*>(malloc(m_decoder_buffer_size));
    m_write_pos = m_buffer;
    m_player_pos = m_buffer;
    m_size = 0;
    return 0;
}

int AudioPlayer::decode_data()
{
    uint8_t *end = m_buffer + m_decoder_buffer_size;
    int size = m_decoder_buffer_size - m_size;
    if ( size > end - m_write_pos )
    {
        size = end - m_write_pos;
    }
    if ( size )
    {
        size = m_decoder->decode( m_write_pos, size );
        if ( size >= 0 )
        {
            m_write_pos += size;
            m_size += size;
            if ( m_write_pos == end )
            {
                m_write_pos = m_buffer;
            }
        }
    }
    return size;
}

int AudioPlayer::play_data()
{
    uint8_t *end = m_buffer + m_decoder_buffer_size;
    int size = m_size;
    if ( size > end - m_player_pos )
    {
        size = end - m_player_pos;
    }
    if ( size == 0 )
    {
        return 0;
    }
    int written = write_i2s_data( m_player_pos, size );
    if (written >= 0)
    {
        m_player_pos += written;
        m_size -= written;
        if ( m_player_pos == end )
        {
            m_player_pos = m_buffer;
        }
    #ifdef AUDIO_PLAYER_DEBUG
        for(int i=0; i<written; i++)
        {
            fprintf( stderr, "%02X ", m_player_pos[i] );
        }
        fprintf( stderr, "\n" );
    #endif
    }
    return written;
}

bool AudioPlayer::update()
{
    xSemaphoreTake( m_mutex, portMAX_DELAY );
    if (m_decoder == nullptr)
    {
        xSemaphoreGive( m_mutex );
        return false;
    }
    int played = 0, decoded = 0;
    do
    {
        played = play_data();
        if ( played < 0 )
        {
            delete m_decoder;
            m_decoder = nullptr;
            xSemaphoreGive( m_mutex );
            return false;
        }
        decoded = decode_data();
        if ( decoded < 0 )
        {
            delete m_decoder;
            m_decoder = nullptr;
            xSemaphoreGive( m_mutex );
            return false;
        }
    } while ( played > 0 || decoded > 0 );
    if ( m_size == 0 )
    {
        // to clear i2s dma buffer
        write_i2s_data( nullptr, 0 );
        delete m_decoder;
        m_decoder = nullptr;
        xSemaphoreGive( m_mutex );
        if ( m_on_play_complete )
        {
            m_on_play_complete();
        }
        return false;
    }
    xSemaphoreGive( m_mutex );
    return true;
}

