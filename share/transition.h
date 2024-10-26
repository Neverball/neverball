/*
 * Copyright (C) 2024 Jānis Rūcis
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

#ifndef TRANSITION_H
#define TRANSITION_H

#define INTENT_FORWARD 0
#define INTENT_BACK 1

void transition_init(void);
void transition_quit(void);

void transition_add(int id);
void transition_remove(int id);

void transition_timer(float dt);
void transition_paint(void);

int transition_slide(int id, int enter, int intent);

#endif