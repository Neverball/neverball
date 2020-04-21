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

#ifdef __EMSCRIPTEN__
#include <gl4esinit.h>
#endif

#include <SDL.h>

#include "video.h"
#include "common.h"
#include "image.h"
#include "vec3.h"
#include "glext.h"
#include "config.h"
#include "gui.h"
#include "hmd.h"

extern const char TITLE[];
extern const char ICON[];

struct video video;

/*---------------------------------------------------------------------------*/

/* Normally...... show the system cursor and hide the virtual cursor.        */
/* In HMD mode... show the virtual cursor and hide the system cursor.        */

void video_show_cursor()
{
    if (hmd_stat())
    {
        gui_set_cursor(1);
        SDL_ShowCursor(SDL_DISABLE);
    }
    else
    {
        gui_set_cursor(0);
        SDL_ShowCursor(SDL_ENABLE);
    }
}

/* When the cursor is to be hidden, make sure neither the virtual cursor     */
/* nor the system cursor is visible.                                         */

void video_hide_cursor()
{
    gui_set_cursor(0);
    SDL_ShowCursor(SDL_DISABLE);
}

/*---------------------------------------------------------------------------*/

static char snapshot_path[MAXSTR] = "";

static void snapshot_init(void)
{
    snapshot_path[0] = 0;
}

static void snapshot_prep(const char *path)
{
    if (path && *path)
        SAFECPY(snapshot_path, path);
}

static void snapshot_take(void)
{
    if (snapshot_path[0])
    {
        image_snap(snapshot_path);
        snapshot_path[0] = 0;
    }
}

void video_snap(const char *path)
{
    snapshot_prep(path);
}

/*---------------------------------------------------------------------------*/

static SDL_Window    *window;
static SDL_GLContext  context;

static void set_window_title(const char *title)
{
    SDL_SetWindowTitle(window, title);
}

static void set_window_icon(const char *filename)
{
#if !defined(__APPLE__)
    SDL_Surface *icon;

    if ((icon = load_surface(filename)))
    {
        SDL_SetWindowIcon(window, icon);
        free(icon->pixels);
        SDL_FreeSurface(icon);
    }
#endif
    return;
}

int video_display(void)
{
    if (window)
        return SDL_GetWindowDisplayIndex(window);
    else
        return -1;
}

int video_init(void)
{
    if (!video_mode(config_get_d(CONFIG_FULLSCREEN),
                    config_get_d(CONFIG_WIDTH),
                    config_get_d(CONFIG_HEIGHT)))
    {
        log_printf("Failure to create window (%s)\n", SDL_GetError());
        return 0;
    }

    return 1;
}

