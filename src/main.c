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
#include <SDL_ttf.h>
#include <string.h>
#include <stdio.h>

#include "gl.h"
#include "state.h"
#include "config.h"

#define TITLE "Neverball"

/*---------------------------------------------------------------------------*/

static int shot(void)
{
    static char path[STRMAX];
    static char file[32];
    static int  num = 0;

    int i;
    int w = config_w();
    int h = config_h();

    SDL_Surface *buf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24,
                                            RMASK, GMASK, BMASK, 0);
    SDL_Surface *img = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24,
                                            RMASK, GMASK, BMASK, 0);

    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buf->pixels);

    for (i = 0; i < h; i++)
        memcpy((GLubyte *) img->pixels + 3 * w * i,
               (GLubyte *) buf->pixels + 3 * w * (h - i), 3 * w);

    sprintf(file, "screen%02d.bmp", num++);

    if (config_home(path, file))
        SDL_SaveBMP(img,  path);

    SDL_FreeSurface(img);
    SDL_FreeSurface(buf);

    return 1;
}

/*---------------------------------------------------------------------------*/

static int loop(void)
{
    SDL_Event e;
    int d = 1;

    while (d && SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_MOUSEMOTION:
            d = st_point(+e.motion.x,
                         -e.motion.y + config_h(),
                         +e.motion.xrel,
                         -e.motion.yrel);
              break;

        case SDL_MOUSEBUTTONDOWN:
            d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : +1, 1);
            break;

        case SDL_MOUSEBUTTONUP:
            d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : +1, 0);
            break;

        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_F10) { d = shot();        break; }
            if (e.key.keysym.sym == SDLK_F9)  { config_tog_fps();  break; }
            if (e.key.keysym.sym == SDLK_F8)  { config_tog_nice(); break; }

            d = st_keybd(e.key.keysym.sym);
            break;

        case SDL_JOYAXISMOTION:
            d = st_stick(e.jaxis.axis, e.jaxis.value);
            break;

        case SDL_JOYBUTTONDOWN:
            d = st_buttn(e.jbutton.button, 1);
            break;

        case SDL_JOYBUTTONUP:
            d = st_buttn(e.jbutton.button, 0);
            break;

        case SDL_QUIT:
            d = 0;
        }

    return d;
}

int main(int argc, char *argv[])
{
    SDL_Joystick *joy = NULL;

    if (config_path())
    {
        config_load();
    
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) == 0)
        {
            /* Initialize the first connected joystick. */

            if (SDL_NumJoysticks() > 0)
            {
                if ((joy = SDL_JoystickOpen(0)))
                    SDL_JoystickEventState(SDL_ENABLE);
            }

            /* Initialize the audio. */

            if (config_set_audio(config_rate(), config_buff()))
            {
                /* Require 16-bit double buffer with 16-bit depth buffer. */

                SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
                SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
                SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

                /* Initialize the video. */

                if (config_set_mode(config_w(), config_h(), config_mode()))
                {
                    int t1, t0 = SDL_GetTicks();

                    SDL_WM_SetCaption(TITLE, TITLE);

                    /* Run the main game loop. */

                    goto_state(&st_title);

                    while (loop())
                        if ((t1 = SDL_GetTicks()) > t0)
                        {
                            st_timer((t1 - t0) / 1000.0);
                            st_paint();

                            SDL_GL_SwapBuffers();
                            if (config_nice())
                                SDL_Delay(1);
                        
                            t0 = t1;
                        }
                }
                else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());
            }

            if (SDL_JoystickOpened(0))
                SDL_JoystickClose(joy);

            SDL_Quit();
        }
        else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());

        config_store();
    }
    else fprintf(stderr, "%s: Can't find data directory\n", argv[0]);

    return 0;
}

/*---------------------------------------------------------------------------*/

