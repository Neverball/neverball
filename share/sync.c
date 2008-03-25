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

#include "glext.h"

/*---------------------------------------------------------------------------*/
#ifndef __APPLE__

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

#endif
/*---------------------------------------------------------------------------*/
#ifdef __linux__

#include <GL/glx.h>

void sync_init(void)
{
    Display *dpy;

    if ((dpy = glXGetCurrentDisplay()))
    {
        int scr = DefaultScreen(dpy);

        PFNGLXSWAPINTERVALSGIPROC _glXSwapInvervalSGI = NULL;

        if (search(glXQueryExtensionsString(dpy, scr), "GLX_SGI_swap_control"))
        {
            if ((_glXSwapInvervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
                 glXGetProcAddress((const GLubyte *) "glXSwapIntervalSGI")))
                _glXSwapInvervalSGI(1);
        }
    }
}

#endif
/*---------------------------------------------------------------------------*/
#ifdef _WIN32

void sync_init(void)
{
/* TODO: Sit down at a Windows machine and make this work.
    PFNWGLSWAPINTERVALEXTPROC _wglSwapInvervalEXT = NULL;

    if (search(wglGetExtensionsString(), "WGL_EXT_swap_control"))
    {
        if ((_wglSwapInvervalEXT = (PFNGLXSWAPINTERVALEXTPROC)
             wglGetProcAddress((const GLubyte *) "wglSwapIntervalEXT")))
            _wglSwapInvervalEXT(1);
    }
*/
}

#endif
/*---------------------------------------------------------------------------*/
#ifdef __APPLE__

#include <OpenGL/OpenGL.h>

void sync_init(void)
{
    long swap = 1;
    CGLSetParameter(CGLGetCurrentContext(),  kCGLCPSwapInterval, &swap);
}

#endif
/*---------------------------------------------------------------------------*/
