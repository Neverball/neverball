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

#include "main.h"
#include "play.h"
#include "game.h"
#include "title.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

#define PAUSE_S "data/png/pause.png"

static struct image pause_i, *pause_p = &pause_i;

/*---------------------------------------------------------------------------*/

static void pause_enter(void)
{
    image_load(pause_p, PAUSE_S);
}

static void pause_leave(void)
{
    image_free(pause_p);
}

static void pause_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    game_render_env();
    image_rect(pause_p, 0.0, 0.33, 1.0, 0.66, 0.5);
}

static int pause_keybd(int c)
{
    if (c == SDLK_F1)
        goto_state(&st_title);

    if (c == SDLK_ESCAPE)
        goto_state(&st_play);

    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_pause = {
    pause_enter,
    pause_leave,
    pause_paint,
    NULL,
    NULL,
    NULL,
    pause_keybd
};
