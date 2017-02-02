/*
 * Copyright (C) 2003 Robert Kooima
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

#include "gui.h"
#include "util.h"
#include "audio.h"
#include "config.h"
#include "demo.h"
#include "progress.h"
#include "text.h"
#include "common.h"

#include "game_common.h"

#include "st_save.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

static struct state *ok_state;
static struct state *cancel_state;

int goto_save(struct state *ok, struct state *cancel)
{
    ok_state     = ok;
    cancel_state = cancel;

    return goto_state(&st_save);
}

/*---------------------------------------------------------------------------*/

static int file_id;

enum
{
    SAVE_SAVE = GUI_LAST
};

//char text_input[MAXSTR];

static int save_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
        return goto_state(cancel_state);

    case SAVE_SAVE:
        if (strlen(text_input) == 0)
            return 1;

        if (demo_exists(text_input))
        {
            return goto_state(&st_clobber);
        }
        else
        {
            demo_rename(text_input);
            return goto_state(ok_state);
        }

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

static int save_gui(void)
{
    int id, jd;

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Replay Name"), GUI_MED, 0, 0);
        gui_space(id);

        file_id = gui_label(id, " ", GUI_MED, gui_yel, gui_yel);

        gui_space(id);
        gui_keyboard(id);
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            enter_id = gui_start(jd, _("Save"), GUI_SML, SAVE_SAVE, 0);
            gui_space(jd);
            gui_state(jd, _("Cancel"), GUI_SML, GUI_BACK, 0);
        }

        gui_layout(id, 0, 0);

        gui_set_trunc(file_id, TRUNC_HEAD);
        gui_set_label(file_id, text_input);
    }

    return id;
}

static void on_text_input(int typing)
{
    if (file_id)
    {
        gui_set_label(file_id, text_input);

        if (typing)
            audio_play(AUD_MENU, 1.0f);
    }
}

static int save_enter(struct state *st, struct state *prev)
{
    const char *name;

    name = demo_format_name(config_get_s(CONFIG_REPLAY_NAME),
                            set_id(curr_set()),
                            level_name(curr_level()));

    text_input_start(on_text_input);
    text_input_str(name, 0);

    return save_gui();
}

static void save_leave(struct state *st, struct state *next, int id)
{
    text_input_stop();

    gui_delete(id);
}

static int save_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return save_action(GUI_BACK, 0);

        if (c == '\b' || c == 0x7F)
        {
            gui_focus(enter_id);
            return save_action(GUI_BS, 0);
        }
        else
        {
            gui_focus(enter_id);
            return 1;
        }
    }
    return 1;
}

static int save_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
        {
            int tok = gui_token(gui_active());
            int val = gui_value(gui_active());

            return save_action(tok, (tok == GUI_CHAR ?
                                     gui_keyboard_char(val) :
                                     val));
        }
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return save_action(GUI_BACK, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int clobber_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    if (tok == SAVE_SAVE)
    {
        demo_rename(text_input);
        return goto_state(ok_state);
    }
    return goto_state(&st_save);
}

static int clobber_gui(void)
{
    int id, jd, kd, ld;

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, _("Overwrite?"), GUI_MED, gui_red, gui_red);
        ld = gui_label(id, "MMMMMMMM", GUI_MED, gui_yel, gui_yel);

        if ((jd = gui_harray(id)))
        {
            gui_start(jd, _("Cancel"),    GUI_SML, GUI_BACK, 0);
            gui_state(jd, _("Overwrite"), GUI_SML, SAVE_SAVE, 0);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);

        gui_set_trunc(ld, TRUNC_TAIL);
        gui_set_label(ld, text_input);
    }

    return id;
}

static int clobber_enter(struct state *st, struct state *prev)
{
    return clobber_gui();
}

static int clobber_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return clobber_action(GUI_BACK, 0);
    }
    return 1;
}

static int clobber_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return clobber_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return clobber_action(GUI_BACK, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_save = {
    save_enter,
    save_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    save_keybd,
    save_buttn
};

struct state st_clobber = {
    clobber_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    clobber_keybd,
    clobber_buttn
};
