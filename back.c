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

static GLuint back_list;

static GLuint back_u;
static GLuint back_n;
static GLuint back_s;
static GLuint back_w;
static GLuint back_e;
static GLuint back_d;

/*---------------------------------------------------------------------------*/

#define STRMAX 256

static GLuint back_load(const char *root, char *suff)
{
    char filename[STRMAX];
    GLuint text;

    /* Try to load an uncompressed copy of the image. */

    strncpy(filename, root, STRMAX - 8);
    strcat (filename, suff);
    strcat (filename, ".tga");

    if ((text = make_image_from_file(NULL, NULL, filename)))
        return text;

    /* Try to load a lossless copy of the image. */

    strncpy(filename, root, STRMAX - 8);
    strcat (filename, suff);
    strcat (filename, ".png");

    if ((text = make_image_from_file(NULL, NULL, filename)))
        return text;

    /* Try to load a lossy copy of the image. */

    strncpy(filename, root, STRMAX - 8);
    strcat (filename, suff);
    strcat (filename, ".jpg");

    if ((text = make_image_from_file(NULL, NULL, filename)))
        return text;

    return 0;
}

void back_init(const char *s)
{
    back_u = back_load(s, "_u");
    back_n = back_load(s, "_n");
    back_s = back_load(s, "_s");
    back_w = back_load(s, "_w");
    back_e = back_load(s, "_e");
    back_d = back_load(s, "_d");

    back_list = glGenLists(1);

    glNewList(back_list, GL_COMPILE);
    { 
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glBindTexture(GL_TEXTURE_2D, back_u);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 1); glVertex3i(-1, +1, -1);
            glTexCoord2i(1, 1); glVertex3i(+1, +1, -1);
            glTexCoord2i(1, 0); glVertex3i(+1, +1, +1);
            glTexCoord2i(0, 0); glVertex3i(-1, +1, +1);
        }
        glEnd();

        glBindTexture(GL_TEXTURE_2D, back_n);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 1); glVertex3i(-1, -1, -1);
            glTexCoord2i(1, 1); glVertex3i(+1, -1, -1);
            glTexCoord2i(1, 0); glVertex3i(+1, +1, -1);
            glTexCoord2i(0, 0); glVertex3i(-1, +1, -1);
        }
        glEnd();

        glBindTexture(GL_TEXTURE_2D, back_s);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 1); glVertex3i(+1, -1, +1);
            glTexCoord2i(1, 1); glVertex3i(-1, -1, +1);
            glTexCoord2i(1, 0); glVertex3i(-1, +1, +1);
            glTexCoord2i(0, 0); glVertex3i(+1, +1, +1);
        }
        glEnd();

        glBindTexture(GL_TEXTURE_2D, back_w);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 1); glVertex3i(-1, -1, +1);
            glTexCoord2i(1, 1); glVertex3i(-1, -1, -1);
            glTexCoord2i(1, 0); glVertex3i(-1, +1, -1);
            glTexCoord2i(0, 0); glVertex3i(-1, +1, +1);
        }
        glEnd();

        glBindTexture(GL_TEXTURE_2D, back_e);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 1); glVertex3i(+1, -1, -1);
            glTexCoord2i(1, 1); glVertex3i(+1, -1, +1);
            glTexCoord2i(1, 0); glVertex3i(+1, +1, +1);
            glTexCoord2i(0, 0); glVertex3i(+1, +1, -1);
        }
        glEnd();

        glBindTexture(GL_TEXTURE_2D, back_d);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 1); glVertex3i(-1, -1, +1);
            glTexCoord2i(1, 1); glVertex3i(+1, -1, +1);
            glTexCoord2i(1, 0); glVertex3i(+1, -1, -1);
            glTexCoord2i(0, 0); glVertex3i(-1, -1, -1);
        }
        glEnd();
    }
    glEndList();
}

void back_free(void)
{
    if (glIsList(back_list))
        glDeleteLists(back_list, 1);

    if (glIsTexture(back_d))
        glDeleteTextures(1, &back_d);
    if (glIsTexture(back_e))
        glDeleteTextures(1, &back_e);
    if (glIsTexture(back_w))
        glDeleteTextures(1, &back_w);
    if (glIsTexture(back_s))
        glDeleteTextures(1, &back_s);
    if (glIsTexture(back_n))
        glDeleteTextures(1, &back_n);
    if (glIsTexture(back_u))
        glDeleteTextures(1, &back_u);
}

void back_draw(void)
{
    glPushAttrib(GL_LIGHTING_BIT);
    glPushMatrix();
    {
        glDisable(GL_LIGHTING);

        glScalef(BACK_DIST, BACK_DIST, BACK_DIST);

        /* My crappy skyboxes somehow look better looking east. */
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

        glCallList(back_list);
    }
    glPopMatrix();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/
