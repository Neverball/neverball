/*   Copyright (C) 2003  Robert Kooima                                       */
/*                                                                           */
/*   SUPER EMPTY BALL  is  free software; you  can redistribute  it and/or   */
/*   modify  it under  the  terms  of  the  GNU General Public License  as   */
/*   published by  the Free Software Foundation;  either version 2  of the   */
/*   License, or (at your option) any later version.                         */
/*                                                                           */
/*   This program is  distributed in the hope that it  will be useful, but   */
/*   WITHOUT  ANY   WARRANTY;  without   even  the  implied   warranty  of   */
/*   MERCHANTABILITY  or FITNESS FOR  A PARTICULAR  PURPOSE.  See  the GNU   */
/*   General Public License for more details.                                */

#include <GL/gl.h>
#include <stdlib.h>

#include <etc.h>

#include "main.h"
#include "title.h"
#include "play.h"
#include "game.h"

#define PAUSE_S "data/png/pause.png"

static void  *pause_p = NULL;
static GLuint pause_o = 0;

/*---------------------------------------------------------------------------*/

static void pause_enter(void)
{
    if (pause_p == NULL)
    {
        int w, h, b;

        pause_p = etc_load_png(PAUSE_S, &w, &h, &b);
        pause_o = etc_make_tex(pause_p,  w,  h,  b);
    }
}

static void pause_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    game_render_env();

    etc_proj_identity();
    etc_draw_tex(pause_o, 0.0, 0.33, 1.0, 0.66, 0.5);
}

static int pause_keybd(int c)
{
    if (c == 17)
    {
        goto_state(&st_title);
        return +1;
    }
    if (c == 27)
    {
        goto_state(&st_play);
        return +1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

struct state st_pause = {
    pause_enter,
    NULL,
    pause_paint,
    NULL,
    NULL,
    NULL,
    pause_keybd
};
