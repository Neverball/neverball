#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <glw.h>
#include <vec.h>
#include <sol.h>
#include <aio.h>

#include <GL/glu.h>

#include "game.h"
#include "play.h"

#define TITLE "SUPER EMPTY BALL"
#define WINIT 800
#define HINIT 600

/*---------------------------------------------------------------------------*/

static double now(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0;
}

/*---------------------------------------------------------------------------*/

static double goto_time    = 0.0;
static struct state *state = NULL;

void goto_state(struct state *st)
{
    if (state && state->leave)
        state->leave();

    state     = st;
    goto_time = now();

    if (state && state->leave)
        state->leave();
}

double time_state(void)
{
    return now() - goto_time;
}

static void st_shape(int w, int h)
{
    glViewport(0, 0, w, h);
}

static void st_paint(void)
{
    if (state && state->paint)
        state->paint();
}

static int st_point(int x, int y)
{
    if (state && state->point)
        return state->point(x, y);
    else
        return 0;
}

static int st_click(int b)
{
    if (state && state->click)
        return state->click(b);
    else
        return 0;
}

static int st_keybd(int c)
{
    if (state && state->keybd)
        return state->keybd(c);
    else
        return 0;
}

/*---------------------------------------------------------------------------*/

static int loop(int swap)
{
    int e, d = 0, s = swap;

    while (d >= 0 && (e = glw_update(s, 0)))
    {
        int x = glw_x();
        int y = glw_y();

        switch (e)
        {
        case GLW_RESIZE:      st_shape(x, y); break;
        case GLW_MOTION:  d = st_point(x, y); break;
        case GLW_L_BTN_D: d = st_click(1);    break;
        case GLW_L_BTN_U: d = st_click(0);    break;
        case GLW_KEY_D:   d = st_keybd(x);    break;
        case GLW_RENDER:  d = +1;             break;
        case GLW_CLOSE:   d = -1;
        }

        s = 0;
    }
    return d;
}

int main(void)
{
    int    d = 0;
    double t = now();

    if (glw_create(TITLE, WINIT, HINIT, 1))
    {
        if (aio_init() == 0)
        {
            game_init();
            goto_state(&st_play);

            while ((d = loop(1)) >= 0)
            {
                double s = now();

                game_step(s - t);
                st_paint();

                t = s;
            }
            aio_fini();
        }
        glw_delete();
    }
    return 0;
}
