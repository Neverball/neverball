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

#include <SDL.h>
#include <SDL_mixer.h>

#include "audio.h"

/*---------------------------------------------------------------------------*/

static int audio_state = 0;

static Mix_Chunk *buff[AUD_COUNT];
static int        chan[AUD_COUNT];

#define CH_STATE 0
#define CH_MENU  1
#define CH_GRAB  2
#define CH_TICK  3
#define CH_BUMP  4
#define CH_COUNT 5

/*---------------------------------------------------------------------------*/

static void chunk_load(int i, const char *filename, int channel)
{
    buff[i] = Mix_LoadWAV(filename);
    chan[i] = channel;
}

static void chunk_free(int i)
{
    if (buff[i])
    {
        Mix_FreeChunk(buff[i]);
        buff[i] = NULL;
    }
}

/*---------------------------------------------------------------------------*/

int audio_init(int r, int b)
{
    if (audio_state == 0)
    {
        if (Mix_OpenAudio(r, MIX_DEFAULT_FORMAT, 1, b) == 0)
        {
            chunk_load(AUD_TITLE, "snd/title.ogg", CH_STATE);
            chunk_load(AUD_MENU,  "snd/menu.wav",  CH_MENU);
            chunk_load(AUD_LEVEL, "snd/level.ogg", CH_STATE);
            chunk_load(AUD_READY, "snd/ready.ogg", CH_STATE);
            chunk_load(AUD_SET,   "snd/set.ogg",   CH_STATE);
            chunk_load(AUD_GO,    "snd/go.ogg",    CH_STATE);
            chunk_load(AUD_BALL,  "snd/ball.ogg",  CH_GRAB);
            chunk_load(AUD_BUMP,  "snd/bump.ogg",  CH_BUMP);
            chunk_load(AUD_COIN,  "snd/coin.wav",  CH_GRAB);
            chunk_load(AUD_TICK,  "snd/tick.wav",  CH_TICK);
            chunk_load(AUD_JUMP,  "snd/jump.ogg",  CH_STATE);
            chunk_load(AUD_GOAL,  "snd/goal.ogg",  CH_STATE);
            chunk_load(AUD_FALL,  "snd/fail.ogg",  CH_STATE);
            chunk_load(AUD_TIME,  "snd/fail.ogg",  CH_STATE);
            chunk_load(AUD_OVER,  "snd/over.ogg",  CH_STATE);
            chunk_load(AUD_PAUSE, "snd/pause.ogg", CH_STATE);

            audio_state = 1;

            return 1;
        }
        return 0;
    }
    return 1;
}

void audio_play(int i, float v)
{
    if (audio_state == 1 && buff[i])
    {
        Mix_VolumeChunk(buff[i], (int) (v * MIX_MAX_VOLUME));
        Mix_PlayChannel(chan[i], buff[i], 0);
    }
}

void audio_free(void)
{
    if (audio_state == 1)
    {
        Mix_CloseAudio();

        chunk_free(AUD_PAUSE);
        chunk_free(AUD_OVER);
        chunk_free(AUD_TIME);
        chunk_free(AUD_FALL);
        chunk_free(AUD_GOAL);
        chunk_free(AUD_JUMP);
        chunk_free(AUD_TICK);
        chunk_free(AUD_COIN);
        chunk_free(AUD_BUMP);
        chunk_free(AUD_BALL);
        chunk_free(AUD_GO);
        chunk_free(AUD_SET);
        chunk_free(AUD_LEVEL);
        chunk_free(AUD_READY);
        chunk_free(AUD_MENU);
        chunk_free(AUD_TITLE);

        audio_state = 0;
    }
}

/*---------------------------------------------------------------------------*/
