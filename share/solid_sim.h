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

#ifndef SOLID_SIM_H
#define SOLID_SIM_H

#include "solid_vary.h"

/*---------------------------------------------------------------------------*/

void sol_init_sim(struct s_vary *);
void sol_quit_sim(void);

float sol_step(struct s_vary *, const float *, float, int, int *);

/*---------------------------------------------------------------------------*/

#endif
