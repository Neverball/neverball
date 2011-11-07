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
#include "set.h"
#include "util.h"
#include "progress.h"
#include "audio.h"
#include "config.h"
#include "common.h"

#include "game_common.h"

#include "st_set.h"
#include "st_level.h"
#include "st_start.h"
#include "st_title.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

enum
{
    START_BACK = GUI_LAST,
    START_CHALLENGE,
    START_LOCK_GOALS,
    START_LEVEL
};

static int shot_id;
static int file_id;
static int challenge_id;

/*---------------------------------------------------------------------------*/

/* Create a level selector button based upon its existence and status. */

static void gui_level(int id, int i)
{
    struct level *l = get_level(i);

    const GLubyte *fore = 0;
    const GLubyte *back = 0;

    int jd;

    if (!l)
    {
        gui_label(id, " ", GUI_SML, gui_blk, gui_blk);
        return;
    }

    if (level_opened(l))
    {
        fore = level_bonus(l)     ? gui_grn : gui_wht;
        back = level_completed(l) ? fore    : gui_yel;
    }

    jd = gui_label(id, level_name(l), GUI_SML, back, fore);

    if (level_opened(l) || config_cheat())
        gui_set_state(jd, START_LEVEL, i);
}

static void start_over_level(int i)
{
    struct level *l = get_level(i);

    if (level_opened(l) || config_cheat())
    {
        gui_set_image(shot_id, level_shot(l));

        set_score_board(level_score(l, SCORE_COIN), -1,
                        level_score(l, SCORE_TIME), -1,
                        level_score(l, SCORE_GOAL), -1);

        if (file_id)
            gui_set_label(file_id, level_file(l));
    }
}

static void start_over(int id, int pulse)
{
    int tok;

    if (id == 0)
        return;

    if (pulse)
        gui_pulse(id, 1.2f);

    tok = gui_token(id);

    if (tok == START_CHALLENGE || tok == START_BACK)
    {
        gui_set_image(shot_id, set_shot(curr_set()));

        set_score_board(set_score(curr_set(), SCORE_COIN), -1,
                        set_score(curr_set(), SCORE_TIME), -1,
                        NULL, -1);
    }

    if (tok == START_LEVEL)
        start_over_level(gui_value(id));
}

/*---------------------------------------------------------------------------*/

static int start_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case START_BACK:
        return goto_state(&st_set);

    case START_CHALLENGE:
        if (config_cheat())
        {
            progress_init(curr_mode() == MODE_CHALLENGE ?
                          MODE_NORMAL : MODE_CHALLENGE);
            gui_toggle(challenge_id);
            return 1;
        }
        else
        {
            progress_init(MODE_CHALLENGE);
            return start_action(START_LEVEL, 0);
        }
        break;

    case GUI_SCORE:
        /* FIXME, there's no need to rebuild the entire screen. */
        gui_score_set(val);
        return goto_state(&st_start);

    case START_LOCK_GOALS:
        config_set_d(CONFIG_LOCK_GOALS, val);
        return goto_state(&st_start);

    case START_LEVEL:
        if (progress_play(get_level(val)))
            return goto_state(&st_level);

        break;
    }

    return 1;
}

static int start_gui(void)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);
    int i, j;

    int id, jd, kd, ld;

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {

            gui_label(jd, set_name(curr_set()), GUI_SML, gui_yel, gui_red);
            gui_filler(jd);
            gui_start(jd, _("Back"),  GUI_SML, START_BACK, 0);
        }

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            if (config_cheat())
            {
                if ((kd = gui_vstack(jd)))
                {
                    shot_id = gui_image(kd, set_shot(curr_set()),
                                        6 * w / 16, 6 * h / 16);
                    file_id = gui_label(kd, " ", GUI_SML, gui_yel, gui_red);
                }
            }
            else
            {
                shot_id = gui_image(jd, set_shot(curr_set()),
                                    7 * w / 16, 7 * h / 16);
            }

            if ((kd = gui_varray(jd)))
            {
                for (i = 0; i < 5; i++)
                    if ((ld = gui_harray(kd)))
                        for (j = 4; j >= 0; j--)
                            gui_level(ld, i * 5 + j);

                challenge_id = gui_state(kd, _("Challenge"), GUI_SML,
                                         START_CHALLENGE, 0);

                gui_set_hilite(challenge_id, curr_mode() == MODE_CHALLENGE);
            }
        }
        gui_space(id);
        gui_score_board(id, (GUI_SCORE_COIN |
                             GUI_SCORE_TIME |
                             GUI_SCORE_GOAL), 0, 0);
        gui_space(id);

        if ((jd = gui_hstack(id)))
        {
            gui_filler(jd);

            if ((kd = gui_harray(jd)))
            {
                int btn0, btn1;

                btn0 = gui_state(kd,
                                 /* Translators: adjust the amount of
                                  * whitespace here as necessary for
                                  * the buttons to look good. */
                                 _("   No   "), GUI_SML, START_LOCK_GOALS, 0);

                btn1 = gui_state(kd, _("Yes"), GUI_SML, START_LOCK_GOALS, 1);

                if (config_get_d(CONFIG_LOCK_GOALS))
                    gui_set_hilite(btn1, 1);
                else
                    gui_set_hilite(btn0, 1);
            }

            gui_space(jd);

            gui_label(jd, _("Lock Goals of Completed Levels?"), GUI_SML, 0, 0);

            gui_filler(jd);
        }

        gui_layout(id, 0, 0);

        if (file_id)
            gui_set_trunc(file_id, TRUNC_HEAD);

        set_score_board(NULL, -1, NULL, -1, NULL, -1);
    }

    return id;
}

static int start_enter(struct state *st, struct state *prev)
{
    progress_init(MODE_NORMAL);

    audio_music_fade_to(0.5f, "bgm/inter.ogg");

    return start_gui();
}

static void start_point(int id, int x, int y, int dx, int dy)
{
    start_over(gui_point(id, x, y), 1);
}

static void start_stick(int id, int a, float v, int bump)
{
    start_over(gui_stick(id, a, v, bump), 1);
}

static int start_score(int d)
{
    int s = (d < 0 ?
             GUI_SCORE_PREV(gui_score_get()) :
             GUI_SCORE_NEXT(gui_score_get()));

    gui_score_set(s);
    start_over(gui_active(), 0.0f);
    return 1;
}

static int start_keybd(int c, int d)
{
    if (d)
    {
        if (c == SDLK_c && config_cheat())
        {
            set_cheat();
            return goto_state(&st_start);
        }
        else if (c == SDLK_F12 && config_cheat())
        {
            char *dir = concat_string("Screenshots/shot-",
                                      set_id(curr_set()), NULL);
            int i;

            fs_mkdir(dir);

            /* Iterate over all levels, taking a screenshot of each. */

            for (i = 0; i < MAXLVL; i++)
                if (level_exists(i))
                    level_snap(i, dir);

            free(dir);
        }
        else if (config_tst_d(CONFIG_KEY_SCORE_NEXT, c))
            return start_score(+1);
    }

    return 1;
}

static int start_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return start_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return start_action(START_BACK, 0);
    }
    return 1;
}

static int start_click(int b, int d)
{
    if (gui_click(b, d))
    {
        return start_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 1);
    }

    if (d)
    {
        if (b == SDL_BUTTON_WHEELUP)   start_score(-1);
        if (b == SDL_BUTTON_WHEELDOWN) start_score(+1);
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_start = {
    start_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    start_point,
    start_stick,
    shared_angle,
    start_click,
    start_keybd,
    start_buttn
};
