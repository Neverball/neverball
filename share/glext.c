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

static int shadow = 1;

int glext_shadow()
{
    return shadow;
}

void glext_set_shadow(int s)
{
    shadow = s;
}

/*---------------------------------------------------------------------------*/

#ifdef _WIN32

PFNGLACTIVETEXTUREPROC   glActiveTexture   = NULL;
PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f = NULL;

void glext_init(void)
{
    glActiveTexture   = (PFNGLACTIVETEXTUREPROC)
        wglGetProcAddress("glActiveTexture");
    glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)
        wglGetProcAddress("glMultiTexCoord2f");

    if (glActiveTexture   == NULL)
        glActiveTexture   = (PFNGLACTIVETEXTUREPROC)
            wglGetProcAddress("glActiveTextureARB");
    if (glMultiTexCoord2f == NULL)
        glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)
            wglGetProcAddress("glMultiTexCoord2fARB");

    if (glActiveTexture == NULL)
        shadow = 0;
    if (glMultiTexCoord2f == NULL)
        shadow = 0;
}

#else

void glext_init(void)
{
    /* Nothing to do! */
}

#endif /* _WIN32 */

/*---------------------------------------------------------------------------*/
