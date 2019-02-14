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

/*---------------------------------------------------------------------------*/

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "version.h"
#include "glext.h"
#include "config.h"
#include "video.h"
#include "image.h"
#include "audio.h"
#include "demo.h"
#include "progress.h"
#include "gui.h"
#include "set.h"
#include "tilt.h"
#include "hmd.h"
#include "fs.h"
#include "common.h"
#include "text.h"
#include "mtrl.h"
#include "geom.h"
#include "joy.h"

#include "st_conf.h"
#include "st_title.h"
#include "st_demo.h"
#include "st_level.h"
#include "st_pause.h"

const char TITLE[] = "Neverball " VERSION;
const char ICON[] = "icon/neverball.png";

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
        wire = 0;
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        wire = 1;
    }
#endif
}

/*---------------------------------------------------------------------------*/

/*
 * Track held direction keys.
 */
static char key_pressed[4];

static const int key_other[4] = { 1, 0, 3, 2 };

static const int *key_axis[4] = {
    &CONFIG_JOYSTICK_AXIS_Y0,
    &CONFIG_JOYSTICK_AXIS_Y0,
    &CONFIG_JOYSTICK_AXIS_X0,
    &CONFIG_JOYSTICK_AXIS_X0
};

static const float key_tilt[4] = { -1.0f, +1.0f, -1.0f, +1.0f };

static int handle_key_dn(SDL_Event *e)
{
    int d = 1;
    int c = e->key.keysym.sym;

    int dir = -1;

    /* SDL made me do it. */
#ifdef __APPLE__
    if (c == SDLK_q && e->key.keysym.mod & KMOD_GUI)
        return 0;
#endif
#ifdef _WIN32
    if (c == SDLK_F4 && e->key.keysym.mod & KMOD_ALT)
        return 0;
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
        if (config_cheat())
            toggle_wire();
        break;
    case KEY_RESOURCES:
        if (config_cheat())
        {
            light_load();
            mtrl_reload();
        }
        break;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 1);
        break;
    case KEY_EXIT:
        d = st_keybd(KEY_EXIT, 1);
        break;

    default:
        if (config_tst_d(CONFIG_KEY_FORWARD,  c))
            dir = 0;
        else if (config_tst_d(CONFIG_KEY_BACKWARD, c))
            dir = 1;
        else if (config_tst_d(CONFIG_KEY_LEFT, c))
            dir = 2;
        else if (config_tst_d(CONFIG_KEY_RIGHT, c))
            dir = 3;

        if (dir != -1)
        {
            /* Ignore auto-repeat on direction keys. */

            if (e->key.repeat)
                break;

            key_pressed[dir] = 1;
            st_stick(config_get_d(*key_axis[dir]), key_tilt[dir]);
        }
        else
            d = st_keybd(e->key.keysym.sym, 1);
    }

    return d;
}

static int handle_key_up(SDL_Event *e)
{
    int d = 1;
    int c = e->key.keysym.sym;

    int dir = -1;

    switch (c)
    {
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 0);
        break;
    case KEY_EXIT:
        d = st_keybd(KEY_EXIT, 0);
        break;
    default:
        if (config_tst_d(CONFIG_KEY_FORWARD, c))
            dir = 0;
        else if (config_tst_d(CONFIG_KEY_BACKWARD, c))
            dir = 1;
        else if (config_tst_d(CONFIG_KEY_LEFT, c))
            dir = 2;
        else if (config_tst_d(CONFIG_KEY_RIGHT, c))
            dir = 3;

        if (dir != -1)
        {
            key_pressed[dir] = 0;

            if (key_pressed[key_other[dir]])
                st_stick(config_get_d(*key_axis[dir]), -key_tilt[dir]);
            else
                st_stick(config_get_d(*key_axis[dir]), 0.0f);
        }
        else
            d = st_keybd(e->key.keysym.sym, 0);
    }

    return d;
}

