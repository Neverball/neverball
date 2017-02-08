/*
 * Copyright (C) 2003 Robert Kooima - 2006 Jean Privat
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
#include <ctype.h>

#include "common.h"
#include "gui.h"
#include "util.h"
#include "audio.h"
#include "config.h"
#include "video.h"
#include "text.h"
#include "geom.h"

#include "game_common.h"
#include "game_server.h"
#include "game_client.h"

#include "st_name.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

static struct state *ok_state;
static struct state *cancel_state;

static unsigned int draw_back;

int goto_name(struct state *ok, struct state *cancel, unsigned int back)
{
    ok_state     = ok;
    cancel_state = cancel;
    draw_back    = back;

    return goto_state(&st_name);
}

/*---------------------------------------------------------------------------*/

enum
{
    NAME_OK = GUI_LAST
};

static int name_id;

static int name_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
        return goto_state(cancel_state);

    case NAME_OK:
        if (strlen(text_input) == 0)
           return 1;

        config_set_s(CONFIG_PLAYER, text_input);

        return goto_state(ok_state);

    case GUI_CL:
        gui_keyboard_lock();
        break;

    case GUI_BS:
        text_input_del();
        break;

    case GUI_CHAR:
        text_input_char(val);
        break;
    }
    return 1;
}

static int enter_id;

static int name_gui(void)
{
    int id, jd;

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Player Name"), GUI_MED, 0, 0);
        gui_space(id);

        name_id = gui_label(id, " ", GUI_MED, gui_yel, gui_yel);

        gui_space(id);
        gui_keyboard(id);
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            enter_id = gui_start(jd, _("OK"), GUI_SML, NAME_OK, 0);
            gui_space(jd);
            gui_state(jd, _("Cancel"), GUI_SML, GUI_BACK, 0);
        }

        gui_layout(id, 0, 0);

        gui_set_trunc(name_id, TRUNC_HEAD);
        gui_set_label(name_id, text_input);
    }

    return id;
}

static void on_text_input(int typing)
{
    if (name_id)
    {
        gui_set_label(name_id, text_input);

        if (typing)
            audio_play(AUD_MENU, 1.0f);
    }
}

static int name_enter(struct state *st, struct state *prev)
{
    if (draw_back)
    {
        game_client_free(NULL);
        back_init("back/gui.png");
    }

    text_input_start(on_text_input);
    text_input_str(config_get_s(CONFIG_PLAYER), 0);

    return name_gui();
}

static void name_leave(struct state *st, struct state *next, int id)
{
    if (draw_back)
        back_free();

    text_input_stop();

    gui_delete(id);
}

static void name_paint(int id, float t)
{
    if (draw_back)
    {
        video_push_persp((float) config_get_d(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
        {
            back_draw_easy();
        }
        video_pop_matrix();
    }
    else
        game_client_draw(0, t);

    gui_paint(id);
}

static int name_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return name_action(GUI_BACK, 0);

        if (c == '\b' || c == 0x7F)
        {
            gui_focus(enter_id);
            return name_action(GUI_BS, 0);
        }
        else
        {
            gui_focus(enter_id);
            return 1;
        }
    }
    return 1;
}

static int name_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
        {
            int tok = gui_token(gui_active());
            int val = gui_value(gui_active());

            return name_action(tok, (tok == GUI_CHAR ?
                                     gui_keyboard_char(val) :
                                     val));
        }
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            name_action(GUI_BACK, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_name = {
    name_enter,
    name_leave,
    name_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    name_keybd,
    name_buttn
};

