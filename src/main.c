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
#include "main.h"
#include "text.h"
#include "state.h"
#include "audio.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

#define TITLE "Neverball"
#define CONF_FILE "data/config.txt"

/* Set up some reasonable configuration defaults. */

int main_mode   = SDL_OPENGL;
int main_width  = 1024;
int main_height = 768;
int main_geom   = 1;
int main_rate   = 44100;
int main_buff   = AUD_BUFF_HI;
int main_nice   = 1;
int main_fps    = 0;

int joy_axis_x       = 0;
int joy_axis_y       = 1;
int joy_button_r     = 0;
int joy_button_l     = 1;
int joy_button_a     = 2;
int joy_button_b     = 3;
int joy_button_pause = 8;

/*---------------------------------------------------------------------------*/

static int shot(void)
{
    static char str[32];
    static int  num = 0;

    int i;
    int w = main_width;
    int h = main_height;

    SDL_Surface *buf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24,
                                            RMASK, GMASK, BMASK, 0);
    SDL_Surface *img = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24,
                                            RMASK, GMASK, BMASK, 0);

    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buf->pixels);

    for (i = 0; i < h; i++)
        memcpy((GLubyte *) img->pixels + 3 * w * i,
               (GLubyte *) buf->pixels + 3 * w * (h - i), 3 * w);

    sprintf(str, "screen%02d.bmp", num++);

    SDL_SaveBMP(img, str);
    SDL_FreeSurface(img);
    SDL_FreeSurface(buf);

    return 1;
}

/*---------------------------------------------------------------------------*/

void conf_store(void)
{
    FILE *fp = fopen(CONF_FILE, "w");

    if (fp)
    {
        fprintf(fp, "fullscreen %d\n", (main_mode & SDL_FULLSCREEN) ? 1 : 0);
        fprintf(fp, "width %d\n",      main_width);
        fprintf(fp, "height %d\n",     main_height);
        fprintf(fp, "textures %d\n",   get_image_scale());
        fprintf(fp, "geometry %d\n",   main_geom);
        fprintf(fp, "audio_rate %d\n", main_rate);
        fprintf(fp, "audio_buff %d\n", main_buff);
        fprintf(fp, "nice %d\n",       main_nice);
        fprintf(fp, "fps %d\n",        main_fps);

        fprintf(fp, "joy_axis_x %d\n",       joy_axis_x);
        fprintf(fp, "joy_axis_y %d\n",       joy_axis_y);
        fprintf(fp, "joy_button_r %d\n",     joy_button_r);
        fprintf(fp, "joy_button_l %d\n",     joy_button_l);
        fprintf(fp, "joy_button_a %d\n",     joy_button_a);
        fprintf(fp, "joy_button_b %d\n",     joy_button_b);
        fprintf(fp, "joy_button_pause %d\n", joy_button_pause);

        fclose(fp);
    }
}

void conf_load(void)
{
    FILE *fp = fopen(CONF_FILE, "r");

    char key[32];
    int  val;

    if (fp)
    {
        while (fscanf(fp, "%s %d", key, &val) == 2)
        {
            if (strcmp(key, "fullscreen") == 0)
                main_mode = SDL_OPENGL | (val ? SDL_FULLSCREEN : 0);

            if (strcmp(key, "width")      == 0) main_width = val;
            if (strcmp(key, "height")     == 0) main_height = val;
            if (strcmp(key, "textures")   == 0) set_image_scale(val);
            if (strcmp(key, "geometry")   == 0) main_geom = val;
            if (strcmp(key, "audio_rate") == 0) main_rate = val;
            if (strcmp(key, "audio_buff") == 0) main_buff = val;
            if (strcmp(key, "nice")       == 0) main_nice = val;
            if (strcmp(key, "fps")        == 0) main_fps = val;

            if (strcmp(key, "joy_axis_x")       == 0) joy_axis_x = val;
            if (strcmp(key, "joy_axis_y")       == 0) joy_axis_y = val;
            if (strcmp(key, "joy_button_r")     == 0) joy_button_r = val;
            if (strcmp(key, "joy_button_l")     == 0) joy_button_l = val;
            if (strcmp(key, "joy_button_a")     == 0) joy_button_a = val;
            if (strcmp(key, "joy_button_b")     == 0) joy_button_b = val;
            if (strcmp(key, "joy_button_pause") == 0) joy_button_pause = val;
        }
        fclose(fp);
    }
}

/*---------------------------------------------------------------------------*/

static void init(void)
{
    glClearColor(0.0f, 0.8f, 0.0f, 0.0f);

    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int set_mode(int w, int h, int m)
{
    if (SDL_SetVideoMode(w, h, 0, m))
    {
        text_free();

        main_width  = w;
        main_height = h;
        main_mode   = m;

        glViewport(0, 0, w, h);
        init();

        return text_init(h);;
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
            d = st_point(e.motion.x, main_height - e.motion.y,
                         e.motion.xrel, -e.motion.yrel);
              break;
        case SDL_MOUSEBUTTONDOWN:
            d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : +1, 1);
            break;
        case SDL_MOUSEBUTTONUP:
            d = st_click((e.button.button == SDL_BUTTON_LEFT) ? -1 : +1, 0);
            break;
        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_F10) { d = shot(); break;}
            if (e.key.keysym.sym == SDLK_F9) { main_fps  = 1-main_fps;  break;}
            if (e.key.keysym.sym == SDLK_F8) { main_nice = 1-main_nice; break;}
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
    SDL_Joystick *joy;

    conf_load();
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) == 0)
    {
        /* Initialize the first connected joystick. */

        if (SDL_NumJoysticks() > 0)
        {
            if ((joy = SDL_JoystickOpen(0)))
                SDL_JoystickEventState(SDL_ENABLE);
        }

        /* Initialize audio and text rendering. */

        if (audio_init(main_rate, main_buff) && text_init(main_height))
        {
            /* Require 16-bit double color buffer with 16-bit depth buffer. */

            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

            /* Initialize the video. */

            if (SDL_SetVideoMode(main_width, main_height, 0, main_mode))
            {
                int t1, t0 = SDL_GetTicks();

                SDL_WM_SetCaption(TITLE, TITLE);

                /* Run the main game loop. */

                init();
                goto_state(&st_title);

                while (loop())
                    if ((t1 = SDL_GetTicks()) > t0)
                    {
                        st_timer((t1 - t0) / 1000.0);
                        st_paint();

                        SDL_GL_SwapBuffers();
                        if (main_nice) SDL_Delay(1);
                        
                        t0 = t1;
                    }
            }
            else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());
                
            text_free();
            audio_free();
        }

        if (SDL_JoystickOpened(0))
            SDL_JoystickClose(joy);

        SDL_Quit();
    }
    else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());

    conf_store();

    return 0;
}

/*---------------------------------------------------------------------------*/

