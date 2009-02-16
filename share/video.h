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

#ifndef VIDEO_H
#define VIDEO_H

/*---------------------------------------------------------------------------*/

int  config_mode(int, int, int);

int  config_perf(void);
void config_sync(void);
void config_swap(void);

void config_set_grab(int w);
void config_clr_grab(void);
int  config_get_grab(void);

/*---------------------------------------------------------------------------*/

void config_push_persp(float, float, float);
void config_push_ortho(void);
void config_pop_matrix(void);
void config_clear(void);

/*---------------------------------------------------------------------------*/

#endif
