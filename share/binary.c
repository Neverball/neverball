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

#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_byteorder.h>

/*---------------------------------------------------------------------------*/

void put_float(FILE *fout, const float *f)
{
    unsigned char *p = (unsigned char *) f;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    fputc((int) p[3], fout);
    fputc((int) p[2], fout);
    fputc((int) p[1], fout);
    fputc((int) p[0], fout);
#else
    fputc((int) p[0], fout);
    fputc((int) p[1], fout);
    fputc((int) p[2], fout);
    fputc((int) p[3], fout);
#endif
}

void put_int(FILE *fout, const int *i)
{
    unsigned char *p = (unsigned char *) i;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    fputc((int) p[3], fout);
    fputc((int) p[2], fout);
    fputc((int) p[1], fout);
    fputc((int) p[0], fout);
#else
    fputc((int) p[0], fout);
    fputc((int) p[1], fout);
    fputc((int) p[2], fout);
    fputc((int) p[3], fout);
#endif
}

void put_short(FILE *fout, const short *s)
{
    unsigned char *p = (unsigned char *) s;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    fputc((int) p[1], fout);
    fputc((int) p[0], fout);
#else
    fputc((int) p[0], fout);
    fputc((int) p[1], fout);
#endif
}

void put_array(FILE *fout, const float *v, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++)
        put_float(fout, v + i);
}

/*---------------------------------------------------------------------------*/

void get_float(FILE *fin, float *f)
{
    unsigned char *p = (unsigned char *) f;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    p[3] = (unsigned char) fgetc(fin);
    p[2] = (unsigned char) fgetc(fin);
    p[1] = (unsigned char) fgetc(fin);
    p[0] = (unsigned char) fgetc(fin);
#else
    p[0] = (unsigned char) fgetc(fin);
    p[1] = (unsigned char) fgetc(fin);
    p[2] = (unsigned char) fgetc(fin);
    p[3] = (unsigned char) fgetc(fin);
#endif
}

void get_int(FILE *fin, int *i)
{
    unsigned char *p = (unsigned char *) i;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    p[3] = (unsigned char) fgetc(fin);
    p[2] = (unsigned char) fgetc(fin);
    p[1] = (unsigned char) fgetc(fin);
    p[0] = (unsigned char) fgetc(fin);
#else
    p[0] = (unsigned char) fgetc(fin);
    p[1] = (unsigned char) fgetc(fin);
    p[2] = (unsigned char) fgetc(fin);
    p[3] = (unsigned char) fgetc(fin);
#endif
}

void get_short(FILE *fin, short *s)
{
    unsigned char *p = (unsigned char *) s;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    p[1] = (unsigned char) fgetc(fin);
    p[0] = (unsigned char) fgetc(fin);
#else
    p[0] = (unsigned char) fgetc(fin);
    p[1] = (unsigned char) fgetc(fin);
#endif
}

void get_array(FILE *fin, float *v, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++)
        get_float(fin, v + i);
}

/*---------------------------------------------------------------------------*/
