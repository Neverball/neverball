/*   
 *   Copyright (C) 2003 Robert Kooima
 *
 *   SUPER EMPTY BALL is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by the
 *   Free Software Foundation;  either version 2 of the  License, or (at your
 *   option) any later version.
 *
 *   This program  is distributed  in the  hope that it  will be  useful, but
 *   WITHOUT   ANY   WARRANTY;  without   even   the   implied  warranty   of
 *   MERCHANTABILITY  or  FITNESS FOR  A  PARTICULAR  PURPOSE.   See the  GNU
 *   General Public License for more details.
 */

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include "audio.h"

/*---------------------------------------------------------------------------*/

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
    if (Mix_OpenAudio(r, MIX_DEFAULT_FORMAT, 1, b) == 0)
    {
        Mix_AllocateChannels(CH_COUNT);

        chunk_load(AUD_TITLE, "data/wav/title.wav", CH_STATE);
        chunk_load(AUD_MENU,  "data/wav/menu.wav",  CH_MENU);
        chunk_load(AUD_LEVEL, "data/wav/level.wav", CH_STATE);
        chunk_load(AUD_READY, "data/wav/ready.wav", CH_STATE);
        chunk_load(AUD_SET,   "data/wav/set.wav",   CH_STATE);
        chunk_load(AUD_GO,    "data/wav/go.wav",    CH_STATE);
        chunk_load(AUD_BALL,  "data/wav/ball.wav",  CH_GRAB);
        chunk_load(AUD_BUMP,  "data/wav/bump.wav",  CH_BUMP);
        chunk_load(AUD_COIN,  "data/wav/coin.wav",  CH_GRAB);
        chunk_load(AUD_TICK,  "data/wav/tick.wav",  CH_TICK);
        chunk_load(AUD_GOAL,  "data/wav/goal.wav",  CH_STATE);
        chunk_load(AUD_FALL,  "data/wav/fail.wav",  CH_STATE);
        chunk_load(AUD_TIME,  "data/wav/fail.wav",  CH_STATE);
        chunk_load(AUD_OVER,  "data/wav/over.wav",  CH_STATE);
        chunk_load(AUD_PAUSE, "data/wav/pause.wav", CH_STATE);

        return 1;
    }
    return 0;
}

void audio_play(int i, float v)
{
    Mix_VolumeChunk(buff[i], (int) (v * MIX_MAX_VOLUME));
    Mix_PlayChannel(chan[i], buff[i], 0);
}

void audio_free(void)
{
    Mix_CloseAudio();

    chunk_free(AUD_PAUSE);
    chunk_free(AUD_OVER);
    chunk_free(AUD_TIME);
    chunk_free(AUD_FALL);
    chunk_free(AUD_GOAL);
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
}

/*---------------------------------------------------------------------------*/
