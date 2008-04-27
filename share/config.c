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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "config.h"
#include "glext.h"
#include "vec3.h"
#include "sync.h"

/*---------------------------------------------------------------------------*/

/* Define the mkdir symbol. */

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

/*---------------------------------------------------------------------------*/

static int   option_d[CONFIG_OPTION_D_COUNT];
static char *option_s[CONFIG_OPTION_S_COUNT];

static int dirty = 0;

/*---------------------------------------------------------------------------*/

static void config_key(const char *s, int i, int d)
{
    int c;

    config_set_d(i, d);

    for (c = 0; c < SDLK_LAST; c++)
        if (strcmp(s, SDL_GetKeyName((SDLKey) c)) == 0)
        {
            config_set_d(i, c);
            break;
        }
}

/*---------------------------------------------------------------------------*/

void config_init(void)
{
    memset(option_d, 0, sizeof (option_d));
    memset(option_s, 0, sizeof (option_s));

    config_set_d(CONFIG_FULLSCREEN,           DEFAULT_FULLSCREEN);
    config_set_d(CONFIG_WIDTH,                DEFAULT_WIDTH);
    config_set_d(CONFIG_HEIGHT,               DEFAULT_HEIGHT);
    config_set_d(CONFIG_STEREO,               DEFAULT_STEREO);
    config_set_d(CONFIG_CAMERA,               DEFAULT_CAMERA);
    config_set_d(CONFIG_TEXTURES,             DEFAULT_TEXTURES);
    config_set_d(CONFIG_GEOMETRY,             DEFAULT_GEOMETRY);
    config_set_d(CONFIG_REFLECTION,           DEFAULT_REFLECTION);
    config_set_d(CONFIG_MULTISAMPLE,          DEFAULT_MULTISAMPLE);
    config_set_d(CONFIG_MIPMAP,               DEFAULT_MIPMAP);
    config_set_d(CONFIG_ANISO,                DEFAULT_ANISO);
    config_set_d(CONFIG_BACKGROUND,           DEFAULT_BACKGROUND);
    config_set_d(CONFIG_SHADOW,               DEFAULT_SHADOW);
    config_set_d(CONFIG_AUDIO_BUFF,           DEFAULT_AUDIO_BUFF);
    config_set_d(CONFIG_MOUSE_SENSE,          DEFAULT_MOUSE_SENSE);
    config_set_d(CONFIG_MOUSE_INVERT,         DEFAULT_MOUSE_INVERT);
    config_set_d(CONFIG_VSYNC,                DEFAULT_VSYNC);
    config_set_d(CONFIG_NICE,                 DEFAULT_NICE);
    config_set_d(CONFIG_FPS,                  DEFAULT_FPS);
    config_set_d(CONFIG_SOUND_VOLUME,         DEFAULT_SOUND_VOLUME);
    config_set_d(CONFIG_MUSIC_VOLUME,         DEFAULT_MUSIC_VOLUME);
    config_set_d(CONFIG_JOYSTICK,             DEFAULT_JOYSTICK);
    config_set_d(CONFIG_JOYSTICK_DEVICE,      DEFAULT_JOYSTICK_DEVICE);
    config_set_d(CONFIG_JOYSTICK_AXIS_X,      DEFAULT_JOYSTICK_AXIS_X);
    config_set_d(CONFIG_JOYSTICK_AXIS_Y,      DEFAULT_JOYSTICK_AXIS_Y);
    config_set_d(CONFIG_JOYSTICK_BUTTON_A,    DEFAULT_JOYSTICK_BUTTON_A);
    config_set_d(CONFIG_JOYSTICK_BUTTON_B,    DEFAULT_JOYSTICK_BUTTON_B);
    config_set_d(CONFIG_JOYSTICK_BUTTON_L,    DEFAULT_JOYSTICK_BUTTON_L);
    config_set_d(CONFIG_JOYSTICK_BUTTON_R,    DEFAULT_JOYSTICK_BUTTON_R);
    config_set_d(CONFIG_JOYSTICK_BUTTON_EXIT, DEFAULT_JOYSTICK_BUTTON_EXIT);
    config_set_d(CONFIG_JOYSTICK_CAMERA_1,    DEFAULT_JOYSTICK_CAMERA_1);
    config_set_d(CONFIG_JOYSTICK_CAMERA_2,    DEFAULT_JOYSTICK_CAMERA_2);
    config_set_d(CONFIG_JOYSTICK_CAMERA_3,    DEFAULT_JOYSTICK_CAMERA_3);
    config_set_d(CONFIG_JOYSTICK_DPAD_L,      DEFAULT_JOYSTICK_DPAD_L);
    config_set_d(CONFIG_JOYSTICK_DPAD_R,      DEFAULT_JOYSTICK_DPAD_R);
    config_set_d(CONFIG_JOYSTICK_DPAD_U,      DEFAULT_JOYSTICK_DPAD_U);
    config_set_d(CONFIG_JOYSTICK_DPAD_D,      DEFAULT_JOYSTICK_DPAD_D);
    config_set_d(CONFIG_KEY_CAMERA_1,         DEFAULT_KEY_CAMERA_1);
    config_set_d(CONFIG_KEY_CAMERA_2,         DEFAULT_KEY_CAMERA_2);
    config_set_d(CONFIG_KEY_CAMERA_3,         DEFAULT_KEY_CAMERA_3);
    config_set_d(CONFIG_KEY_CAMERA_R,         DEFAULT_KEY_CAMERA_R);
    config_set_d(CONFIG_KEY_CAMERA_L,         DEFAULT_KEY_CAMERA_L);
    config_set_d(CONFIG_VIEW_FOV,             DEFAULT_VIEW_FOV);
    config_set_d(CONFIG_VIEW_DP,              DEFAULT_VIEW_DP);
    config_set_d(CONFIG_VIEW_DC,              DEFAULT_VIEW_DC);
    config_set_d(CONFIG_VIEW_DZ,              DEFAULT_VIEW_DZ);
    config_set_d(CONFIG_ROTATE_FAST,          DEFAULT_ROTATE_FAST);
    config_set_d(CONFIG_ROTATE_SLOW,          DEFAULT_ROTATE_SLOW);
    config_set_s(CONFIG_PLAYER,               DEFAULT_PLAYER);
    config_set_s(CONFIG_BALL,                 DEFAULT_BALL);
    config_set_s(CONFIG_WIIMOTE_ADDR,         DEFAULT_WIIMOTE_ADDR);
    config_set_d(CONFIG_CHEAT,                DEFAULT_CHEAT);
    config_set_d(CONFIG_STATS,                DEFAULT_STATS);
    config_set_d(CONFIG_UNIFORM,              DEFAULT_UNIFORM);
    config_set_d(CONFIG_KEY_FORWARD,          DEFAULT_KEY_FORWARD);
    config_set_d(CONFIG_KEY_BACKWARD,         DEFAULT_KEY_BACKWARD);
    config_set_d(CONFIG_KEY_LEFT,             DEFAULT_KEY_LEFT);
    config_set_d(CONFIG_KEY_RIGHT,            DEFAULT_KEY_RIGHT);
    config_set_d(CONFIG_KEY_PAUSE,            DEFAULT_KEY_PAUSE);
    config_set_d(CONFIG_KEY_RESTART,          DEFAULT_KEY_RESTART);
    config_set_d(CONFIG_KEY_SCORE_NEXT,       DEFAULT_KEY_SCORE_NEXT);
    config_set_d(CONFIG_SCREENSHOT,           DEFAULT_SCREENSHOT);
    config_set_d(CONFIG_LOCK_GOALS,           DEFAULT_LOCK_GOALS);
}

