#include <stdlib.h>
#include <GL/gl.h>

#include <aio.h>
#include <aux.h>

#include "main.h"
#include "title.h"
#include "game.h"

#define GOAL_S "data/png/goal.png"

static void  *goal_p = NULL;
static int    goal_w = 0;
static int    goal_h = 0;
static int    goal_b = 0;
static GLuint goal_o = 0;

/*---------------------------------------------------------------------------*/

static void goal_enter(void)
{
    if (goal_p == NULL)
    {
        goal_p = aux_load_png(GOAL_S, &goal_w, &goal_h, &goal_b);
        goal_o = aux_make_tex(goal_p,  goal_w,  goal_h,  goal_b);
    }
}

static void goal_leave(void)
{
}

static void goal_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    game_render_env();

    aux_proj_identity();
    aux_draw_tex(goal_o, 0.0, 0.33, 1.0, 0.66, 1.0);
}

static int goal_timer(double dt)
{
    if (time_state() < 2.0)
    {
        double g[3] = { 0.0, 9.8, 0.0 };

        (void) game_update_env(g, dt);
    }
    else
    {
        level_pass();
        goto_state(&st_level);
    }

    return 1;
}

static int goal_point(int x, int y)
{
    return 0;
}

static int goal_click(int d)
{
    return 0;
}

static int goal_keybd(int c)
{
    return 0;
}

/*---------------------------------------------------------------------------*/

struct state st_goal = {
    goal_enter,
    goal_leave,
    goal_paint,
    goal_timer,
    goal_point,
    goal_click,
    goal_keybd,
};
