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

void tilt_init(void)
{
}

void tilt_free(void)
{
}

int tilt_stat(void)
{
    return 0;
}

int  tilt_get_button(int *b, int *s)
{
    return 0;
}

void tilt_get_direct(int *x, int *y)
{
    *x = 1;
    *y = 1;
}

float tilt_get_x(void)
{
    return 0.0f;
}

float tilt_get_z(void)
{
    return 0.0f;
}
