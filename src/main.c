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
#include <string.h>

#include "vec3.h"
#include "game.h"
#include "solid.h"
#include "state.h"

/*---------------------------------------------------------------------------*/

#define TITLE "SUPER EMPTY BALL"

int mode   = SDL_OPENGL;
int width  = 1280;
int height = 600;

/*---------------------------------------------------------------------------*/

static int size(int w, int h, int m)
{
    glViewport(0, 0, w, h);

    width  = w;
    height = h;

    return SDL_SetVideoMode(w, h, 0, m) ? 1 : 0;
}

static int loop(void)
{
    SDL_Event e;
    int d = 1;

    while (d && SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_VIDEORESIZE:
            d = size(e.resize.w, e.resize.h, mode);
            break;
        case SDL_MOUSEMOTION:
            d = st_point(e.motion.x, e.motion.y);
            break;
        case SDL_MOUSEBUTTONDOWN:
            d = st_click(1);
            break;
        case SDL_MOUSEBUTTONUP:
            d = st_click(0);
            break;
        case SDL_KEYDOWN:
            d = st_keybd(e.key.keysym.sym);
            break;
        case SDL_QUIT:
            d = 0;
        }

    return d;
}

int main(int argc, char *argv[])
{
    int argi;

    for (argi = 1; argi < argc; argi++)
    {
        if (strcmp(argv[argi], "-fs") == 0) mode |= SDL_FULLSCREEN;
        if (strcmp(argv[argi], "-rs") == 0) mode |= SDL_RESIZABLE;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0)
    {
        if (!Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024))
        {
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

            if (SDL_SetVideoMode(width, height, 0, mode))
            {
                double t0 = SDL_GetTicks() / 1000.0;

                SDL_WM_SetCaption(TITLE, TITLE);

                game_init();
                goto_state(&st_title);

                while (loop())
                {
                    double t1 = SDL_GetTicks() / 1000.0;

                    st_timer(t1 - t0);
                    st_paint();

                    SDL_GL_SwapBuffers();
                    SDL_Delay(1);
                        
                    t0 = t1;
                }
            }
            Mix_CloseAudio();
        }
        SDL_Quit();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

