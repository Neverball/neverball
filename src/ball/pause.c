#include <GL/gl.h>
#include <stdlib.h>

#include <aux.h>

#include "main.h"
#include "play.h"
#include "game.h"

#define PAUSE_S "data/png/pause.png"

static void  *pause_p = NULL;
static int    pause_w = 0;
static int    pause_h = 0;
static int    pause_b = 0;
static GLuint pause_o = 0;

/*---------------------------------------------------------------------------*/

static void pause_enter(void)
{
    if (pause_p == NULL)
    {
        pause_p = aux_load_png(PAUSE_S, &pause_w, &pause_h, &pause_b);
        pause_o = aux_make_tex(pause_p,  pause_w,  pause_h,  pause_b);
    }
}

static void pause_leave(void)
{
}

static void pause_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    game_render_env();

    aux_proj_identity();
    aux_draw_tex(pause_o, 0.0, 0.33, 1.0, 0.66, 0.5);
}

static int pause_timer(double dt)
{
    return 0;
}

static int pause_point(int x, int y)
{
    return 0;
}

static int pause_click(int d)
{
    return 0;
}

static int pause_keybd(int c)
{
    if (c == 24)
        return -1;
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
    pause_leave,
    pause_paint,
    pause_timer,
    pause_point,
    pause_click,
    pause_keybd
};
