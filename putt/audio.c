/*   
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERPUTT is  free software; you can redistribute  it and/or modify
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

#include "config.h"
#include "audio.h"

/*---------------------------------------------------------------------------*/

static int audio_state = 0;

static Mix_Music *song;
static Mix_Chunk *buff[AUD_COUNT];
static int        chan[AUD_COUNT];

#define CH_MENU  0
#define CH_TICK  1
#define CH_BUMP  2
#define CH_VOICE 3
#define CH_COUNT 4

/*---------------------------------------------------------------------------*/

static void chunk_load(int i, const char *filename, int channel)
{
    buff[i] = Mix_LoadWAV(config_data(filename));
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

void audio_init(void)
{
    int r = config_get(CONFIG_AUDIO_RATE);
    int b = config_get(CONFIG_AUDIO_BUFF);

    if (audio_state == 0)
    {
        if (Mix_OpenAudio(r, MIX_DEFAULT_FORMAT, 1, b) == 0)
        {
            chunk_load(AUD_BIRDIE,   "snd/birdie.ogg",   CH_VOICE);
            chunk_load(AUD_BOGEY,    "snd/bogey.ogg",    CH_VOICE);
            chunk_load(AUD_BUMP,     "snd/bink.wav",     CH_VOICE);
            chunk_load(AUD_DOUBLE,   "snd/double.ogg",   CH_VOICE);
            chunk_load(AUD_EAGLE,    "snd/eagle.ogg",    CH_VOICE);
            chunk_load(AUD_JUMP,     "snd/jump.ogg",     CH_BUMP);
            chunk_load(AUD_MENU,     "snd/menu.wav",     CH_MENU);
            chunk_load(AUD_ONE,      "snd/one.ogg",      CH_VOICE);
            chunk_load(AUD_PAR,      "snd/par.ogg",      CH_VOICE);
            chunk_load(AUD_PENALTY,  "snd/penalty.ogg",  CH_VOICE);
            chunk_load(AUD_PLAYER1,  "snd/player1.ogg",  CH_VOICE);
            chunk_load(AUD_PLAYER2,  "snd/player2.ogg",  CH_VOICE);
            chunk_load(AUD_PLAYER3,  "snd/player3.ogg",  CH_VOICE);
            chunk_load(AUD_PLAYER4,  "snd/player4.ogg",  CH_VOICE);
            chunk_load(AUD_SUCCESS,  "snd/success.ogg",  CH_VOICE);

            audio_state = 1;

            audio_volume(config_get(CONFIG_SOUND_VOLUME),
                         config_get(CONFIG_MUSIC_VOLUME));
        }
        else fprintf(stderr, "Sound disabled\n");
    }
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

        chunk_free(AUD_SUCCESS);
        chunk_free(AUD_PLAYER4);
        chunk_free(AUD_PLAYER3);
        chunk_free(AUD_PLAYER2);
        chunk_free(AUD_PLAYER1);
        chunk_free(AUD_PENALTY);
        chunk_free(AUD_PAR);
        chunk_free(AUD_ONE);
        chunk_free(AUD_MENU);
        chunk_free(AUD_JUMP);
        chunk_free(AUD_EAGLE);
        chunk_free(AUD_DOUBLE);
        chunk_free(AUD_BUMP);
        chunk_free(AUD_BOGEY);
        chunk_free(AUD_BIRDIE);

        audio_state = 0;
    }
}

/*---------------------------------------------------------------------------*/

void audio_music_play(const char *filename)
{
    if (audio_state)
    {
        audio_music_stop();

        if ((config_get(CONFIG_MUSIC_VOLUME) > 0) &&
            (song = Mix_LoadMUS(config_data(filename))))
            Mix_PlayMusic(song, -1);
    }
}

void audio_music_fade(float t)
{
    if (audio_state && song && Mix_PlayingMusic())
        Mix_FadeOutMusic((int) (t * 1000.f));
}

void audio_music_stop(void)
{
    if (audio_state)
    {
        if (Mix_PlayingMusic())
            Mix_HaltMusic();

        if (song)
            Mix_FreeMusic(song);

        song = NULL;
    }
}

void audio_volume(int s, int m)
{
    if (audio_state)
    {
        Mix_Volume(-1, s * MIX_MAX_VOLUME / 10);
        Mix_VolumeMusic(m * MIX_MAX_VOLUME / 10);
    }
}

/*---------------------------------------------------------------------------*/
