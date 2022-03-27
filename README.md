# audioplayer_esp32
Audio I2S player for ESP32

To use vgm files please, download this library also: https://github.com/lexus2k/vgm_decoder
AudioPlayer needs it to play vgm files

Demo with playing vgm files on ESP32 is here: https://github.com/lexus2k/vgm_test

## Easy to use

Example
```.cpp
#include "audio_player.h"
#include "nixie_melodies.h"

// Let's use 16kHz sound
AudioPlayer audio_player( 16000 );

static void main_task(void *pvParameter)
{
    /* set prebuffering in milliseconds. It is required, when thread sleeps */
    audio_player.set_prebuffering( 50 );
    // Use audio_player.begin(EAudioChannels::BOTH, false) to use with external I2S decoder
    // Use audio_player.begin(EAudioChannels::BOTH, true) or audio_player.begin() to use with internal built-in I2S DAC decoder
    audio_player.begin();
    audio_player.play( &melodyMonkeyIslandP );

    for(;;)
    {
        audio_player.update();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    audio_player.end();
}

```
