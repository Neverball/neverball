#include "gui.h"
#include "config.h"
#include "audio.h"
#include "game.h"
#include "state.h"

#include "st_shared.h"

void shared_leave(int id)
{
    gui_delete(id);
}

void shared_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

void shared_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

int shared_point_basic(int id, int x, int y)
/* Pulse, activate and return the active id (if changed)*/
{
    int jd = gui_point(id, x, y);
    if (jd)
        gui_pulse(jd, 1.2f);
    return jd;
}

void shared_point(int id, int x, int y, int dx, int dy)
{
    shared_point_basic(id, x, y);
}

int shared_stick_basic(int id, int a, int v)
/* Pulse, activate and return the active id (if changed)*/
{
    int jd = 0;
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        jd = gui_stick(id, v, 0);
    else if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        jd = gui_stick(id, 0, v);
    if (jd)
        gui_pulse(jd, 1.2f);
    return jd;
}

void shared_stick(int id, int a, int v)
{
    shared_stick_basic(id, a, v);
}

int shared_click(int b, int d)
{
    if (b < 0 && d == 1)
	return st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 1);
    else 
	return 1;
}

