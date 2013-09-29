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

#ifndef HMD_COMMON_H
#define HMD_COMMON_H

/*---------------------------------------------------------------------------*/

void hmd_common_init(int, int);
void hmd_common_left();
void hmd_common_right();
void hmd_common_swap(float, float, const float *, const float *);
void hmd_common_free();

/*---------------------------------------------------------------------------*/

#endif
