#include <GL/gl.h>

#include "main.h"
#include "game.h"

/*---------------------------------------------------------------------------*/

static void play_enter(void)
{
}

static void play_leave(void)
{
}

static void play_paint(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    game_paint();
}

static int play_point(int x, int y)
{
    return game_point(x, y);
}

static int play_click(int d)
{
    return game_click(d);
}

static int play_keybd(int c)
{
    return game_keybd(c);
}

/*---------------------------------------------------------------------------*/

struct state st_play = {
    play_enter,
    play_leave,
    play_paint,
    play_point,
    play_click,
    play_keybd
};
