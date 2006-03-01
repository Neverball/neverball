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

#ifndef CONFIG_H
#define CONFIG_H

#include <SDL.h>
#include <stdio.h>
#include "i18n.h"

/*---------------------------------------------------------------------------*/

#define CONFIG_DATA "./data"
#define CONFIG_LOCALE "./locale"
#define CONFIG_USER ".neverball"

/*
 * Global settings are stored in USER_CONFIG_FILE.  Replays are stored
 * in  USER_REPLAY_FILE.  These files  are placed  in the  user's home
 * directory as given by the HOME environment var.  If the config file
 * is deleted, it will be recreated using the defaults.
 */
#define USER_CONFIG_FILE "neverballrc"
#define USER_REPLAY_FILE _("Last")
#define ALLOW_CHEAT	 1

/*---------------------------------------------------------------------------*/

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xFF000000
#define GMASK 0x00FF0000
#define BMASK 0x0000FF00
#define AMASK 0x000000FF
#else
#define RMASK 0x000000FF
#define GMASK 0x0000FF00
#define BMASK 0x00FF0000
#define AMASK 0xFF000000
#endif

#ifdef _WIN32
#define FMODE_RB "rb"
#define FMODE_WB "wb"
#else
#define FMODE_RB "r"
#define FMODE_WB "w"
#endif

#define AUDIO_BUFF_HI 2048
#define AUDIO_BUFF_LO 1024

/*---------------------------------------------------------------------------*/

enum {
    CONFIG_FULLSCREEN,
    CONFIG_WIDTH,
    CONFIG_HEIGHT,
    CONFIG_STEREO,
    CONFIG_CAMERA,
    CONFIG_TEXTURES,
    CONFIG_GEOMETRY,
    CONFIG_REFLECTION,
    CONFIG_BACKGROUND,
    CONFIG_SHADOW,
    CONFIG_AUDIO_RATE,
    CONFIG_AUDIO_BUFF,
    CONFIG_MOUSE_SENSE,
    CONFIG_MOUSE_INVERT,
    CONFIG_NICE,
    CONFIG_FPS,
    CONFIG_SOUND_VOLUME,
    CONFIG_MUSIC_VOLUME,
    CONFIG_JOYSTICK,
    CONFIG_JOYSTICK_DEVICE,
    CONFIG_JOYSTICK_AXIS_X,
    CONFIG_JOYSTICK_AXIS_Y,
    CONFIG_JOYSTICK_BUTTON_A,
    CONFIG_JOYSTICK_BUTTON_B,
    CONFIG_JOYSTICK_BUTTON_R,
    CONFIG_JOYSTICK_BUTTON_L,
    CONFIG_JOYSTICK_BUTTON_EXIT,
    CONFIG_JOYSTICK_CAMERA_1,
    CONFIG_JOYSTICK_CAMERA_2,
    CONFIG_JOYSTICK_CAMERA_3,
    CONFIG_KEY_CAMERA_1,
    CONFIG_KEY_CAMERA_2,
    CONFIG_KEY_CAMERA_3,
    CONFIG_KEY_CAMERA_R,
    CONFIG_KEY_CAMERA_L,
    CONFIG_VIEW_FOV,
    CONFIG_VIEW_DP,
    CONFIG_VIEW_DC,
    CONFIG_VIEW_DZ,
    CONFIG_ROTATE_FAST,
    CONFIG_ROTATE_SLOW,
    CONFIG_LAST_SET,
    CONFIG_MODE,
    CONFIG_CHEAT,

    CONFIG_OPTION_D_COUNT
};

enum {
    CONFIG_PLAYER,
    CONFIG_BALL,
    CONFIG_COIN,
    CONFIG_LANG,

    CONFIG_OPTION_S_COUNT
};

/*---------------------------------------------------------------------------*/