static int loop(void)
{
    SDL_Event e;
    int d = 1;

    int ax, ay, dx, dy;

    /* Process SDL events. */

    while (d && SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            return 0;

        case SDL_MOUSEMOTION:
            /* Convert to OpenGL coordinates. */

            ax = +e.motion.x;
            ay = -e.motion.y + video.window_h;
            dx = +e.motion.xrel;
            dy = (config_get_d(CONFIG_MOUSE_INVERT) ?
                  +e.motion.yrel : -e.motion.yrel);

            /* Convert to pixels. */

            ax = ROUND(ax * video.device_scale);
            ay = ROUND(ay * video.device_scale);
            dx = ROUND(dx * video.device_scale);
            dy = ROUND(dy * video.device_scale);

            st_point(ax, ay, dx, dy);

            break;

        case SDL_MOUSEBUTTONDOWN:
            d = st_click(e.button.button, 1);
            break;

        case SDL_MOUSEBUTTONUP:
            d = st_click(e.button.button, 0);
            break;

        case SDL_KEYDOWN:
            d = handle_key_dn(&e);
            break;

        case SDL_KEYUP:
            d = handle_key_up(&e);
            break;

        case SDL_WINDOWEVENT:
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_FOCUS_LOST:
                if (video_get_grab())
                    goto_state(&st_pause);
                break;

            case SDL_WINDOWEVENT_MOVED:
                if (config_get_d(CONFIG_DISPLAY) != video_display())
                    config_set_d(CONFIG_DISPLAY, video_display());
                break;

            case SDL_WINDOWEVENT_RESIZED:
                log_printf("Resize event (%u, %dx%d)\n",
                           e.window.windowID,
                           e.window.data1,
                           e.window.data2);
                break;

            case SDL_WINDOWEVENT_SIZE_CHANGED:
                log_printf("Size change event (%u, %dx%d)\n",
                           e.window.windowID,
                           e.window.data1,
                           e.window.data2);
                break;
            }
            break;

        case SDL_TEXTINPUT:
            text_input_str(e.text.text, 1);
            break;

        case SDL_JOYAXISMOTION:
            joy_axis(e.jaxis.which, e.jaxis.axis, JOY_VALUE(e.jaxis.value));
            break;

        case SDL_JOYBUTTONDOWN:
            d = joy_button(e.jbutton.which, e.jbutton.button, 1);
            break;

        case SDL_JOYBUTTONUP:
            d = joy_button(e.jbutton.which, e.jbutton.button, 0);
            break;

        case SDL_JOYDEVICEADDED:
            joy_add(e.jdevice.which);
            break;

        case SDL_JOYDEVICEREMOVED:
            joy_remove(e.jdevice.which);
            break;

        case SDL_MOUSEWHEEL:
            st_wheel(e.wheel.x, e.wheel.y);
            break;
        }
    }

    /* Process events via the tilt sensor API. */

    if (tilt_stat())
    {
        int b;
        int s;

        st_angle(tilt_get_x(),
                 tilt_get_z());

        while (tilt_get_button(&b, &s))
        {
            const int X = config_get_d(CONFIG_JOYSTICK_AXIS_X0);
            const int Y = config_get_d(CONFIG_JOYSTICK_AXIS_Y0);
            const int L = config_get_d(CONFIG_JOYSTICK_DPAD_L);
            const int R = config_get_d(CONFIG_JOYSTICK_DPAD_R);
            const int U = config_get_d(CONFIG_JOYSTICK_DPAD_U);
            const int D = config_get_d(CONFIG_JOYSTICK_DPAD_D);

            if (b == L || b == R || b == U || b == D)
            {
                static int pad[4] = { 0, 0, 0, 0 };

                /* Track the state of the D-pad buttons. */

                if      (b == L) pad[0] = s;
                else if (b == R) pad[1] = s;
                else if (b == U) pad[2] = s;
                else if (b == D) pad[3] = s;

                /* Convert D-pad button events into joystick axis motion. */

                if      (pad[0] && !pad[1]) st_stick(X, -1.0f);
                else if (pad[1] && !pad[0]) st_stick(X, +1.0f);
                else                        st_stick(X,  0.0f);

                if      (pad[2] && !pad[3]) st_stick(Y, -1.0f);
                else if (pad[3] && !pad[2]) st_stick(Y, +1.0f);
                else                        st_stick(Y,  0.0f);
            }
            else d = st_buttn(b, s);
        }
    }

    return d;
}

/*---------------------------------------------------------------------------*/

static char *opt_data;
static char *opt_replay;
static char *opt_level;

#define opt_usage                                                     \
    "Usage: %s [options ...]\n"                                       \
    "Options:\n"                                                      \
    "  -h, --help                show this usage message.\n"          \
    "  -v, --version             show version.\n"                     \
    "  -d, --data <dir>          use 'dir' as game data directory.\n" \
    "  -r, --replay <file>       play the replay 'file'.\n"           \
    "  -l, --level <file>        load the level 'file'\n"

#define opt_error(option) \
    fprintf(stderr, "Option '%s' requires an argument.\n", option)