void config_load(void)
{
    FILE *fp;

    if ((fp = fopen(config_user(USER_CONFIG_FILE), "r")))
    {
        char buf[MAXSTR];
        char key[MAXSTR];
        char val[MAXSTR];

        while (fgets(buf, MAXSTR, fp))
            if (sscanf(buf, "%s %s", key, val) == 2)
            {
                if      (strcmp(key, "fullscreen")            == 0)
                    config_set_d(CONFIG_FULLSCREEN,           atoi(val));
                else if (strcmp(key, "width")                 == 0)
                    config_set_d(CONFIG_WIDTH,                atoi(val));
                else if (strcmp(key, "height")                == 0)
                    config_set_d(CONFIG_HEIGHT,               atoi(val));
                else if (strcmp(key, "stereo")                == 0)
                    config_set_d(CONFIG_STEREO,               atoi(val));
                else if (strcmp(key, "camera")                == 0)
                    config_set_d(CONFIG_CAMERA,               atoi(val));
                else if (strcmp(key, "textures")              == 0)
                    config_set_d(CONFIG_TEXTURES,             atoi(val));
                else if (strcmp(key, "geometry")              == 0)
                    config_set_d(CONFIG_GEOMETRY,             atoi(val));
                else if (strcmp(key, "reflection")            == 0)
                    config_set_d(CONFIG_REFLECTION,           atoi(val));
                else if (strcmp(key, "multisample")           == 0)
                    config_set_d(CONFIG_MULTISAMPLE,          atoi(val));
                else if (strcmp(key, "mipmap")                == 0)
                    config_set_d(CONFIG_MIPMAP,               atoi(val));
                else if (strcmp(key, "aniso")                 == 0)
                    config_set_d(CONFIG_ANISO,                atoi(val));
                else if (strcmp(key, "background")            == 0)
                    config_set_d(CONFIG_BACKGROUND,           atoi(val));
                else if (strcmp(key, "shadow")                == 0)
                    config_set_d(CONFIG_SHADOW,               atoi(val));
                else if (strcmp(key, "audio_buff")            == 0)
                    config_set_d(CONFIG_AUDIO_BUFF,           atoi(val));
                else if (strcmp(key, "mouse_sense")           == 0)
                    config_set_d(CONFIG_MOUSE_SENSE,          atoi(val));
                else if (strcmp(key, "mouse_invert")          == 0)
                    config_set_d(CONFIG_MOUSE_INVERT,         atoi(val));
                else if (strcmp(key, "vsync")                 == 0)
                    config_set_d(CONFIG_VSYNC,                atoi(val));
                else if (strcmp(key, "nice")                  == 0)
                    config_set_d(CONFIG_NICE,                 atoi(val));
                else if (strcmp(key, "fps")                   == 0)
                    config_set_d(CONFIG_FPS,                  atoi(val));
                else if (strcmp(key, "sound_volume")          == 0)
                    config_set_d(CONFIG_SOUND_VOLUME,         atoi(val));
                else if (strcmp(key, "music_volume")          == 0)
                    config_set_d(CONFIG_MUSIC_VOLUME,         atoi(val));
                else if (strcmp(key, "joystick")              == 0)
                    config_set_d(CONFIG_JOYSTICK,             atoi(val));
                else if (strcmp(key, "joystick_device")       == 0)
                    config_set_d(CONFIG_JOYSTICK_DEVICE,      atoi(val));
                else if (strcmp(key, "joystick_axis_x")       == 0)
                    config_set_d(CONFIG_JOYSTICK_AXIS_X,      atoi(val));
                else if (strcmp(key, "joystick_axis_y")       == 0)
                    config_set_d(CONFIG_JOYSTICK_AXIS_Y,      atoi(val));
                else if (strcmp(key, "joystick_button_a")     == 0)
                    config_set_d(CONFIG_JOYSTICK_BUTTON_A,    atoi(val));
                else if (strcmp(key, "joystick_button_b")     == 0)
                    config_set_d(CONFIG_JOYSTICK_BUTTON_B,    atoi(val));
                else if (strcmp(key, "joystick_button_r")     == 0)
                    config_set_d(CONFIG_JOYSTICK_BUTTON_R,    atoi(val));
                else if (strcmp(key, "joystick_button_l")     == 0)
                    config_set_d(CONFIG_JOYSTICK_BUTTON_L,    atoi(val));
                else if (strcmp(key, "joystick_button_exit")  == 0)
                    config_set_d(CONFIG_JOYSTICK_BUTTON_EXIT, atoi(val));
                else if (strcmp(key, "joystick_camera_1")     == 0)
                    config_set_d(CONFIG_JOYSTICK_CAMERA_1,    atoi(val));
                else if (strcmp(key, "joystick_camera_2")     == 0)
                    config_set_d(CONFIG_JOYSTICK_CAMERA_2,    atoi(val));
                else if (strcmp(key, "joystick_camera_3")     == 0)
                    config_set_d(CONFIG_JOYSTICK_CAMERA_3,    atoi(val));
                else if (strcmp(key, "view_fov")              == 0)
                    config_set_d(CONFIG_VIEW_FOV,             atoi(val));
                else if (strcmp(key, "view_dp")               == 0)
                    config_set_d(CONFIG_VIEW_DP,              atoi(val));
                else if (strcmp(key, "view_dc")               == 0)
                    config_set_d(CONFIG_VIEW_DC,              atoi(val));
                else if (strcmp(key, "view_dz")               == 0)
                    config_set_d(CONFIG_VIEW_DZ,              atoi(val));
                else if (strcmp(key, "rotate_fast")           == 0)
                    config_set_d(CONFIG_ROTATE_FAST,          atoi(val));
                else if (strcmp(key, "rotate_slow")           == 0)
                    config_set_d(CONFIG_ROTATE_SLOW,          atoi(val));

                else if (strcmp(key, "key_forward")  == 0)
                    config_key(val, CONFIG_KEY_FORWARD, DEFAULT_KEY_FORWARD);
                else if (strcmp(key, "key_backward")  == 0)
                    config_key(val, CONFIG_KEY_BACKWARD, DEFAULT_KEY_BACKWARD);
                else if (strcmp(key, "key_left")  == 0)
                    config_key(val, CONFIG_KEY_LEFT, DEFAULT_KEY_LEFT);
                else if (strcmp(key, "key_right")  == 0)
                    config_key(val, CONFIG_KEY_RIGHT, DEFAULT_KEY_RIGHT);

                else if (strcmp(key, "key_camera_1")  == 0)
                    config_key(val, CONFIG_KEY_CAMERA_1, DEFAULT_KEY_CAMERA_1);
                else if (strcmp(key, "key_camera_2")  == 0)
                    config_key(val, CONFIG_KEY_CAMERA_2, DEFAULT_KEY_CAMERA_2);
                else if (strcmp(key, "key_camera_3")  == 0)
                    config_key(val, CONFIG_KEY_CAMERA_3, DEFAULT_KEY_CAMERA_3);
                else if (strcmp(key, "key_camera_r")  == 0)
                    config_key(val, CONFIG_KEY_CAMERA_R, DEFAULT_KEY_CAMERA_R);
                else if (strcmp(key, "key_camera_l")  == 0)
                    config_key(val, CONFIG_KEY_CAMERA_L, DEFAULT_KEY_CAMERA_L);

                else if (strcmp(key, "key_pause")    == 0)
                    config_key(val, CONFIG_KEY_PAUSE,   DEFAULT_KEY_PAUSE);
                else if (strcmp(key, "key_restart")  == 0)
                    config_key(val, CONFIG_KEY_RESTART, DEFAULT_KEY_RESTART);

                else if (strcmp(key, "key_score_next") == 0)
                    config_key(val, CONFIG_KEY_SCORE_NEXT, DEFAULT_KEY_SCORE_NEXT);

                else if (strcmp(key, "player") == 0)
                    config_set_s(CONFIG_PLAYER, val);
                else if (strcmp(key, "ball_file") == 0)
                    config_set_s(CONFIG_BALL, val);
                else if (strcmp(key, "wiimote_addr") == 0)
                    config_set_s(CONFIG_WIIMOTE_ADDR, val);

                else if (strcmp(key, "cheat")   == 0)
                    config_set_d(CONFIG_CHEAT, atoi(val));
                else if (strcmp(key, "stats")   == 0)
                    config_set_d(CONFIG_STATS, atoi(val));
                else if (strcmp(key, "uniform") == 0)
                    config_set_d(CONFIG_UNIFORM, atoi(val));
                else if (strcmp(key, "screenshot") == 0)
                    config_set_d(CONFIG_SCREENSHOT, atoi(val));
                else if (strcmp(key, "lock_goals") == 0)
                    config_set_d(CONFIG_LOCK_GOALS, atoi(val));
            }

        fclose(fp);

        dirty = 0;
    }
}

