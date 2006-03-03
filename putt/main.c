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

/*---------------------------------------------------------------------------*/

#ifdef WIN32
#pragma comment(lib, "SDL_ttf.lib")
#pragma comment(lib, "SDL_mixer.lib")
#pragma comment(lib, "SDL_image.lib")
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "opengl32.lib")
#endif

/*---------------------------------------------------------------------------*/

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "glext.h"
#include "audio.h"
#include "image.h"
#include "state.h"
#include "config.h"
#include "course.h"
#include "hole.h"
#include "game.h"
#include "gui.h"

#include "st_conf.h"
#include "st_all.h"

#define TITLE "Neverputt"

/*---------------------------------------------------------------------------*/

static int shot(void)
{
    static char filename[MAXSTR];
    static int  num = 0;

    sprintf(filename, "screen%02d.bmp", num++);

    image_snap(filename, config_get_d(CONFIG_WIDTH), config_get_d(CONFIG_HEIGHT));

    return 1;
}

/*---------------------------------------------------------------------------*/

static void toggle_wire(void)
{
    static int wire = 0;

    if (wire)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        wire = 0;
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        wire = 1;
    }
}
/*---------------------------------------------------------------------------*/

static int loop(void)
{
    SDL_Event e;
    int d = 1;

    while (d && SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            return 0;

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
            config_tgl_pause();

        if (!config_get_pause())
            switch (e.type)
            {
            case SDL_MOUSEMOTION:
                st_point(+e.motion.x,
                         -e.motion.y + config_get_d(CONFIG_HEIGHT),
                         +e.motion.xrel,
                         -e.motion.yrel);
                break;

            case SDL_MOUSEBUTTONDOWN:
                d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : 1, 1);
                break;

            case SDL_MOUSEBUTTONUP:
                d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : 1, 0);
                break;

            case SDL_KEYDOWN:
                switch (e.key.keysym.sym)
                {
                case SDLK_F10: d = shot();                break;
                case SDLK_F9:  config_tgl_d(CONFIG_FPS);  break;
                case SDLK_F8:  config_tgl_d(CONFIG_NICE); break;
                case SDLK_F7:  toggle_wire();             break;
                
                default:
                    d = st_keybd(e.key.keysym.sym, 1);
                }
                break;

            case SDL_ACTIVEEVENT:
                if (e.active.state == SDL_APPINPUTFOCUS)
                {
                    if (e.active.gain == 0)
                        config_set_pause();
                }
                break;
            }
    }
    return d;
}

int main(int argc, char *argv[])
{
    int camera = 0;
 
    srand((int) time(NULL));

    language_init("neverball", CONFIG_LOCALE);

    if (config_data_path((argc > 1 ? argv[1] : NULL), COURSE_FILE))
    {
        if (config_user_path(NULL))
        {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0)
            {
                config_init();
                config_load();

		/* Initialize the language. */
		
		language_set(language_from_code(config_simple_get_s(CONFIG_LANG)));

                /* Cache Neverball's camera setting. */

                camera = config_get_d(CONFIG_CAMERA);

                /* Initialize the audio. */

                audio_bind(AUD_BIRDIE,  1, "snd/birdie.ogg");
                audio_bind(AUD_BOGEY,   1, "snd/bogey.ogg");
                audio_bind(AUD_BUMP,    1, "snd/bink.wav");
                audio_bind(AUD_DOUBLE,  1, "snd/double.ogg");
                audio_bind(AUD_EAGLE,   1, "snd/eagle.ogg");
                audio_bind(AUD_JUMP,    2, "snd/jump.ogg");
                audio_bind(AUD_MENU,    2, "snd/menu.wav");
                audio_bind(AUD_ONE,     1, "snd/one.ogg");
                audio_bind(AUD_PAR,     1, "snd/par.ogg");
                audio_bind(AUD_PENALTY, 1, "snd/penalty.ogg");
                audio_bind(AUD_PLAYER1, 1, "snd/player1.ogg");
                audio_bind(AUD_PLAYER2, 1, "snd/player2.ogg");
                audio_bind(AUD_PLAYER3, 1, "snd/player3.ogg");
                audio_bind(AUD_PLAYER4, 1, "snd/player4.ogg");
                audio_bind(AUD_SWITCH,  2, "snd/switch.wav");
                audio_bind(AUD_SUCCESS, 1, "snd/success.ogg");

                audio_init();

                /* Require 16-bit double buffer with 16-bit depth buffer. */

                SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
                SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

                /* Initialize the video. */

                if (config_mode(config_get_d(CONFIG_FULLSCREEN),
                                config_get_d(CONFIG_WIDTH),
                                config_get_d(CONFIG_HEIGHT)))
                {
                    int t1, t0 = SDL_GetTicks();

		    SDL_Surface *icon = IMG_Load(config_data("icon/neverputt.png"));
		    SDL_WM_SetIcon(icon, NULL);
                    SDL_WM_SetCaption(TITLE, TITLE); 

                    /* Run the main game loop. */

                    init_state(&st_null);
                    goto_state(&st_title);

                    while (loop())
                        if ((t1 = SDL_GetTicks()) > t0)
                        {
                           if (config_get_pause())
                            {
                                st_paint();
                                gui_blank();
                            }
                            else
                            {
                                st_timer((t1 - t0) / 1000.f);
                                st_paint();
                            }
                            SDL_GL_SwapBuffers();

                            t0 = t1;

                            if (config_get_d(CONFIG_NICE))
                                SDL_Delay(1);
                        }
                }
                else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());

                /* Restore Neverball's camera setting. */

                config_set_d(CONFIG_CAMERA, camera);
                config_save();

                SDL_Quit();
            }
            else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());
        }
        else fprintf(stderr, _("Failure to establish config directory\n"));
    }
    else fprintf(stderr, _("Failure to establish game data directory\n"));

    return 0;
}

/*---------------------------------------------------------------------------*/