static void opt_parse(int argc, char **argv)
{
    int i;

    /* Scan argument list. */

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help")    == 0)
        {
            printf(opt_usage, argv[0]);
            exit(EXIT_SUCCESS);
        }

        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            printf("%s\n", VERSION);
            exit(EXIT_SUCCESS);
        }

        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--data")    == 0)
        {
            if (i + 1 == argc)
            {
                opt_error(argv[i]);
                exit(EXIT_FAILURE);
            }
            opt_data = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--replay")  == 0)
        {
            if (i + 1 == argc)
            {
                opt_error(argv[i]);
                exit(EXIT_FAILURE);
            }
            opt_replay = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--level")  == 0)
        {
            if (i + 1 == argc)
            {
                opt_error(argv[i]);
                exit(EXIT_FAILURE);
            }
            opt_level = argv[++i];
            continue;
        }

        /* Perform magic on a single unrecognized argument. */

        if (argc == 2)
        {
            size_t len = strlen(argv[i]);
            int level = 0;

            if (len > 4)
            {
                char *ext = argv[i] + len - 4;

                if (strcmp(ext, ".map") == 0)
                    strncpy(ext, ".sol", 4);

                if (strcmp(ext, ".sol") == 0)
                    level = 1;
            }

            if (level)
                opt_level = argv[i];
            else
                opt_replay = argv[i];

            break;
        }
    }
}

#undef opt_usage
#undef opt_error

/*---------------------------------------------------------------------------*/

static int is_replay(struct dir_item *item)
{
    return str_ends_with(item->path, ".nbr");
}

static int is_score_file(struct dir_item *item)
{
    return str_starts_with(item->path, "neverballhs-");
}

static void make_dirs_and_migrate(void)
{
    Array items;
    int i;

    const char *src;
    char *dst;

    if (fs_mkdir("Replays"))
    {
        if ((items = fs_dir_scan("", is_replay)))
        {
            for (i = 0; i < array_len(items); i++)
            {
                src = DIR_ITEM_GET(items, i)->path;
                dst = concat_string("Replays/", src, NULL);
                fs_rename(src, dst);
                free(dst);
            }

            fs_dir_free(items);
        }
    }

    if (fs_mkdir("Scores"))
    {
        if ((items = fs_dir_scan("", is_score_file)))
        {
            for (i = 0; i < array_len(items); i++)
            {
                src = DIR_ITEM_GET(items, i)->path;
                dst = concat_string("Scores/",
                                    src + sizeof ("neverballhs-") - 1,
                                    ".txt",
                                    NULL);
                fs_rename(src, dst);
                free(dst);
            }

            fs_dir_free(items);
        }
    }

    fs_mkdir("Screenshots");
}

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int t1, t0;

    if (!fs_init(argv[0]))
    {
        fprintf(stderr, "Failure to initialize virtual file system (%s)\n",
                fs_error());
        return 1;
    }

    opt_parse(argc, argv);

    config_paths(opt_data);
    log_init("Neverball", "neverball.log");
    make_dirs_and_migrate();

    /* Initialize SDL. */

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) == -1)
    {
        log_printf("Failure to initialize SDL (%s)\n", SDL_GetError());
        return 1;
    }

    /* Enable joystick events. */

    joy_init();

    /* Intitialize configuration. */

    config_init();
    config_load();

    /* Initialize localization. */

    lang_init();

    /* Initialize audio. */

    audio_init();
    tilt_init();

    /* Initialize video. */

    if (!video_init())
        return 1;

    /* Material system. */

    mtrl_init();

    /* Screen states. */

    init_state(&st_null);

    /* Initialize demo playback or load the level. */

    if (opt_replay &&
        fs_add_path(dir_name(opt_replay)) &&
        progress_replay(base_name(opt_replay)))
    {
        demo_play_goto(1);
        goto_state(&st_demo_play);
    }
    else if (opt_level)
    {
        const char *path = fs_resolve(opt_level);
        int loaded = 0;

        if (path)
        {
            /* HACK: must be around for the duration of the game. */
            static struct level level;

            if (level_load(path, &level))
            {
                progress_init(MODE_STANDALONE);

                if (progress_play(&level))
                {
                    goto_state(&st_level);
                    loaded = 1;
                }
            }
        }
        else log_printf("File %s is not in game path\n", opt_level);

        if (!loaded)
            goto_state(&st_title);
    }
    else
        goto_state(&st_title);

    /* Run the main game loop. */

    t0 = SDL_GetTicks();

    while (loop())
    {
        if ((t1 = SDL_GetTicks()) > t0)
        {
            /* Step the game state. */

            st_timer(0.001f * (t1 - t0));

            t0 = t1;

            /* Render. */

            hmd_step();
            st_paint(0.001f * t0);
            video_swap();

            if (config_get_d(CONFIG_NICE))
                SDL_Delay(1);
        }
    }

    config_save();

    mtrl_quit();

    tilt_free();
    hmd_free();
    joy_quit();
    SDL_Quit();

    return 0;
}

/*---------------------------------------------------------------------------*/