void config_save(void)
{
    FILE *fp;

    if (dirty && (fp = fopen(config_user(USER_CONFIG_FILE), "w")))
    {
        fprintf(fp, "fullscreen           %d\n",
                option_d[CONFIG_FULLSCREEN]);
        fprintf(fp, "width                %d\n",
                option_d[CONFIG_WIDTH]);
        fprintf(fp, "height               %d\n",
                option_d[CONFIG_HEIGHT]);
        fprintf(fp, "stereo               %d\n",
                option_d[CONFIG_STEREO]);
        fprintf(fp, "camera               %d\n",
                option_d[CONFIG_CAMERA]);
        fprintf(fp, "textures             %d\n",
                option_d[CONFIG_TEXTURES]);
        fprintf(fp, "geometry             %d\n",
                option_d[CONFIG_GEOMETRY]);
        fprintf(fp, "reflection           %d\n",
                option_d[CONFIG_REFLECTION]);
        fprintf(fp, "multisample          %d\n",
                option_d[CONFIG_MULTISAMPLE]);
        fprintf(fp, "mipmap               %d\n",
                option_d[CONFIG_MIPMAP]);
        fprintf(fp, "aniso                %d\n",
                option_d[CONFIG_ANISO]);
        fprintf(fp, "background           %d\n",
                option_d[CONFIG_BACKGROUND]);
        fprintf(fp, "shadow               %d\n",
                option_d[CONFIG_SHADOW]);
        fprintf(fp, "audio_buff           %d\n",
                option_d[CONFIG_AUDIO_BUFF]);
        fprintf(fp, "mouse_sense          %d\n",
                option_d[CONFIG_MOUSE_SENSE]);
        fprintf(fp, "mouse_invert         %d\n",
                option_d[CONFIG_MOUSE_INVERT]);
        fprintf(fp, "vsync                %d\n",
                option_d[CONFIG_VSYNC]);
        fprintf(fp, "nice                 %d\n",
                option_d[CONFIG_NICE]);
        fprintf(fp, "fps                  %d\n",
                option_d[CONFIG_FPS]);
        fprintf(fp, "sound_volume         %d\n",
                option_d[CONFIG_SOUND_VOLUME]);
        fprintf(fp, "music_volume         %d\n",
                option_d[CONFIG_MUSIC_VOLUME]);
        fprintf(fp, "joystick             %d\n",
                option_d[CONFIG_JOYSTICK]);
        fprintf(fp, "joystick_device      %d\n",
                option_d[CONFIG_JOYSTICK_DEVICE]);
        fprintf(fp, "joystick_axis_x      %d\n",
                option_d[CONFIG_JOYSTICK_AXIS_X]);
        fprintf(fp, "joystick_axis_y      %d\n",
                option_d[CONFIG_JOYSTICK_AXIS_Y]);
        fprintf(fp, "joystick_button_a    %d\n",
                option_d[CONFIG_JOYSTICK_BUTTON_A]);
        fprintf(fp, "joystick_button_b    %d\n",
                option_d[CONFIG_JOYSTICK_BUTTON_B]);
        fprintf(fp, "joystick_button_r    %d\n",
                option_d[CONFIG_JOYSTICK_BUTTON_R]);
        fprintf(fp, "joystick_button_l    %d\n",
                option_d[CONFIG_JOYSTICK_BUTTON_L]);
        fprintf(fp, "joystick_button_exit %d\n",
                option_d[CONFIG_JOYSTICK_BUTTON_EXIT]);
        fprintf(fp, "joystick_camera_1    %d\n",
                option_d[CONFIG_JOYSTICK_CAMERA_1]);
        fprintf(fp, "joystick_camera_2    %d\n",
                option_d[CONFIG_JOYSTICK_CAMERA_2]);
        fprintf(fp, "joystick_camera_3    %d\n",
                option_d[CONFIG_JOYSTICK_CAMERA_3]);
        fprintf(fp, "view_fov             %d\n",
                option_d[CONFIG_VIEW_FOV]);
        fprintf(fp, "view_dp              %d\n",
                option_d[CONFIG_VIEW_DP]);
        fprintf(fp, "view_dc              %d\n",
                option_d[CONFIG_VIEW_DC]);
        fprintf(fp, "view_dz              %d\n",
                option_d[CONFIG_VIEW_DZ]);
        fprintf(fp, "rotate_fast          %d\n",
                option_d[CONFIG_ROTATE_FAST]);
        fprintf(fp, "rotate_slow          %d\n",
                option_d[CONFIG_ROTATE_SLOW]);

        fprintf(fp, "key_forward          %s\n",
                SDL_GetKeyName((SDLKey) option_d[CONFIG_KEY_FORWARD]));
        fprintf(fp, "key_backward         %s\n",
                SDL_GetKeyName((SDLKey) option_d[CONFIG_KEY_BACKWARD]));
        fprintf(fp, "key_left             %s\n",
                SDL_GetKeyName((SDLKey) option_d[CONFIG_KEY_LEFT]));
        fprintf(fp, "key_right            %s\n",
                SDL_GetKeyName((SDLKey) option_d[CONFIG_KEY_RIGHT]));

        fprintf(fp, "key_camera_1         %s\n",
                SDL_GetKeyName((SDLKey) option_d[CONFIG_KEY_CAMERA_1]));
        fprintf(fp, "key_camera_2         %s\n",
                SDL_GetKeyName((SDLKey) option_d[CONFIG_KEY_CAMERA_2]));
        fprintf(fp, "key_camera_3         %s\n",
                SDL_GetKeyName((SDLKey) option_d[CONFIG_KEY_CAMERA_3]));
        fprintf(fp, "key_camera_r         %s\n",
                SDL_GetKeyName((SDLKey) option_d[CONFIG_KEY_CAMERA_R]));
        fprintf(fp, "key_camera_l         %s\n",
                SDL_GetKeyName((SDLKey) option_d[CONFIG_KEY_CAMERA_L]));

        fprintf(fp, "key_pause            %s\n",
                SDL_GetKeyName((SDLKey) option_d[CONFIG_KEY_PAUSE]));
        fprintf(fp, "key_restart          %s\n",
                SDL_GetKeyName((SDLKey) option_d[CONFIG_KEY_RESTART]));

        fprintf(fp, "key_score_next       %s\n",
                SDL_GetKeyName((SDLKey) option_d[CONFIG_KEY_SCORE_NEXT]));

        if (strlen(option_s[CONFIG_PLAYER]) > 0)
            fprintf(fp, "player       %s\n", option_s[CONFIG_PLAYER]);
        if (strlen(option_s[CONFIG_BALL]) > 0)
            fprintf(fp, "ball_file    %s\n", option_s[CONFIG_BALL]);
        if (strlen(option_s[CONFIG_WIIMOTE_ADDR]) > 0)
            fprintf(fp, "wiimote_addr %s\n", option_s[CONFIG_WIIMOTE_ADDR]);

        fprintf(fp, "stats                %d\n", option_d[CONFIG_STATS]);
        fprintf(fp, "uniform              %d\n", option_d[CONFIG_UNIFORM]);
        fprintf(fp, "screenshot           %d\n", option_d[CONFIG_SCREENSHOT]);
        fprintf(fp, "lock_goals           %d\n", option_d[CONFIG_LOCK_GOALS]);

        if (config_cheat())
            fprintf(fp, "cheat                %d\n", option_d[CONFIG_CHEAT]);

        fclose(fp);
    }

    dirty = 0;
}

