/*
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include <emscripten.h>

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "config.h"
#include "audio.h"
#include "common.h"
#include "fs.h"
#include "log.h"

/*---------------------------------------------------------------------------*/

#define LOG_VOLUME(v) ((float) pow((double) (v), 2.0))

/*---------------------------------------------------------------------------*/

void audio_init(void)
{
    /* Initialize web audio. */

    EM_ASM({
        Neverball.audioInit();
    });

    /* Set the initial volumes. */

    audio_volume(config_get_d(CONFIG_SOUND_VOLUME),
                 config_get_d(CONFIG_MUSIC_VOLUME));
}

void audio_free(void)
{
    EM_ASM({
        Neverball.audioQuit();
    });
}

void audio_play(const char *filename, float a)
{
    int size = 0;
    unsigned char *data = fs_load_cache(filename, &size);

    if (data)
    {
        // Play the file data.

        EM_ASM({
            const fileName = UTF8ToString($0);
            const data = $1;
            const size = $2;
            const a = $3;

            const fileData = Module.HEAP8.buffer.slice(data, data + size);

            Neverball.audioPlay(fileName, fileData, a);
        }, filename, data, size, LOG_VOLUME(CLAMP(0.0f, a, 1.0f)));
    }
}

/*---------------------------------------------------------------------------*/

void audio_music_fade_out(float t)
{
    EM_ASM({
        Neverball.audioMusicFadeOut($0);
    }, t);
}

void audio_music_fade_in(float t)
{
    EM_ASM({
        Neverball.audioMusicFadeIn($0);
    }, t);
}

void audio_music_fade_to(float t, const char *filename)
{
    int size = 0;
    unsigned char *data = fs_load_cache(filename, &size);

    if (data)
    {
        // Play the file data.

        EM_ASM({
            const fileName = UTF8ToString($0);
            const data = $1;
            const size = $2;
            const t = $3;

            const fileData = Module.HEAP8.buffer.slice(data, data + size);

            Neverball.audioMusicFadeTo(fileName, fileData, t);
        }, filename, data, size, t);
    }
}

void audio_music_stop(void)
{
    EM_ASM({
        Neverball.audioMusicStop();;
    });
}

/*---------------------------------------------------------------------------*/

/*
 * Logarithmic volume control.
 */
void audio_volume(int s, int m)
{
    float sl = (float) s / 10.0f;
    float ml = (float) m / 10.0f;

    float sound_vol = LOG_VOLUME(sl);
    float music_vol = LOG_VOLUME(ml);

    EM_ASM({
        Neverball.audioVolume($0, $1);
    }, sound_vol, music_vol);
}

/*---------------------------------------------------------------------------*/
