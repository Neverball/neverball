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
#include <string.h>

#include "glext.h"
#include "back.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

static GLUquadric *back_quad = NULL;
static GLuint      back_list;
static GLuint      back_text;

/*---------------------------------------------------------------------------*/

#define STRMAX 256

void back_init(const char *s, int b)
{
    back_text = make_image_from_file(NULL, NULL, s);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if ((back_quad = gluNewQuadric()))
    {
        int slices = b ? 64 : 32;
        int stacks = b ? 32 : 16;

        gluQuadricOrientation(back_quad, GLU_INSIDE);
        gluQuadricNormals(back_quad, GLU_SMOOTH);
        gluQuadricTexture(back_quad, GL_TRUE);

        back_list = glGenLists(1);
    
        glNewList(back_list, GL_COMPILE);
        {
            glColor3f(1.0f, 1.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, back_text);
            gluSphere(back_quad, 1.0, slices, stacks);
        }
        glEndList();
    }
}

void back_free(void)
{
    if (glIsList(back_list))
        glDeleteLists(back_list, 1);

    if (glIsTexture(back_text))
        glDeleteTextures(1, &back_text);
}

void back_draw(void)
{
    glPushAttrib(GL_LIGHTING_BIT);
    glPushMatrix();
    {
        glDisable(GL_LIGHTING);

        glScalef(BACK_DIST, BACK_DIST, BACK_DIST);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

        glCallList(back_list);
    }
    glPopMatrix();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/