/*---------------------------------------------------------------------------*/

int check_extension(const char *needle)
{
    const GLubyte *haystack, *c;

    /* Search for the given string in the OpenGL extension strings. */

    for (haystack = glGetString(GL_EXTENSIONS); *haystack; haystack++)
    {
        for (c = (const GLubyte *) needle; *c && *haystack; c++, haystack++)
            if (*c != *haystack)
                break;

        if ((*c == 0) && (*haystack == ' ' || *haystack == '\0'))
            return 1;
    }

    return 0;
}

int config_mode(int f, int w, int h)
{
    int stereo  = config_get_d(CONFIG_STEREO)      ? 1 : 0;
    int stencil = config_get_d(CONFIG_REFLECTION)  ? 1 : 0;
    int buffers = config_get_d(CONFIG_MULTISAMPLE) ? 1 : 0;
    int samples = config_get_d(CONFIG_MULTISAMPLE);

    SDL_GL_SetAttribute(SDL_GL_STEREO,             stereo);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,       stencil);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, buffers);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samples);

    /* Try to set the currently specified mode. */

    if (SDL_SetVideoMode(w, h, 0, SDL_OPENGL | (f ? SDL_FULLSCREEN : 0)))
    {
        config_set_d(CONFIG_FULLSCREEN, f);
        config_set_d(CONFIG_WIDTH,      w);
        config_set_d(CONFIG_HEIGHT,     h);

        glViewport(0, 0, w, h);
        glClearColor(0.0f, 0.0f, 0.1f, 0.0f);

        glEnable(GL_NORMALIZE);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glEnable(GL_BLEND);

        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,
                      GL_SEPARATE_SPECULAR_COLOR);
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthFunc(GL_LEQUAL);

        if (config_get_d(CONFIG_VSYNC))
            sync_init();

        /* If GL supports multisample, and SDL got a multisample buffer... */

