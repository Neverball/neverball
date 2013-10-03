/*
 * Copyright (C) 2013 Robert Kooima
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

#include <string.h>

#include "fbo.h"

/*---------------------------------------------------------------------------*/

GLboolean fbo_create(fbo *F, GLsizei w, GLsizei h)
{
    F->width  = w;
    F->height = h;

    glGenTextures     (1, &F->color_texture);
    glGenTextures     (1, &F->depth_texture);
    glGenFramebuffers_(1, &F->framebuffer);

    glBindTexture  (GL_TEXTURE_2D, F->color_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D   (GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
                    GL_RGBA, GL_UNSIGNED_INT, NULL);

    glBindTexture  (GL_TEXTURE_2D, F->depth_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D   (GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, w, h, 0,
                    GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

    glBindFramebuffer_     (GL_FRAMEBUFFER, F->framebuffer);
    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D, F->color_texture, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_TEXTURE_2D, F->depth_texture, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_TEXTURE_2D, F->depth_texture, 0);

    if (glCheckFramebufferStatus_(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        glBindFramebuffer_ (GL_FRAMEBUFFER, 0);
        return GL_TRUE;
    }
    else
    {
        glBindFramebuffer_ (GL_FRAMEBUFFER, 0);
        return GL_FALSE;
    }
}

void fbo_delete(fbo *F)
{
    if (F->color_texture) glDeleteTextures     (1, &F->color_texture);
    if (F->depth_texture) glDeleteTextures     (1, &F->depth_texture);
    if (F->framebuffer)   glDeleteFramebuffers_(1, &F->framebuffer);

    memset(F, 0, sizeof (fbo));
}

/*---------------------------------------------------------------------------*/
