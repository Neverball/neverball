/*
 * Copyright (C) 2003 Robert Kooima - 2006 Jean Privat
 * Part of the Neverball Project http://icculus.org/neverball/
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */


#include <string.h>

#include "gui.h"
#include "geom.h"
#include "part.h"
#include "audio.h"
#include "config.h"
#include "video.h"

#include "st_resol.h"

extern struct state  st_null;
static struct state *st_back;

/*---------------------------------------------------------------------------*/

struct mode
{
    int w;
    int h;
};

static const struct mode modes[] = {
    { 2560, 1440 },
    { 1920, 1200 },
    { 1920, 1080 },
    { 1680, 1050 },
    { 1600, 1200 },
    { 1600, 900 },
    { 1440, 900 },
    { 1366, 768 },
    { 1280, 1024 },
    { 1280, 800 },
    { 1280, 720 },
    { 1024, 768 },
    { 800, 600 },
    { 640, 480 },
    { 480, 320 },
    { 320, 240 }
};

/*---------------------------------------------------------------------------*/

enum
{
    RESOL_BACK = 1,
    RESOL_MODE
};

static int resol_action(int tok, int val)
{
    int r = 1;

    audio_play("snd/menu.ogg", 1.0f);

    switch (tok)
    {
    case RESOL_BACK:
        goto_state(st_back);
        st_back = NULL;
        break;

    case RESOL_MODE:
        goto_state(&st_null);
        r = video_mode(config_get_d(CONFIG_FULLSCREEN),
                       modes[val].w,
                       modes[val].h);
        goto_state(&st_resol);
        break;
    }

    return r;
}

static int resol_gui(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_harray(id)))
        {
            gui_label(jd, _("Resolution"), GUI_SML, 0, 0);
            gui_space(jd);
            gui_start(jd, _("Back"),       GUI_SML, RESOL_BACK, 0);
        }

        gui_space(id);

        {
            const int W = config_get_d(CONFIG_WIDTH);
            const int H = config_get_d(CONFIG_HEIGHT);

            int i, j, n = ARRAYSIZE(modes);

            char buff[sizeof ("1234567890 x 1234567890")] = "";

            for (i = 0; i < n; i += 4)
            {
                if ((jd = gui_harray(id)))
                {
                    for (j = 3; j >= 0; j--)
                    {
                        int m = i + j;

                        if (m < n)
                        {
                            sprintf(buff, "%d x %d", modes[m].w, modes[m].h);
                            kd = gui_state(jd, buff, GUI_SML, RESOL_MODE, m);
                            gui_set_hilite(kd, (modes[m].w == W &&
                                                modes[m].h == H));
                        }
                        else
                        {
                            gui_space(jd);
                        }
                    }
                }
            }
        }

        gui_layout(id, 0, 0);
    }

    return id;
}

static int resol_enter(struct state *st, struct state *prev)
{
    if (!st_back)
    {
        /* Note the parent screen if not done yet. */

        st_back = prev;
    }

    back_init("back/gui.png");

    audio_music_fade_to(0.5f, "bgm/inter.ogg");

    return resol_gui();
}

static void resol_leave(struct state *st, struct state *next, int id)
{
    back_free();
    gui_delete(id);
}

static void resol_paint(int id, float st)
{
    video_push_persp((float) config_get_d(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
    {
        back_draw_easy();
    }
    video_pop_matrix();
    gui_paint(id);
}

static void resol_timer(int id, float dt)
{
    gui_timer(id, dt);
}

static void resol_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void resol_stick(int id, int a, float v, int bump)
{
    gui_pulse(gui_stick(id, a, v, bump), 1.2f);
}

static int resol_click(int b, int d)
{
    if (gui_click(b, d))
    {
        int active = gui_active();

        return resol_action(gui_token(active), gui_value(active));
    }
    return 1;
}

static int resol_keybd(int c, int d)
{
    return (d && c == KEY_EXIT) ? resol_action(RESOL_BACK, 0) : 1;
}

static int resol_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return resol_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return resol_action(RESOL_BACK, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_resol = {
    resol_enter,
    resol_leave,
    resol_paint,
    resol_timer,
    resol_point,
    resol_stick,
    NULL,
    resol_click,
    resol_keybd,
    resol_buttn
};