#ifdef GL_ARB_multisample
        if (check_extension("ARB_multisample"))
        {
            SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &buffers);
            if (buffers)
                glEnable(GL_MULTISAMPLE_ARB);
        }
#endif

        return 1;
    }

    /* If the mode failed, try it without stereo. */

    else if (stereo)
    {
        config_set_d(CONFIG_STEREO, 0);
        return config_mode(f, w, h);
    }

    /* If the mode failed, try decreasing the level of multisampling. */

    else if (buffers)
    {
        config_set_d(CONFIG_MULTISAMPLE, samples / 2);
        return config_mode(f, w, h);
    }

    /* If that mode failed, try it without reflections. */

    else if (stencil)
    {
        config_set_d(CONFIG_REFLECTION, 0);
        return config_mode(f, w, h);
    }

    /* If THAT mode failed, punt. */

    return 0;
}

/*---------------------------------------------------------------------------*/

static float ms     = 0;
static int   fps    = 0;
static int   last   = 0;
static int   ticks  = 0;
static int   frames = 0;

int  config_perf(void)
{
    return fps;
}

void config_swap(void)
{
    int dt;

    SDL_GL_SwapBuffers();

    /* Accumulate time passed and frames rendered. */

    dt = (int) SDL_GetTicks() - last;

    frames +=  1;
    ticks  += dt;
    last   += dt;

    /* Average over 250ms. */

    if (ticks > 1000)
    {
        /* Round the frames-per-second value to the nearest integer. */

        double k = 1000.0 * frames / ticks;
        double f = floor(k);
        double c = ceil (k);

        /* Compute frame time and frames-per-second stats. */

        fps = (int) ((c - k < k - f) ? c : f);
        ms  = (float) ticks / (float) frames;

        /* Reset the counters for the next update. */

        frames = 0;
        ticks  = 0;

        /* Output statistics if configured. */

        if (option_d[CONFIG_STATS])
            fprintf(stdout, "%4d %8.4f\n", fps, ms);
    }
}

