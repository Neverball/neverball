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
#include <string.h>

#include "fs.h"

/*---------------------------------------------------------------------------*/

void put_float(fs_file fout, float f)
{
    unsigned int val = 0;

    memcpy(&val, &f, sizeof (val));

    fs_putc((val)       & 0xff, fout);
    fs_putc((val >> 8)  & 0xff, fout);
    fs_putc((val >> 16) & 0xff, fout);
    fs_putc((val >> 24) & 0xff, fout);
}

void put_index(fs_file fout, int val)
{
    fs_putc((val)       & 0xff, fout);
    fs_putc((val >> 8)  & 0xff, fout);
    fs_putc((val >> 16) & 0xff, fout);
    fs_putc((val >> 24) & 0xff, fout);
}

void put_short(fs_file fout, short val)
{
    fs_putc((val)      & 0xff, fout);
    fs_putc((val >> 8) & 0xff, fout);
}

void put_array(fs_file fout, const float *v, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++)
        put_float(fout, v[i]);
}

/*---------------------------------------------------------------------------*/

float get_float(fs_file fin)
{
    unsigned int val =
        (fs_getc(fin) & 0xff)       |
        (fs_getc(fin) & 0xff) << 8  |
        (fs_getc(fin) & 0xff) << 16 |
        (fs_getc(fin) & 0xff) << 24;

    float f = 0.0f;

    memcpy(&f, &val, sizeof (f));

    return f;
}

int get_index(fs_file fin)
{
    int val =
        (fs_getc(fin) & 0xff)       |
        (fs_getc(fin) & 0xff) << 8  |
        (fs_getc(fin) & 0xff) << 16 |
        (fs_getc(fin) & 0xff) << 24;

    return val;
}

short get_short(fs_file fin)
{
    short val = (fs_getc(fin) & 0xff) | (fs_getc(fin) & 0xff) << 8;

    return val;
}

void get_array(fs_file fin, float *v, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++)
        v[i] = get_float(fin);
}

/*---------------------------------------------------------------------------*/

void put_string(fs_file fout, const char *s)
{
    fs_puts(s, fout);
    fs_putc('\0', fout);
}

void get_string(fs_file fin, char *s, size_t max)
{
    size_t pos = 0;
    int c;

    while ((c = fs_getc(fin)) >= 0)
    {
        if (pos < max)
        {
            s[pos++] = c;

            /* Terminate the string, but keep reading until NUL. */

            if (pos == max)
                s[pos - 1] = 0;
        }

        if (c == 0)
            break;
    }
}

/*---------------------------------------------------------------------------*/
