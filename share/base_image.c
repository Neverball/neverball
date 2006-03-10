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

/*---------------------------------------------------------------------------*/

void image_swab(SDL_Surface *src)
{
    int i, j, b = (src->format->BitsPerPixel == 32) ? 4 : 3;
    
    SDL_LockSurface(src);
    {
        unsigned char *s = (unsigned char *) src->pixels;
        unsigned char  t;

        /* Iterate over each pixel of the image. */

        for (i = 0; i < src->h; i++)
            for (j = 0; j < src->w; j++)
            {
                int k = (i * src->w + j) * b;

                /* Swap the red and blue channels of each. */

                t        = s[k + 2];
                s[k + 2] = s[k + 0];
                s[k + 0] =        t;
            }
    }
    SDL_UnlockSurface(src);
}

void image_white(SDL_Surface *src)
{
    int i, j, b = (src->format->BitsPerPixel == 32) ? 4 : 3;
    
    SDL_LockSurface(src);
    {
        unsigned char *s = (unsigned char *) src->pixels;

        /* Iterate over each pixel of the image. */

        for (i = 0; i < src->h; i++)
            for (j = 0; j < src->w; j++)
            {
                int k = (i * src->w + j) * b;

                /* Whiten the RGB channels without touching any Alpha. */

                s[k + 0] = 0xFF;
                s[k + 1] = 0xFF;
                s[k + 2] = 0xFF;
            }
    }
    SDL_UnlockSurface(src);
}

SDL_Surface *image_scale(SDL_Surface *src, int n)
{
    int si, di;
    int sj, dj;
    int k, b = (src->format->BitsPerPixel == 32) ? 4 : 3;

    SDL_Surface *dst = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                            src->w / n,
                                            src->h / n,
                                            src->format->BitsPerPixel,
                                            src->format->Rmask,
                                            src->format->Gmask,
                                            src->format->Bmask,
                                            src->format->Amask);
    if (dst)
    {
        SDL_LockSurface(src);
        SDL_LockSurface(dst);
        {
            unsigned char *s = (unsigned char *) src->pixels;
            unsigned char *d = (unsigned char *) dst->pixels;

            /* Iterate each component of each distination pixel. */

            for (di = 0; di < src->h / n; di++)
                for (dj = 0; dj < src->w / n; dj++)
                    for (k = 0; k < b; k++)
                    {
                        int c = 0;

                        /* Average the NxN source pixel block for each. */

                        for (si = di * n; si < (di + 1) * n; si++)
                            for (sj = dj * n; sj < (dj + 1) * n; sj++)
                                c += s[(si * src->w + sj) * b + k];

                        d[(di * dst->w + dj) * b + k] =
                            (unsigned char) (c / (n * n));
                    }
        }
        SDL_UnlockSurface(dst);
        SDL_UnlockSurface(src);
    }

    return dst;
}

/*---------------------------------------------------------------------------*/