/*---------------------------------------------------------------------------*/

void config_set_d(int i, int d)
{
    option_d[i] = d;
    dirty = 1;
}

void config_tgl_d(int i)
{
    option_d[i] = (option_d[i] ? 0 : 1);
    dirty = 1;
}

int config_tst_d(int i, int d)
{
    return (option_d[i] == d) ? 1 : 0;
}

int config_get_d(int i)
{
    return option_d[i];
}

/*---------------------------------------------------------------------------*/

void config_set_s(int i, const char *src)
{
    int len = (int) strlen(src);

    if (option_s[i])
        free(option_s[i]);

    if ((option_s[i] = (char *) malloc(len + 1)))
        strncpy(option_s[i], src, len + 1);

    dirty = 1;
}

void config_get_s(int i, char *dst, int len)
{
    strncpy(dst, option_s[i], len);
}

/*---------------------------------------------------------------------------*/

static int grabbed = 0;

void config_set_grab(int w)
{
    if (w)
    {
        SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

        SDL_WarpMouse(config_get_d(CONFIG_WIDTH)  / 2,
                      config_get_d(CONFIG_HEIGHT) / 2);

        SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
    }

    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(SDL_DISABLE);

    grabbed = 1;
}

void config_clr_grab(void)
{
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(SDL_ENABLE);
    grabbed = 0;
}