int video_mode(int f, int w, int h)
{
    int stereo  = config_get_d(CONFIG_STEREO)      ? 1 : 0;
    int stencil = config_get_d(CONFIG_REFLECTION)  ? 1 : 0;
    int buffers = config_get_d(CONFIG_MULTISAMPLE) ? 1 : 0;
    int samples = config_get_d(CONFIG_MULTISAMPLE);
    int vsync   = config_get_d(CONFIG_VSYNC)       ? 1 : 0;
    int hmd     = config_get_d(CONFIG_HMD)         ? 1 : 0;
    int highdpi = config_get_d(CONFIG_HIGHDPI)     ? 1 : 0;

    int dpy = config_get_d(CONFIG_DISPLAY);

    int X = SDL_WINDOWPOS_CENTERED_DISPLAY(dpy);
    int Y = SDL_WINDOWPOS_CENTERED_DISPLAY(dpy);

    hmd_free();

    if (window)
    {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
    }

#if ENABLE_OPENGLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

    SDL_GL_SetAttribute(SDL_GL_STEREO,             stereo);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,       stencil);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, buffers);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samples);

    /* Require 16-bit double buffer with 16-bit depth buffer. */

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    /* Try to set the currently specified mode. */

    log_printf("Creating a window (%dx%d, %s)\n",
               w, h, (f ? "fullscreen" : "windowed"));

    window = SDL_CreateWindow("", X, Y, w, h,
                              SDL_WINDOW_OPENGL |
                              (highdpi ? SDL_WINDOW_ALLOW_HIGHDPI : 0) |
                              (f ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));

    if (window)
    {
        if ((context = SDL_GL_CreateContext(window)))
        {
            int buf, smp;
#ifdef __EMSCRIPTEN__
            initialize_gl4es();
#endif

            SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &buf);
            SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &smp);

            /*
             * Work around SDL+WGL returning pixel formats below
             * minimum specifications instead of failing, thus
             * bypassing our fallback path. SDL tries to ensure that
             * WGL plays by the rules, but forgets about extended
             * context attributes such as multisample. See SDL
             * Bugzilla #77.
             */

            if (buf < buffers || smp < samples)
            {
                log_printf("GL context does not meet minimum specifications\n");
                SDL_GL_DeleteContext(context);
                context = NULL;
            }
        }
    }

    if (window && context)
    {
        set_window_title(TITLE);
        set_window_icon(ICON);

        /*
         * SDL_GetWindowSize can be unreliable when going fullscreen
         * on OSX (and possibly elsewhere). We should really be
         * waiting for a resize / size change event, but for now we're
         * doing this lazy thing instead.
         */

        if (f)
        {
            SDL_DisplayMode dm;

            if (SDL_GetDesktopDisplayMode(video_display(), &dm) == 0)
            {
                video.window_w = dm.w;
                video.window_h = dm.h;
            }
        }
        else
        {
            SDL_GetWindowSize(window, &video.window_w, &video.window_h);
        }

        if (highdpi)
        {
            SDL_GL_GetDrawableSize(window, &video.device_w, &video.device_h);
        }
        else
        {
            video.device_w = video.window_w;
            video.device_h = video.window_h;
        }

        video.device_scale = (float) video.device_h / (float) video.window_h;

        log_printf("Created a window (%u, %dx%d, %s)\n",
                   SDL_GetWindowID(window),
                   video.window_w, video.window_h,
                   (f ? "fullscreen" : "windowed"));

        config_set_d(CONFIG_DISPLAY,    video_display());
        config_set_d(CONFIG_FULLSCREEN, f);
        config_set_d(CONFIG_WIDTH,      video.window_w);
        config_set_d(CONFIG_HEIGHT,     video.window_h);

        SDL_GL_SetSwapInterval(vsync);

        if (!glext_init())
            return 0;

        glViewport(0, 0, video.device_w, video.device_h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        glEnable(GL_NORMALIZE);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
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

        /* Set up HMD display if requested. */

        if (hmd)
            hmd_init();

        /* Initialize screen snapshotting. */

        snapshot_init();

        video_show_cursor();

        /* Grab input immediately in HMD mode. */

        if (hmd_stat())
            SDL_SetWindowGrab(window, SDL_TRUE);

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

    if (hmd_stat())
        hmd_swap();

    /* Take a screenshot of the complete back buffer and swap it. */

    snapshot_take();

    SDL_GL_SwapWindow(window);

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
            fprintf(stdout, "%4d %8.4f\n", fps, (double) ms);
    }
}

/*---------------------------------------------------------------------------*/

static int grabbed = 0;

void video_set_grab(int w)
{
#ifdef NDEBUG
    if (w)
    {
        SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

        SDL_WarpMouseInWindow(window,
                              video.window_w / 2,
                              video.window_h / 2);

        SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
    }

    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_SetWindowGrab(window, SDL_TRUE);
    video_hide_cursor();
#endif

    grabbed = 1;
}

void video_clr_grab(void)
{
#ifdef NDEBUG
    SDL_SetRelativeMouseMode(SDL_FALSE);

    /* Never release the grab in HMD mode. */

    if (!hmd_stat())
        SDL_SetWindowGrab(window, SDL_FALSE);

    video_show_cursor();
#endif
    grabbed = 0;
}

int  video_get_grab(void)
{
    return grabbed;
}

/*---------------------------------------------------------------------------*/

void video_calc_view(float *M, const float *c,
                               const float *p,
                               const float *u)
{
    float x[3];
    float y[3];
    float z[3];

    v_sub(z, p, c);
    v_nrm(z, z);
    v_crs(x, u, z);
    v_nrm(x, x);
    v_crs(y, z, x);

    m_basis(M, x, y, z);
}

/*---------------------------------------------------------------------------*/

void video_push_persp(float fov, float n, float f)
{
    if (hmd_stat())
        hmd_persp(n, f);
    else
    {
        GLfloat m[4][4];

        GLfloat r = fov / 2 * V_PI / 180;
        GLfloat s = fsinf(r);
        GLfloat c = fcosf(r) / s;

        GLfloat a = ((GLfloat) video.device_w /
                     (GLfloat) video.device_h);

        glMatrixMode(GL_PROJECTION);
        {
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
        {
            glLoadIdentity();
        }
    }
}

void video_push_ortho(void)
{
    if (hmd_stat())
        hmd_ortho();
    else
    {
        GLfloat w = (GLfloat) video.device_w;
        GLfloat h = (GLfloat) video.device_h;

        glMatrixMode(GL_PROJECTION);
        {
            glLoadIdentity();
            glOrtho_(0.0, w, 0.0, h, -1.0, +1.0);
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glLoadIdentity();
        }
    }
}

void video_pop_matrix(void)
{
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