#define DEFAULT_FULLSCREEN           0
#define DEFAULT_WIDTH                800
#define DEFAULT_HEIGHT               600
#define DEFAULT_STEREO               0
#define DEFAULT_CAMERA               0
#define DEFAULT_TEXTURES             1
#define DEFAULT_GEOMETRY             1
#define DEFAULT_REFLECTION           1
#define DEFAULT_BACKGROUND           1
#define DEFAULT_SHADOW               1
#define DEFAULT_AUDIO_RATE           44100
#define DEFAULT_AUDIO_BUFF           AUDIO_BUFF_HI
#define DEFAULT_MOUSE_SENSE          300
#define DEFAULT_MOUSE_INVERT         0
#define DEFAULT_NICE                 1
#define DEFAULT_FPS                  0
#define DEFAULT_SOUND_VOLUME         10
#define DEFAULT_MUSIC_VOLUME         6
#define DEFAULT_JOYSTICK             0
#define DEFAULT_JOYSTICK_DEVICE      0
#define DEFAULT_JOYSTICK_AXIS_X      0
#define DEFAULT_JOYSTICK_AXIS_Y      1
#define DEFAULT_JOYSTICK_BUTTON_A    0
#define DEFAULT_JOYSTICK_BUTTON_B    1
#define DEFAULT_JOYSTICK_BUTTON_R    2
#define DEFAULT_JOYSTICK_BUTTON_L    3
#define DEFAULT_JOYSTICK_BUTTON_EXIT 4
#define DEFAULT_JOYSTICK_CAMERA_1    5
#define DEFAULT_JOYSTICK_CAMERA_2    6
#define DEFAULT_JOYSTICK_CAMERA_3    7
#define DEFAULT_KEY_CAMERA_1         SDLK_F1
#define DEFAULT_KEY_CAMERA_2         SDLK_F2
#define DEFAULT_KEY_CAMERA_3         SDLK_F3
#define DEFAULT_KEY_CAMERA_R         SDLK_RIGHT
#define DEFAULT_KEY_CAMERA_L         SDLK_LEFT
#define DEFAULT_VIEW_FOV             50
#define DEFAULT_VIEW_DP              75
#define DEFAULT_VIEW_DC              25
#define DEFAULT_VIEW_DZ              200
#define DEFAULT_ROTATE_SLOW          100
#define DEFAULT_ROTATE_FAST          200
#define DEFAULT_LAST_SET             0
#define DEFAULT_MODE                 1
#define DEFAULT_CHEAT                0
#define DEFAULT_PLAYER               ""
#define DEFAULT_BALL                 "png/ball.png"
#define DEFAULT_COIN                 _("png/coin.png")
#define DEFAULT_LANG                 ""

/*---------------------------------------------------------------------------*/

#define JOY_MAX 32767
#define JOY_MID 16383

#define MAXSTR 256
#define MAXLVL 26
#define MAXNAM 9

#define GUI_FACE                    _("ttf/DejaVuSans-Bold.ttf")

/*---------------------------------------------------------------------------*/

void config_init(void);
void config_load(void);
void config_save(void);
int  config_mode(int, int, int);

const char *config_data(const char *);
const char *config_user(const char *);

int  config_data_path(const char *, const char *);
int  config_user_path(const char *);

/*---------------------------------------------------------------------------*/

void config_set_d(int, int);
void config_tgl_d(int);
int  config_tst_d(int, int);
int  config_get_d(int);

void config_set_s(int, const char *);
void config_get_s(int, char *, int);
const char * config_simple_get_s(int i);
	

/*---------------------------------------------------------------------------*/

void config_set_grab(void);
void config_clr_grab(void);
int  config_get_grab(void);

int  config_get_pause(void);
void config_set_pause(void);
void config_clr_pause(void);
void config_tgl_pause(void);

/*---------------------------------------------------------------------------*/

void config_push_persp(float, float, float);
void config_push_ortho(void);
void config_pop_matrix(void);
void config_clear(void);

/*---------------------------------------------------------------------------*/

#endif
