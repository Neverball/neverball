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

#include "gui.h"
#include "config.h"
#include "game.h"
#include "levels.h"
#include "level.h"
#include "audio.h"
#include "hud.h"

#include "st_play.h"
#include "st_over.h"
#include "st_shared.h"
#include "st_pause.h"

#define PAUSE_CONTINUE 1
#define PAUSE_RESTART  2
#define PAUSE_EXIT     3

/*---------------------------------------------------------------------------*/

static struct state *st_continue;
static int paused;

int goto_pause(void)
{
    st_continue = curr_state();
    paused = 1;
    return goto_state(&st_pause);
}

int is_paused(void)
{
    return paused;
}

void clear_pause(void)
{
    paused = 0;
}

/*---------------------------------------------------------------------------*/

static int pause_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch(i)
    {
    case PAUSE_CONTINUE:
        SDL_PauseAudio(0);
        config_set_grab(0);
        return goto_state(st_continue);

    case PAUSE_RESTART:
        level_same();
        clear_pause();
        SDL_PauseAudio(0);
        config_set_grab(1);
        return goto_state(&st_play_ready);

    case PAUSE_EXIT:
        level_stat(GAME_NONE, curr_clock(), curr_coins());
        level_stop();
        clear_pause();
        SDL_PauseAudio(0);
        audio_music_stop();
        return goto_state(&st_over);
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static int pause_enter(void)
{
    int id, jd, title_id;

    config_clr_grab();
    SDL_PauseAudio(1);

    /* Build the pause GUI. */

    if ((id = gui_vstack(0)))
    {
        title_id = gui_label(id, _("Paused"), GUI_LRG, GUI_ALL, 0, 0);

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_state(jd, _("Quit"), GUI_SML, PAUSE_EXIT, 0);

            if (curr_lg()->mode != MODE_CHALLENGE)
                gui_state(jd, _("Restart"), GUI_SML, PAUSE_RESTART, 0);
            else
            {
                int ld = gui_state(jd, _("Restart"), GUI_SML, 0, 0);
                gui_set_color(ld, gui_gry, gui_gry);
            }

            gui_start(jd, _("Continue"), GUI_SML, PAUSE_CONTINUE, 1);
        }

        gui_pulse(title_id, 1.2f);
        gui_layout(id, 0, 0);
    }

    hud_update(0);

    return id;
}

static void pause_paint(int id, float st)
{
    shared_paint(id, st);
    hud_paint();
}

static void pause_timer(int id, float dt)
{
    gui_timer(id, dt);
    hud_timer (dt);
}

static int pause_keybd(int c, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_KEY_PAUSE, c))
            return pause_action(PAUSE_CONTINUE);

        if (config_tst_d(CONFIG_KEY_RESTART, c)
            && curr_lg()->mode != MODE_CHALLENGE)
            return pause_action(PAUSE_RESTART);
    }
    return 1;
}

static int pause_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return pause_action(gui_token(gui_click()));

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
        {
            if (SDL_GetModState() & KMOD_SHIFT)
                return pause_action(PAUSE_EXIT);
            else
                return pause_action(PAUSE_CONTINUE);
        }
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_pause = {
    pause_enter,
    shared_leave,
    pause_paint,
    pause_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    pause_keybd,
    pause_buttn,
    1, 0
};
