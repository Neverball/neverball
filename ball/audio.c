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
#include <string.h>

#include "config.h"
#include "audio.h"

/*---------------------------------------------------------------------------*/

static int audio_state = 0;

static Mix_Music *song;
static Mix_Chunk *buff[AUD_COUNT];
static int        chan[AUD_COUNT];

#define CH_STATE 0
#define CH_MENU  1
#define CH_GRAB  2
#define CH_TICK  3
#define CH_BUMP  4
#define CH_VOICE 5
#define CH_COUNT 6

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

void audio_init(void)
{
    int r = config_get(CONFIG_AUDIO_RATE);
    int b = config_get(CONFIG_AUDIO_BUFF);

    if (audio_state == 0)
    {
        if (Mix_OpenAudio(r, MIX_DEFAULT_FORMAT, 1, b) == 0)
        {
            chunk_load(AUD_MENU,   "snd/menu.wav",   CH_MENU);
            chunk_load(AUD_START,  "snd/select.ogg", CH_VOICE);
            chunk_load(AUD_READY,  "snd/ready.ogg",  CH_VOICE);
            chunk_load(AUD_SET,    "snd/set.ogg",    CH_VOICE);
            chunk_load(AUD_GO,     "snd/go.ogg",     CH_VOICE);
            chunk_load(AUD_BALL,   "snd/ball.ogg",   CH_GRAB);
            chunk_load(AUD_BUMP,   "snd/bump.ogg",   CH_BUMP);
            chunk_load(AUD_COIN,   "snd/coin.wav",   CH_GRAB);
            chunk_load(AUD_TICK,   "snd/tick.ogg",   CH_TICK);
            chunk_load(AUD_TOCK,   "snd/tock.ogg",   CH_TICK);
            chunk_load(AUD_SWITCH, "snd/switch.wav", CH_STATE);
            chunk_load(AUD_JUMP,   "snd/jump.ogg",   CH_STATE);
            chunk_load(AUD_GOAL,   "snd/goal.wav",   CH_STATE);
            chunk_load(AUD_SCORE,  "snd/record.ogg", CH_VOICE);
            chunk_load(AUD_FALL,   "snd/fall.ogg",   CH_VOICE);
            chunk_load(AUD_TIME,   "snd/time.ogg",   CH_VOICE);
            chunk_load(AUD_OVER,   "snd/over.ogg",   CH_VOICE);

            audio_state = 1;

            audio_volume(config_get(CONFIG_SOUND_VOLUME),
                         config_get(CONFIG_MUSIC_VOLUME));
        }
        else
        {
            fprintf(stderr, "Sound disabled\n");
            audio_state = 0;
        }
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

        chunk_free(AUD_OVER);
        chunk_free(AUD_TIME);
        chunk_free(AUD_FALL);
        chunk_free(AUD_SCORE);
        chunk_free(AUD_GOAL);
        chunk_free(AUD_JUMP);
        chunk_free(AUD_SWITCH);
        chunk_free(AUD_TOCK);
        chunk_free(AUD_TICK);
        chunk_free(AUD_COIN);
        chunk_free(AUD_BUMP);
        chunk_free(AUD_BALL);
        chunk_free(AUD_GO);
        chunk_free(AUD_SET);
        chunk_free(AUD_READY);
        chunk_free(AUD_START);
        chunk_free(AUD_MENU);

        audio_state = 0;
    }
}

/*---------------------------------------------------------------------------*/

static const char *current = NULL;
static const char *next    = NULL;

static float fade_volume = 1.0f;
static float fade_rate   = 0.0f;

void audio_music_play(const char *filename)
{
    if (audio_state)
    {
        audio_music_stop();

        if ((config_get(CONFIG_MUSIC_VOLUME) > 0) &&
            (song = Mix_LoadMUS(filename)))
        {
            Mix_PlayMusic(song, -1);
            current = filename;
        }
    }
}

void audio_music_queue(const char *filename)
{
    if (audio_state)
    {
        if (!current || strcmp(filename, current) != 0)
        {
            Mix_VolumeMusic(0);
            fade_volume = 0.0f;

            audio_music_play(filename);
            current = filename;

            Mix_PauseMusic();
        }
    }
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

/*---------------------------------------------------------------------------*/
/*
 * SDL_mixer already provides music fading.  Unfortunately, it halts playback
 * at the end of a fade.  We need to be able to fade music back in from the
 * point where it stopped.  So, we reinvent this wheel.
 */

void audio_timer(float dt)
{
    if (audio_state)
    {
        if (fade_rate > 0.0f || fade_rate < 0.0f)
            fade_volume += dt / fade_rate;

        if (fade_volume < 0.0f)
        {
            fade_volume = 0.0f;

            if (next == NULL)
            {
                fade_rate = 0.0f;
                if (Mix_PlayingMusic())
                    Mix_PauseMusic();
            }
            else
            {
                fade_rate = -fade_rate;
                audio_music_queue(next);
            }

        }

        if (fade_volume > 1.0f)
        {
            fade_rate   = 0.0f;
            fade_volume = 1.0f;
        }   

        if (Mix_PausedMusic() && fade_rate > 0.0f)
            Mix_ResumeMusic();
            
        if (Mix_PlayingMusic())
            Mix_VolumeMusic(config_get(CONFIG_MUSIC_VOLUME) *
                            (int) (fade_volume * MIX_MAX_VOLUME) / 10);
    }
}

void audio_music_fade_out(float t)
{
    fade_rate = -t;
    next = NULL;
}

void audio_music_fade_in(float t)
{
    fade_rate = +t;
    next = NULL;
}

void audio_music_fade_to(float t, const char *filename)
{
    if (!current || strcmp(filename, current) != 0)
    {
        fade_rate = -t;
        next = filename;
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
