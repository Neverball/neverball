#include <stdlib.h>
#include <GL/gl.h>

#include <aio.h>
#include <aux.h>

#include "main.h"
#include "title.h"
#include "game.h"

#define TIME_S "data/png/time.png"

static void  *time_p = NULL;
static int    time_w = 0;
static int    time_h = 0;
static int    time_b = 0;
static GLuint time_o = 0;

/*---------------------------------------------------------------------------*/

static void time_enter(void)
{
    if (time_p == NULL)
    {
        time_p = aux_load_png(TIME_S, &time_w, &time_h, &time_b);
        time_o = aux_make_tex(time_p,  time_w,  time_h,  time_b);
    }
}

static void time_leave(void)
{
}

static void time_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    game_render_env();

    aux_proj_identity();
    aux_draw_tex(time_o, 0.0, 0.33, 1.0, 0.66, 1.0);
}

static int time_timer(double dt)
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

static int time_point(int x, int y)
{
    return 0;
}

static int time_click(int d)
{
    return 0;
}

static int time_keybd(int c)
{
    return 0;
}

/*---------------------------------------------------------------------------*/

struct state st_time = {
    time_enter,
    time_leave,
    time_paint,
    time_timer,
    time_point,
    time_click,
    time_keybd,
};
