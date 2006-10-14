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
#include <ctype.h>

#include "gui.h"
#include "util.h"
#include "audio.h"
#include "config.h"
#include "game.h"
#include "st_shared.h"

#include "st_name.h"

/*---------------------------------------------------------------------------*/

extern struct state st_name;

static struct state *ok_state, *cancel_state;
static char player[MAXNAM];

void name_default(void)
{
    char *login = getenv("LOGNAME");

    if (login == NULL || login[0] == '\0')
        login = "Player";

    strncpy(player, login, MAXNAM);
    player[MAXNAM - 1] = '\0';
}

int goto_name(struct state *ok, struct state *cancel)
{
    config_get_s(CONFIG_PLAYER, player, MAXNAM);

    if (player[0] == '\0')
        name_default();

    ok_state     = ok;
    cancel_state = cancel;

    return goto_state(&st_name);
}

#define NAME_BACK   2
#define NAME_CANCEL 3
#define NAME_OK     4

static int name_id;

static int name_action(int i)
{
    size_t l;

    audio_play(AUD_MENU, 1.0f);

    l = strlen(player);

    switch (i)
    {
    case NAME_OK:
        if (l == 0)
           return 1;
        config_set_s(CONFIG_PLAYER, player);
        return goto_state(ok_state);

    case NAME_BACK:
    case NAME_CANCEL:
        return goto_state(cancel_state);

    case GUI_BS:
        if (l > 0)
        {
            player[l - 1] = '\0';
            gui_set_label(name_id, player);
        }
        break;

    default:
        if (l < MAXNAM - 1)
        {
            player[l + 0] = (char) i;
            player[l + 1] = '\0';

            gui_set_label(name_id, player);
        }
    }
    return 1;
}

static int enter_id;

static int name_enter(void)
{
    int id, jd;

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Player Name"), GUI_MED, GUI_ALL, 0, 0);

        gui_space(id);
        gui_space(id);

        name_id = gui_label(id, player, GUI_MED, GUI_ALL, gui_yel, gui_yel);

        gui_space(id);

        gui_keyboard(id);
        if ((jd = gui_harray(id)))
        {
            enter_id = gui_start(jd, _("OK"), GUI_SML, NAME_OK, 0);
            gui_state(jd, _("Cancel"), GUI_SML, NAME_CANCEL, 0);
        }

        gui_layout(id, 0, 0);
    }

    SDL_EnableUNICODE(1);

    return id;
}

static void name_leave(int id)
{
    SDL_EnableUNICODE(0);
    gui_delete(id);
}

static int name_keybd(int c, int d)
{
    if (d)
        if ((c & 0xFF80) == 0)
        {
            gui_focus(enter_id);
            c &= 0x7F;
            if (c == '\b' || c == 0x7F)
                return name_action(GUI_BS);
            else if (c > ' ')
                return name_action(c);
        }
    return 1;
}

static int name_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return name_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            name_action(NAME_BACK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_name = {
    name_enter,
    name_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_click,
    name_keybd,
    name_buttn,
    1, 0
};

