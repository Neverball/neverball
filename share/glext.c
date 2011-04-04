/*
 * Copyright (C) 2003-2011 Neverball authors
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

#include "glext.h"

/*---------------------------------------------------------------------------*/

#ifndef CONF_OPENGLES

PFNGLACTIVETEXTURE_PROC    glActiveTexture_;

PFNGLGENBUFFERS_PROC       glGenBuffers_;
PFNGLBINDBUFFER_PROC       glBindBuffer_;
PFNGLBUFFERDATA_PROC       glBufferData_;
PFNGLBUFFERSUBDATA_PROC    glBufferSubData_;
PFNGLDELETEBUFFERS_PROC    glDeleteBuffers_;
PFNGLISBUFFER_PROC         glIsBuffer_;

PFNGLPOINTPARAMETERFV_PROC glPointParameterfv_;

#endif

/*---------------------------------------------------------------------------*/

int glext_check(const char *needle)
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

/*---------------------------------------------------------------------------*/

void glext_init(void)
{
#ifndef CONF_OPENGLES

    if (glext_check("ARB_multitexture"))
    {
        glActiveTexture_ = (PFNGLACTIVETEXTURE_PROC)
            SDL_GL_GetProcAddress("glActiveTextureARB");
    }

    if (glext_check("ARB_vertex_buffer_object"))
    {
        glGenBuffers_    = (PFNGLGENBUFFERS_PROC)
            SDL_GL_GetProcAddress("glGenBuffersARB");
        glBindBuffer_    = (PFNGLBINDBUFFER_PROC)
            SDL_GL_GetProcAddress("glBindBufferARB");
        glBufferData_    = (PFNGLBUFFERDATA_PROC)
            SDL_GL_GetProcAddress("glBufferDataARB");
        glBufferSubData_ = (PFNGLBUFFERSUBDATA_PROC)
            SDL_GL_GetProcAddress("glBufferSubDataARB");
        glDeleteBuffers_ = (PFNGLDELETEBUFFERS_PROC)
            SDL_GL_GetProcAddress("glDeleteBuffersARB");
        glIsBuffer_      = (PFNGLISBUFFER_PROC)
            SDL_GL_GetProcAddress("glIsBufferARB");
    }

    if (glext_check("ARB_point_parameters"))
    {
        glPointParameterfv_ = (PFNGLPOINTPARAMETERFV_PROC)
            SDL_GL_GetProcAddress("glPointParameterfvARB");
    }

#endif
}

/*---------------------------------------------------------------------------*/
