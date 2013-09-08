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

#include <SDL.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "version.h"
#include "glext.h"
#include "audio.h"
#include "image.h"
#include "state.h"
#include "config.h"
#include "video.h"
#include "course.h"
#include "hole.h"
#include "game.h"
#include "gui.h"
#include "hmd.h"
#include "fs.h"

#include "st_conf.h"
#include "st_all.h"

const char TITLE[] = "Neverputt " VERSION;
const char ICON[] = "icon/neverputt.png";

/*---------------------------------------------------------------------------*/

static void shot(void)
{
    static char filename[MAXSTR];
    sprintf(filename, "Screenshots/screen%05d.png", config_screenshot());
    video_snap(filename);
}

/*---------------------------------------------------------------------------*/

static void toggle_wire(void)
{
#if !ENABLE_OPENGLES
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
#endif
}

/*---------------------------------------------------------------------------*/

static int loop(void)
{
    SDL_Event e;
    int d = 1;
    int c;

    while (d && SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            return 0;

        switch (e.type)
        {
        case SDL_MOUSEMOTION:
            st_point(+e.motion.x,
                     -e.motion.y + config_get_d(CONFIG_HEIGHT),
                     +e.motion.xrel,
                     -e.motion.yrel);
            break;

        case SDL_MOUSEBUTTONDOWN:
            d = st_click(e.button.button, 1);
            break;

        case SDL_MOUSEBUTTONUP:
            d = st_click(e.button.button, 0);
            break;

        case SDL_KEYDOWN:

            c = e.key.keysym.sym;

#ifdef __APPLE__
            if (c == SDLK_q && e.key.keysym.mod & KMOD_META)
            {
                d = 0;
                break;
            }
#endif
#ifdef WIN32
            if (c == SDLK_F4 && e.key.keysym.mod & KMOD_ALT)
            {
                d = 0;
                break;
            }
#endif

            switch (c)
            {
            case KEY_SCREENSHOT:
                shot();
                break;
            case KEY_FPS:
                config_tgl_d(CONFIG_FPS);
                break;
            case KEY_WIREFRAME:
                toggle_wire();
                break;
            case SDLK_RETURN:
                d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 1);
                break;
            case SDLK_ESCAPE:
                d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_EXIT), 1);
                break;

            default:
                if (config_tst_d(CONFIG_KEY_FORWARD, c))
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), -1.0f);
                else if (config_tst_d(CONFIG_KEY_BACKWARD, c))
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), +1.0f);
                else if (config_tst_d(CONFIG_KEY_LEFT, c))
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), -1.0f);
                else if (config_tst_d(CONFIG_KEY_RIGHT, c))
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), +1.0f);

                d = st_keybd(e.key.keysym.sym, 1);
            }
            break;

        case SDL_KEYUP:

            c = e.key.keysym.sym;

            switch (c)
            {
            case SDLK_RETURN:
                d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 0);
                break;
            case SDLK_ESCAPE:
                d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_EXIT), 0);
                break;

            default:
                if (config_tst_d(CONFIG_KEY_FORWARD, c))
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), 0.0f);
                else if (config_tst_d(CONFIG_KEY_BACKWARD, c))
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_Y), 0.0f);
                else if (config_tst_d(CONFIG_KEY_LEFT, c))
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), 0.0f);
                else if (config_tst_d(CONFIG_KEY_RIGHT, c))
                    st_stick(config_get_d(CONFIG_JOYSTICK_AXIS_X), 0.0f);

                d = st_keybd(e.key.keysym.sym, 0);
            }
            break;

        case SDL_ACTIVEEVENT:
            if (e.active.state == SDL_APPINPUTFOCUS)
                if (e.active.gain == 0 && video_get_grab())
                    goto_pause(&st_over, 0);
            break;

        case SDL_JOYAXISMOTION:
            st_stick(e.jaxis.axis, JOY_VALUE(e.jaxis.value));
            break;

        case SDL_JOYBUTTONDOWN:
            d = st_buttn(e.jbutton.button, 1);
            break;

        case SDL_JOYBUTTONUP:
            d = st_buttn(e.jbutton.button, 0);
            break;
        }
    }
    return d;
}

/*---------------------------------------------------------------------------*/

static char *opt_data;
static char *opt_hole;

static void opt_parse(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--data") == 0)
        {
            if (++i < argc)
                opt_data = argv[i];
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--hole") == 0)
        {
            if (++i < argc)
                opt_hole = argv[i];
        }
    }

    if (argc == 2)
    {
        size_t len = strlen(argv[1]);

        if (len > 4)
        {
            char *ext = argv[1] + len - 4;

            if (strcmp(ext, ".map") == 0)
                strcpy(ext, ".sol");

            if (strcmp(ext, ".sol") == 0)
                opt_hole = argv[1];
        }
    }
}

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int camera = 0;
    SDL_Joystick *joy = NULL;

    if (!fs_init(argv[0]))
    {
        fprintf(stderr, "Failure to initialize virtual file system: %s\n",
                fs_error());
        return 1;
    }

    srand((int) time(NULL));

    lang_init("neverball");

    opt_parse(argc, argv);

    config_paths(opt_data);
    fs_mkdir("Screenshots");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) == 0)
    {
        config_init();
        config_load();

        /* Cache Neverball's camera setting. */

        camera = config_get_d(CONFIG_CAMERA);

        /* Initialize the joystick. */

        if (config_get_d(CONFIG_JOYSTICK) && SDL_NumJoysticks() > 0)
        {
            joy = SDL_JoystickOpen(config_get_d(CONFIG_JOYSTICK_DEVICE));
            if (joy)
            {
                SDL_JoystickEventState(SDL_ENABLE);
                set_joystick(joy);
            }
        }

        /* Initialize the audio. */

        audio_init();

        /* Initialize the video. */

        if (video_init(TITLE, ICON))
        {
            int t1, t0 = SDL_GetTicks();

            /* Run the main game loop. */

            init_state(&st_null);

            if (opt_hole)
            {
                const char *path = fs_resolve(opt_hole);
                int loaded = 0;

                if (path)
                {
                    hole_init(NULL);

                    if (hole_load(0, path) &&
                        hole_load(1, path) &&
                        hole_goto(1, 1))
                    {
                        goto_state(&st_next);
                        loaded = 1;
                    }
                }

                if (!loaded)
                    goto_state(&st_title);
            }
            else
                goto_state(&st_title);

            while (loop())
                if ((t1 = SDL_GetTicks()) > t0)
                {
                    st_timer((t1 - t0) / 1000.f);
                    hmd_step();
                    st_paint(0.001f * t1);
                    video_swap();

                    t0 = t1;

                    if (config_get_d(CONFIG_NICE))
                        SDL_Delay(1);
                }
        }

        /* Restore Neverball's camera setting. */

        config_set_d(CONFIG_CAMERA, camera);
        config_save();

        SDL_Quit();
    }
    else fprintf(stderr, "%s: %s\n", argv[0], SDL_GetError());

    return 0;
}

/*---------------------------------------------------------------------------*/

