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

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "gl.h"
#include "image.h"

int image_scale = 1;

static const GLenum format[5] = {
    0,
    GL_LUMINANCE,
    GL_LUMINANCE_ALPHA,
    GL_RGB,
    GL_RGBA
};

/*---------------------------------------------------------------------------*/

SDL_Surface *image_file(const char *filename)
{
    SDL_Surface *src = IMG_Load(filename);
    SDL_Surface *dst = src;

    if (src && image_scale > 1)
    {
        dst = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                   src->w / image_scale,
                                   src->h / image_scale,
                                   src->format->BitsPerPixel,
                                   src->format->Rmask,
                                   src->format->Gmask,
                                   src->format->Bmask,
                                   src->format->Amask);

        SDL_LockSurface(src);
        SDL_LockSurface(dst);
        {
            gluScaleImage(format[src->format->BytesPerPixel],
                          src->w, src->h, GL_UNSIGNED_BYTE, src->pixels,
                          dst->w, dst->h, GL_UNSIGNED_BYTE, dst->pixels);
        }
        SDL_UnlockSurface(dst);
        SDL_UnlockSurface(src);
    }

    return dst;
}

/*---------------------------------------------------------------------------*/

int image_load(struct image *image, const char *filename)
{
    if ((image->s = image_file(filename)))
    {
        void *p = image->s->pixels;
        int   w = image->s->w;
        int   h = image->s->h;
        int   b = image->s->format->BytesPerPixel;
        int   f = format[b];

        glGenTextures(1, &image->o);
        glBindTexture(GL_TEXTURE_2D, image->o);

        glTexImage2D(GL_TEXTURE_2D, 0, b, w, h, 0, f, GL_UNSIGNED_BYTE, p);

#ifdef GL_CLAMP_TO_EDGE
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif
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

int image_w(const struct image *image)
{
    return (image && image->s) ? image->s->w : 0;
}

int image_h(const struct image *image)
{
    return (image && image->s) ? image->s->h : 0;
}

void image_bind(const struct image *image)
{
    if (image)
        glBindTexture(GL_TEXTURE_2D, image->o);
}

