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

#include "video.h"
#include "vec3.h"
#include "glext.h"
#include "config.h"
#include "syswm.h"
#include "sync.h"

/*---------------------------------------------------------------------------*/

int video_init(const char *title, const char *icon)
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return 0;
    }

    /* This has to happen before mode setting... */

    set_SDL_icon(icon);

    /* Initialize the video. */

    if (!video_mode(config_get_d(CONFIG_FULLSCREEN),
                    config_get_d(CONFIG_WIDTH),
                    config_get_d(CONFIG_HEIGHT)))
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return 0;
    }

    /* ...and this has to happen after it. */

    set_EWMH_icon(icon);

    SDL_WM_SetCaption(title, title);

    return 1;
}

/*---------------------------------------------------------------------------*/

int video_mode(int f, int w, int h)
{
    int stereo  = config_get_d(CONFIG_STEREO)      ? 1 : 0;
    int stencil = config_get_d(CONFIG_REFLECTION)  ? 1 : 0;
    int buffers = config_get_d(CONFIG_MULTISAMPLE) ? 1 : 0;
    int samples = config_get_d(CONFIG_MULTISAMPLE);
    int vsync   = config_get_d(CONFIG_VSYNC)       ? 1 : 0;

    SDL_GL_SetAttribute(SDL_GL_STEREO,             stereo);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,       stencil);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, buffers);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samples);
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL,       vsync);

    /* Require 16-bit double buffer with 16-bit depth buffer. */

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    /* Try to set the currently specified mode. */

    if (SDL_SetVideoMode(w, h, 0, SDL_OPENGL | (f ? SDL_FULLSCREEN : 0)))
    {
        config_set_d(CONFIG_FULLSCREEN, f);
        config_set_d(CONFIG_WIDTH,      w);
        config_set_d(CONFIG_HEIGHT,     h);

        if (!glext_init())
            return 0;

        glViewport(0, 0, w, h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        glEnable(GL_NORMALIZE);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glEnable(GL_BLEND);

#if !ENABLE_OPENGLES
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,
                      GL_SEPARATE_SPECULAR_COLOR);
#endif

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthFunc(GL_LEQUAL);

        /* If GL supports multisample, and SDL got a multisample buffer... */

        if (glext_check("ARB_multisample"))
        {
            SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &buffers);
            if (buffers) glEnable(GL_MULTISAMPLE);
        }

        /* Attempt manual swap control if SDL's is broken. */

        if (vsync && SDL_GL_GetAttribute(SDL_GL_SWAP_CONTROL, &vsync) == -1)
            sync_init();

        return 1;
    }

    /* If the mode failed, try it without stereo. */

    else if (stereo)
    {
        config_set_d(CONFIG_STEREO, 0);
        return video_mode(f, w, h);
    }

    /* If the mode failed, try decreasing the level of multisampling. */

    else if (buffers)
    {
        config_set_d(CONFIG_MULTISAMPLE, samples / 2);
        return video_mode(f, w, h);
    }

    /* If that mode failed, try it without reflections. */

    else if (stencil)
    {
        config_set_d(CONFIG_REFLECTION, 0);
        return video_mode(f, w, h);
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

int  video_perf(void)
{
    return fps;
}

void video_swap(void)
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

        if (config_get_d(CONFIG_STATS))
            fprintf(stdout, "%4d %8.4f\n", fps, ms);
    }
}

/*---------------------------------------------------------------------------*/

static int grabbed = 0;

void video_set_grab(int w)
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

void video_clr_grab(void)
{
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(SDL_ENABLE);
    grabbed = 0;
}

int  video_get_grab(void)
{
    return grabbed;
}

/*---------------------------------------------------------------------------*/

void video_push_persp(float fov, float n, float f)
{
    GLfloat m[4][4];

    GLfloat r = fov / 2 * V_PI / 180;
    GLfloat s = sin(r);
    GLfloat c = cos(r) / s;

    GLfloat a = ((GLfloat) config_get_d(CONFIG_WIDTH) /
                 (GLfloat) config_get_d(CONFIG_HEIGHT));

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();

        m[0][0] = c / a;
        m[0][1] =  0.0f;
        m[0][2] =  0.0f;
        m[0][3] =  0.0f;
        m[1][0] =  0.0f;
        m[1][1] =     c;
        m[1][2] =  0.0f;
        m[1][3] =  0.0f;
        m[2][0] =  0.0f;
        m[2][1] =  0.0f;
        m[2][2] = -(f + n) / (f - n);
        m[2][3] = -1.0f;
        m[3][0] =  0.0f;
        m[3][1] =  0.0f;
        m[3][2] = -2.0f * n * f / (f - n);
        m[3][3] =  0.0f;

        glMultMatrixf(&m[0][0]);
    }
    glMatrixMode(GL_MODELVIEW);
}

void video_push_ortho(void)
{
    GLfloat w = (GLfloat) config_get_d(CONFIG_WIDTH);
    GLfloat h = (GLfloat) config_get_d(CONFIG_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();
        glOrtho_(0.0, w, 0.0, h, -1.0, +1.0);
    }
    glMatrixMode(GL_MODELVIEW);
}

void video_pop_matrix(void)
{
    glMatrixMode(GL_PROJECTION);
    {
        glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
}

void video_clear(void)
{
    if (config_get_d(CONFIG_REFLECTION))
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);
    else
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);
}

/*---------------------------------------------------------------------------*/
