#include <stdlib.h>
#include <GL/gl.h>

#include <aio.h>
#include <aux.h>

#include "main.h"
#include "title.h"
#include "game.h"

#define FALL_S "data/png/fall.png"

static void  *fall_p = NULL;
static int    fall_w = 0;
static int    fall_h = 0;
static int    fall_b = 0;
static GLuint fall_o = 0;

/*---------------------------------------------------------------------------*/

static void fall_enter(void)
{
    if (fall_p == NULL)
    {
        fall_p = aux_load_png(FALL_S, &fall_w, &fall_h, &fall_b);
        fall_o = aux_make_tex(fall_p,  fall_w,  fall_h,  fall_b);
    }
}

static void fall_leave(void)
{
}

static void fall_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    game_render_env();

    aux_proj_identity();
    aux_draw_tex(fall_o, 0.0, 0.33, 1.0, 0.66, 1.0);
}

static int fall_timer(double dt)
{
    if (time_state() < 2.0)
    {
        double g[3] = { 0.0, -9.8, 0.0 };

        (void) game_update_env(g, dt);
    }
    else
    {
        if (game_fail())
            goto_state(&st_level);
        else
            goto_state(&st_gameover);
    }

    return 1;
}

static int fall_point(int x, int y)
{
    return 0;
}

static int fall_click(int d)
{
    return 0;
}

static int fall_keybd(int c)
{
    return 0;
}

/*---------------------------------------------------------------------------*/

struct state st_fall = {
    fall_enter,
    fall_leave,
    fall_paint,
    fall_timer,
    fall_point,
    fall_click,
    fall_keybd,
};
