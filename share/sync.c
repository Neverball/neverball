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

#include <SDL_syswm.h>
#include "glext.h"

/*---------------------------------------------------------------------------*/
#if defined(__APPLE__)

#include <OpenGL/OpenGL.h>

void sync_init(void)
{
    GLint swap = 1;
    CGLSetParameter(CGLGetCurrentContext(),  kCGLCPSwapInterval, &swap);
}

/*---------------------------------------------------------------------------*/
#elif SDL_VIDEO_DRIVER_X11

#include <GL/glx.h>

static int search(const char *haystack, const char *needle)
{
    const char *c;

    for (; *haystack; haystack++)
    {
        for (c = needle; *c && *haystack; c++, haystack++)
            if (*c != *haystack)
                break;

        if ((*c == 0) && (*haystack == ' ' || *haystack == '\0'))
            return 1;
    }

    return 0;
}

void sync_init(void)
{
    SDL_SysWMinfo info;
    Display *dpy;

    SDL_VERSION(&info.version);

    if (SDL_GetWMInfo(&info) != 1)
        return;

    if (info.subsystem != SDL_SYSWM_X11)
        return;

    dpy = info.info.x11.display;

    info.info.x11.lock_func();
    {
        int scr = DefaultScreen(dpy);

        PFNGLXSWAPINTERVALSGIPROC _glXSwapIntervalSGI = NULL;

        if (search(glXQueryExtensionsString(dpy, scr), "GLX_SGI_swap_control"))
        {
            if ((_glXSwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
                 glXGetProcAddress((const GLubyte *) "glXSwapIntervalSGI")))
                _glXSwapIntervalSGI(1);
        }
    }
    info.info.x11.unlock_func();
}

/*---------------------------------------------------------------------------*/
#else

void sync_init(void)
{
    return;
}

#endif