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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glext.h"
#include "audio.h"
#include "image.h"
#include "state.h"
#include "config.h"
#include "hole.h"

#define TITLE "Neverputt"

/*---------------------------------------------------------------------------*/

static int shot(void)
{
    static char filename[MAXSTR];
    static int  num = 0;

    sprintf(filename, "screen%02d.bmp", num++);

    image_snap(filename);

    return 1;
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
                d = st_point(+e.motion.x,
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
                
                default:
                    d = st_keybd(e.key.keysym.sym);
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

    if (config_data_path((argc > 1 ? argv[1] : NULL), HOLE_FILE))
    {
        if (config_user_path(NULL))
        {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0)
            {
                config_init();
                config_load();

                /* Cache Neverball's camera setting. */

                camera = config_get_d(CONFIG_CAMERA);

                /* Initialize the audio. */

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

                    SDL_WM_SetCaption(TITLE, TITLE); 

                    /* Run the main game loop. */

                    goto_state(&st_title);

                    while (loop())
                        if ((t1 = SDL_GetTicks()) > t0)
                        {
                            if (!config_get_pause())
                                st_timer((t1 - t0) / 1000.f);

                            st_paint();
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
        else fprintf(stderr, "Failure to establish config directory\n");
    }
    else fprintf(stderr, "Failure to establish game data directory\n");

    return 0;
}

/*---------------------------------------------------------------------------*/

