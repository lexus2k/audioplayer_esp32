# audioplayer_esp32
Audio I2S player for ESP32

## Easy to use

Example
```.cpp
#include "audio_player.h"
#include "nixie_melodies.h"

// Let's use 16kHz sound
AudioPlayer audio_player( 16000 );

static void main_task(void *pvParameter)
{
    /* set prebuffering in milliseconds. It is require, when thread sleeps */
    audio_player.set_prebuffering( 50 );
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
