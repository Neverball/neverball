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
#include <string.h>
#include <stdio.h>

#include "gl.h"
#include "state.h"
#include "audio.h"

/*---------------------------------------------------------------------------*/

#define TITLE "SUPER EMPTY BALL"

int main_mode   = SDL_OPENGL;
int main_width  = 1024;
int main_height = 768;
int main_geom   = 1;
int main_rate   = 44100;
int main_buff   = 2048;

/*---------------------------------------------------------------------------*/

static int shot(void)
{
    static char str[32];
    static int  num = 0;

    int i;
    int w = main_width;
    int h = main_height;

    SDL_Surface *buf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24,
                                            0x0000FF, 0x00FF00, 0xFF0000, 0);
    SDL_Surface *img = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24,
                                            0x0000FF, 0x00FF00, 0xFF0000, 0);

    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buf->pixels);

    for (i = 0; i < h; i++)
        memcpy((GLubyte *) img->pixels + 3 * w * i,
               (GLubyte *) buf->pixels + 3 * w * (h - i), 3 * w);

    sprintf(str, "seb%02d.bmp", num++);

    SDL_SaveBMP(img, str);
    SDL_FreeSurface(img);
    SDL_FreeSurface(buf);

    return 1;
}

/*---------------------------------------------------------------------------*/

static void init(void)
{
    glClearColor(0.0f, 0.8f, 0.0f, 0.0f);

    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
}

int set_mode(int w, int h, int m)
{
    if (SDL_SetVideoMode(w, h, 0, m))
    {
        main_width  = w;
        main_height = h;
        main_mode   = m;

        glViewport(0, 0, w, h);
        init();

        return 1;
    }
    return 0;
}

static int loop(void)
{
    SDL_Event e;
    int d = 1;

    while (d && SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_MOUSEMOTION:
            d = st_point(e.motion.x, main_height - e.motion.y);
            break;
        case SDL_MOUSEBUTTONDOWN:
            d = st_click(1);
            break;
        case SDL_MOUSEBUTTONUP:
            d = st_click(0);
            break;
        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_F10)
                d = shot();
            else
                d = st_keybd(e.key.keysym.sym);
            break;
        case SDL_QUIT:
            d = 0;
        }

    return d;
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0)
    {
        if (audio_init(main_rate, main_buff))
        {
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

            if (SDL_SetVideoMode(main_width, main_height, 0, main_mode))
            {
                int t1, t0 = SDL_GetTicks();

                SDL_WM_SetCaption(TITLE, TITLE);

                init();
                goto_state(&st_title);

                while (loop())
                    if ((t1 = SDL_GetTicks()) > t0)
                    {
                        st_timer((t1 - t0) / 1000.0);
                        st_paint();

                        SDL_GL_SwapBuffers();
                        SDL_Delay(1);
                        
                        t0 = t1;
                    }
                    else printf("skip\n");
            }
            audio_free();
        }
        SDL_Quit();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

