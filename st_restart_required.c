#include "gui.h"
#include "hud.h"
#include "geom.h"
#include "ball.h"
#include "part.h"
#include "game.h"
#include "audio.h"
#include "config.h"
#include "video.h"

#include "st_restart_required.h"

/*---------------------------------------------------------------------------*/

enum
{
    RESTART_REQUIRED_OK
};

static int restart_required_action(int i)
{
    switch (i)
    {
    case RESTART_REQUIRED_OK:
        return 0;
        break;
    }
    return 1;
}

static int restart_required_enter(struct state *st, struct state *prev)
{
    int id;

    back_init("back/gui.png");

    /* Initialize the configuration GUI. */

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Restart required!"), GUI_MED, gui_red, gui_red);
        gui_space(id);
        gui_multi(id, _("Please restart Sunnyball after exit\\to change this affects!"), GUI_SML, gui_wht, gui_wht);
        gui_space(id);
        gui_state(id, _("Exit"), GUI_SML, RESTART_REQUIRED_OK, 0);
        gui_layout(id, 0, 0);
    }

    return id;
}

static void restart_required_leave(struct state *st, struct state *next, int id)
{
    back_free();
    gui_delete(id);
}

static void restart_required_paint(int id, float st)
{
    video_push_persp((float) config_get_d(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
    {
        back_draw_easy();
    }
    video_pop_matrix();
    gui_paint(id);
}

static void restart_required_timer(int id, float dt)
{
    gui_timer(id, dt);
}

static void restart_required_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void restart_required_stick(int id, int a, float v, int bump)
{
    gui_pulse(gui_stick(id, a, v, bump), 1.2f);
}

static int restart_required_click(int b, int d)
{
    if (gui_click(b, d))
        return restart_required_action(RESTART_REQUIRED_OK);

    return 1;
}

static int restart_required_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? restart_required_action(RESTART_REQUIRED_OK) : 1;
}

static int restart_required_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return restart_required_action(RESTART_REQUIRED_OK);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return restart_required_action(RESTART_REQUIRED_OK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/
struct state st_restart_required = {
    restart_required_enter,
    restart_required_leave,
    restart_required_paint,
    restart_required_timer,
    restart_required_point,
    restart_required_stick,
    NULL,
    restart_required_click,
    restart_required_keybd,
    restart_required_buttn
};
