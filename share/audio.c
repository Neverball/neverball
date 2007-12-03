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

#include "text.h"
#include "config.h"
#include "audio.h"

/*---------------------------------------------------------------------------*/

static int audio_state = 0;

static char       name[MAXSND][MAXSTR];
static int        chan[MAXSND];
static Mix_Chunk *buff[MAXSND];
static Mix_Music *song;

static char  curr_bgm[MAXSTR];
static char  next_bgm[MAXSTR];

static float fade_volume = 1.0f;
static float fade_rate   = 0.0f;

/*---------------------------------------------------------------------------*/

void audio_init(void)
{
    int r = config_get_d(CONFIG_AUDIO_RATE);
    int b = config_get_d(CONFIG_AUDIO_BUFF);
    int i;

    memset(curr_bgm, 0, MAXSTR);
    memset(next_bgm, 0, MAXSTR);

    if (audio_state == 0)
    {
        if (Mix_OpenAudio(r, MIX_DEFAULT_FORMAT, 2, b) == 0)
        {
            for (i = 0; i < MAXSND; i++)
                if (chan[i])
                    buff[i] = Mix_LoadWAV(config_data(name[i]));

            audio_state = 1;

            audio_volume(config_get_d(CONFIG_SOUND_VOLUME),
                         config_get_d(CONFIG_MUSIC_VOLUME));
        }
        else
        {
            fprintf(stderr, L_("Sound disabled\n"));
            audio_state = 0;
        }
    }
}

void audio_free(void)
{
    int i;

    if (audio_state == 1)
    {
        for (i = 0; i < MAXSND; i++)
            if (buff[i])
            {
                Mix_FreeChunk(buff[i]);

                buff[i] = NULL;
            }

        Mix_CloseAudio();
        audio_state = 0;
    }
}

void audio_bind(int i, int c, const char *filename)
{
    strncpy(name[i], filename, MAXSTR);
    chan[i] = c;
}

void audio_play(int i, float v)
{
    if (audio_state == 1 && buff[i])
    {
        Mix_VolumeChunk(buff[i], (int) (v * MIX_MAX_VOLUME));
        Mix_PlayChannel(chan[i], buff[i], 0);
    }
}

/*---------------------------------------------------------------------------*/

void audio_music_play(const char *filename)
{
    if (audio_state)
    {
        audio_music_stop();

        if ((config_get_d(CONFIG_MUSIC_VOLUME) > 0) &&
            (song = Mix_LoadMUS(config_data(filename))))
        {
            Mix_PlayMusic(song, -1);
            strcpy(curr_bgm, filename);
        }
    }
}

void audio_music_queue(const char *filename)
{
    if (audio_state)
    {
        if (strlen(curr_bgm) == 0 || strcmp(filename, curr_bgm) != 0)
        {
            Mix_VolumeMusic(0);
            fade_volume = 0.0f;

            audio_music_play(filename);
            strcpy(curr_bgm, filename);

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

            if (strlen(next_bgm) == 0)
            {
                fade_rate = 0.0f;
                if (Mix_PlayingMusic())
                    Mix_PauseMusic();
            }
            else
            {
                fade_rate = -fade_rate;
                audio_music_queue(next_bgm);
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
            Mix_VolumeMusic(config_get_d(CONFIG_MUSIC_VOLUME) *
                            (int) (fade_volume * MIX_MAX_VOLUME) / 10);
    }
}

void audio_music_fade_out(float t)
{
    fade_rate = -t;
    strcpy(next_bgm, "");
}

void audio_music_fade_in(float t)
{
    fade_rate = +t;
    strcpy(next_bgm, "");
}

void audio_music_fade_to(float t, const char *filename)
{
    if (fade_volume > 0)
    {
        if (strlen(curr_bgm) == 0 || strcmp(filename, curr_bgm) != 0)
        {
            strcpy(next_bgm, filename);
            fade_rate = -t;
        }
        else fade_rate = t;
    }
    else
    {
        audio_music_queue(filename);
        audio_music_fade_in(t);
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
