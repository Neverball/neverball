/*   
 *   Copyright (C) 2003 Robert Kooima
 *
 *   SUPER EMPTY BALL is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by the
 *   Free Software Foundation;  either version 2 of the  License, or (at your
 *   option) any later version.
 *
 *   This program  is distributed  in the  hope that it  will be  useful, but
 *   WITHOUT   ANY   WARRANTY;  without   even   the   implied  warranty   of
 *   MERCHANTABILITY  or  FITNESS FOR  A  PARTICULAR  PURPOSE.   See the  GNU
 *   General Public License for more details.
 */

#include <SDL/SDL_image.h>

#include "gl.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

static const GLenum format[5] = {
    0,
    GL_LUMINANCE,
    GL_LUMINANCE_ALPHA,
    GL_RGB,
    GL_RGBA
};

int image_load(struct image *image, const char *filename)
{
    if ((image->s = IMG_Load(filename)))
    {
        void *p = image->s->pixels;
        int   w = image->s->w;
        int   h = image->s->h;
        int   b = image->s->format->BytesPerPixel;
        int   f = format[b];

        glGenTextures(1, &image->o);
        glBindTexture(GL_TEXTURE_2D, image->o);

        glTexImage2D(GL_TEXTURE_2D, 0, b, w, h, 0, f, GL_UNSIGNED_BYTE, p);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return 1;
    }
    return 0;
}

int image_test(struct image *image)
{
    return (image->s && glIsTexture(image->o)) ? 1 : 0;
}

void image_free(struct image *image)
{
    if (image->o) glDeleteTextures(1, &image->o);
    if (image->s) SDL_FreeSurface(image->s);

    image->s = NULL;
    image->o = 0;
}

/*---------------------------------------------------------------------------*/

void image_bind(struct image *image)
{
    glBindTexture(GL_TEXTURE_2D, image->o);
}

void image_rect(struct image *image,
                double x0, double y0,
                double x1, double y1, double a)
{
    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
    }
    glMatrixMode(GL_MODELVIEW);

    glBindTexture(GL_TEXTURE_2D, image->o);

    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_COLOR_MATERIAL);

        glBegin(GL_QUADS);
        {
            glColor4d(1.0, 1.0, 1.0, a);

            glTexCoord2d(0.0, 1.0); glVertex2d(x0, y0);
            glTexCoord2d(1.0, 1.0); glVertex2d(x1, y0);
            glTexCoord2d(1.0, 0.0); glVertex2d(x1, y1);
            glTexCoord2d(0.0, 0.0); glVertex2d(x0, y1);
        }
        glEnd();
    }
    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    {
        glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------------*/