int  config_get_grab(void)
{
    return grabbed;
}

/*---------------------------------------------------------------------------*/

int config_cheat(void)
{
    return config_get_d(CONFIG_CHEAT);
}

void config_set_cheat(void)
{
    config_set_d(CONFIG_CHEAT, 1);
}

void config_clr_cheat(void)
{
    config_set_d(CONFIG_CHEAT, 0);
}

/*---------------------------------------------------------------------------*/

int config_screenshot(void)
{
    return ++option_d[CONFIG_SCREENSHOT];
}

/*---------------------------------------------------------------------------*/

void config_push_persp(float fov, float n, float f)
{
    GLdouble m[4][4];

    GLdouble r = fov / 2 * V_PI / 180;
    GLdouble s = sin(r);
    GLdouble c = cos(r) / s;

    GLdouble a = ((GLdouble) option_d[CONFIG_WIDTH] /
                  (GLdouble) option_d[CONFIG_HEIGHT]);

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();

        m[0][0] =  c/a;
        m[0][1] =  0.0;
        m[0][2] =  0.0;
        m[0][3] =  0.0;
        m[1][0] =  0.0;
        m[1][1] =    c;
        m[1][2] =  0.0;
        m[1][3] =  0.0;
        m[2][0] =  0.0;
        m[2][1] =  0.0;
        m[2][2] = -(f + n) / (f - n);
        m[2][3] = -1.0;
        m[3][0] =  0.0;
        m[3][1] =  0.0;
        m[3][2] = -2.0 * n * f / (f - n);
        m[3][3] =  0.0;

        glMultMatrixd(&m[0][0]);
    }
    glMatrixMode(GL_MODELVIEW);
}

void config_push_ortho(void)
{
    GLdouble w = (GLdouble) option_d[CONFIG_WIDTH];
    GLdouble h = (GLdouble) option_d[CONFIG_HEIGHT];

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, w, 0.0, h, -1.0, +1.0);
    }
    glMatrixMode(GL_MODELVIEW);
}

void config_pop_matrix(void)
{
    glMatrixMode(GL_PROJECTION);
    {
        glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
}

void config_clear(void)
{
    if (option_d[CONFIG_REFLECTION])
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);
    else
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);
}

/*---------------------------------------------------------------------------*/
