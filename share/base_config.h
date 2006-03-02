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

#ifndef BASE_CONFIG_H
#define BASE_CONFIG_H

/*
 * This file contains some constants
 * And the way to access to directories
 */

#include <stdio.h>
#include "i18n.h"

/*---------------------------------------------------------------------------*/

#define VERSION       "1.4.1svn"      /* Game version */

#define CONFIG_DATA   "./data"        /* Game data directory */
#define CONFIG_LOCALE "./locale"      /* Game localisation */
#define CONFIG_USER   ".neverball"    /* User config directory */

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

#define JOY_MAX 32767
#define JOY_MID 16383

#define MAXSTR 256
#define MAXLVL 26
#define MAXNAM 9

#define GUI_FACE                    _("ttf/DejaVuSans-Bold.ttf")

/*---------------------------------------------------------------------------*/

const char *config_data(const char *);
const char *config_user(const char *);

int  config_data_path(const char *, const char *);
int  config_user_path(const char *);

/*---------------------------------------------------------------------------*/

#endif
