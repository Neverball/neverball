/*
 * Copyright (C) 2007 Robert Kooima
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

#include "mode.h"
#include "lang.h"

/*---------------------------------------------------------------------------*/

static int mode = MODE_NORMAL;

/*---------------------------------------------------------------------------*/

int curr_mode(void)
{
    return mode;
}

void mode_set(int new_mode)
{
    mode = new_mode;
}

const char *mode_to_str(int m, int l)
{
    switch (m)
    {
    case MODE_CHALLENGE: return l ? _("Challenge Mode") : _("Challenge");
    case MODE_NORMAL:    return l ? _("Normal Mode")    : _("Normal");
    case MODE_PRACTICE:  return l ? _("Practice Mode")  : _("Practice");
    default:             return l ? _("Unknown Mode")   : _("Unknown");
    }
}

/*---------------------------------------------------------------------------*/
