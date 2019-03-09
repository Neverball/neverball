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

#include "gui.h"
#include "set.h"
#include "progress.h"
#include "powerup.h"
#include "audio.h"
#include "config.h"
#include "demo.h"

#include "game_server.h"
#include "game_client.h"
#include "game_common.h"

#include "st_level.h"
#include "st_play.h"
#include "st_start.h"
#include "st_over.h"
#include "st_done.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

static int check_nodemo = 1;

static int evalue;
static int fvalue;
static int svalue;

enum {
    START_LEVEL
};

static int shop_action(int tok, int val)
{
    switch (tok)
    {
    case START_LEVEL:
        aud_play(AUD_MENU, 1.0f);
        if (val == 3) {
            aud_play("snd/speedifier.ogg", 1.0f);
            svalue -= 1;
            config_set_d(CONSUMEABLE_ACCOUNT_SPEEDIFIER, svalue);
            init_speedifier();
        } else if (val == 2) {
            aud_play("snd/floatifier.ogg", 1.0f);
            fvalue -= 1;
            config_set_d(CONSUMEABLE_ACCOUNT_FLOATIFIER, fvalue);
            init_floatifier();
        } else if (val == 1) {
            aud_play("snd/earninator.ogg", 1.0f);
            evalue -= 1;
            config_set_d(CONSUMEABLE_ACCOUNT_EARNINATOR, evalue);
            init_earninator();
        }
        break;
    }
    return 1;
}

static int level_gui(void)
{
    evalue = config_get_d(CONSUMEABLE_ACCOUNT_EARNINATOR);
    fvalue = config_get_d(CONSUMEABLE_ACCOUNT_FLOATIFIER);
    svalue = config_get_d(CONSUMEABLE_ACCOUNT_SPEEDIFIER);

    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            gui_filler(jd);

            if ((kd = gui_vstack(jd)))
            {
                const char *ln = level_name (curr_level());
                int b          = level_bonus(curr_level());

                char setattr[MAXSTR], lvlattr[MAXSTR];

                if (b)
                    sprintf(lvlattr, _("Bonus Level %s"), ln);
                else
                    sprintf(lvlattr, _("Level %s"), ln);

                if (curr_mode() == MODE_CHALLENGE || curr_mode() == MODE_HARDCORE)
                    sprintf(setattr, "%s: %s", set_name(curr_set()),
                            mode_to_str(MODE_CHALLENGE, 1));
                else if (curr_mode() == MODE_STANDALONE)
                    sprintf(setattr, _("Standalone level"));
                else
                    sprintf(setattr, "%s", set_name(curr_set()));

                gui_label(kd, lvlattr,
                          b ? GUI_MED : GUI_LRG,
                          b ? gui_wht : 0,
                          b ? gui_grn : 0);

                gui_label(kd, setattr, GUI_SML, gui_wht, gui_wht);

                gui_set_rect(kd, GUI_ALL);
            }
            gui_filler(jd);
        }
        gui_space(id);

        // REMEMBER: Put back to the newest values: curr_mode() == MODE_CHALLENGE
        if (curr_mode() == MODE_CHALLENGE /*|| curr_mode() == MODE_HARDCORE*/) 
        {
            gui_label(id, _("Use special powers"), GUI_SML, gui_wht, gui_wht);
            if ((jd = gui_harray(id)))
            {
                int ced, cfd, csd;
                char pow1attr[MAXSTR], pow2attr[MAXSTR], pow3attr[MAXSTR];

                sprintf(pow3attr, _("Speedifier (%i)"), svalue);
                if ((csd = gui_varray(jd)))
                {
                    gui_label(csd, pow3attr, GUI_SML, svalue > 0 ? gui_grn : gui_gry, svalue > 0 ? gui_wht : gui_gry);
                    if (svalue > 0)
                        gui_set_state(csd, START_LEVEL, 3);
                }

                sprintf(pow2attr, _("Floatifier (%i)"), fvalue);
                if ((cfd = gui_varray(jd)))
                {
                    gui_label(cfd, pow2attr, GUI_SML, fvalue > 0 ? gui_blu : gui_gry, fvalue > 0 ? gui_wht : gui_gry);
                    if (fvalue > 0)
                        gui_set_state(cfd, START_LEVEL, 2);
                }

                sprintf(pow1attr, _("Earninator (%i)"), evalue);
                if ((ced = gui_varray(jd)))
                {
                    gui_label(ced, pow1attr, GUI_SML, evalue > 0 ? gui_red : gui_gry, evalue > 0 ? gui_wht : gui_gry);
                    if (evalue > 0)
                        gui_set_state(ced, START_LEVEL, 1);
                }
            }
        }
        else
        {
            gui_multi(id, level_msg(curr_level()), GUI_SML, gui_wht, gui_wht);
        }

        gui_layout(id, 0, 0);
    }

    return id;
}

static int level_enter(struct state *st, struct state *prev)
{
    game_client_fly(1.0f);

    if (check_nodemo && !demo_fp)
    {
        goto_state(&st_nodemo);
        return 0;
    }
    else check_nodemo = 1;

    return level_gui();
}

static void level_timer(int id, float dt)
{
    gui_timer(id, dt);
    game_step_fade(dt);
}

static int level_click(int b, int d)
{
    // REMEMBER: Put back to the newest values: curr_mode() == MODE_NORMAL
    if (b == SDL_BUTTON_LEFT && d == 1)
    {
        if (curr_mode() == MODE_NORMAL)
        {
            return goto_state(&st_play_ready);
        }
    }
    return 1;
}

static int level_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
        {
            progress_stop();
            return goto_state(&st_exit);
        }
        if (c == KEY_POSE)
            return goto_state(&st_poser);
    }
    return 1;
}

static int level_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b) || config_tst_d(CONFIG_JOYSTICK_BUTTON_START, b))
        {
            // REMEMBER: Put back to the newest values: curr_mode() == MODE_NORMAL
            if (curr_mode() == MODE_NORMAL)
            {
                return goto_state(&st_play_ready);
            }
        }
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
        {
            progress_stop();
            return goto_state(&st_exit);
        }
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static void poser_paint(int id, float t)
{
    game_client_draw(POSE_LEVEL, t);
}

static int poser_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT || c == KEY_POSE)
            return goto_state(&st_level);
    }
    return 1;
}

static int poser_buttn(int c, int d)
{
    if (d && config_tst_d(CONFIG_JOYSTICK_BUTTON_B, c))
        return goto_state(&st_level);

    return 1;
}

/*---------------------------------------------------------------------------*/

static int nodemo_gui(void)
{
    int id;

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Warning!"), GUI_MED, 0, 0);
        gui_space(id);
        gui_multi(id, _("A replay file could not be opened for writing.\\"
                        "This game will not be recorded.\\"),
                  GUI_SML, gui_wht, gui_wht);

        gui_layout(id, 0, 0);
    }

    return id;
}

static int nodemo_enter(struct state *st, struct state *prev)
{
    check_nodemo = 0;

    return nodemo_gui();
}

static void nodemo_timer(int id, float dt)
{
    game_step_fade(dt);
    gui_timer(id, dt);
}

static int nodemo_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return goto_state(&st_level);
    }
    return 1;
}

static int nodemo_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_level);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_level);
    }
    return 1;
}


/*---------------------------------------------------------------------------*/

static int exit_enter(struct state *st, struct state *prev)
{
    struct state *dst;

    if (progress_done())
        dst = &st_done;
    else if (curr_mode() == MODE_CHALLENGE || curr_mode() == MODE_HARDCORE)
        dst = &st_over;
    else if (curr_mode() == MODE_STANDALONE)
        dst = NULL;
    else
        dst = &st_start;

    if (dst)
    {
        /* Visit the auxilliary screen or exit to level selection. */

        goto_state(dst != prev ? dst : &st_start);
    }
    else
    {
        /* Quit the game. */

        SDL_Event e = { SDL_QUIT };
        SDL_PushEvent(&e);
    }

    /* HACK: The GUI ID gets lost if you goto_state during a goto_state. */

    return curr_state()->gui_id;
}

/*---------------------------------------------------------------------------*/

struct state st_level = {
    level_enter,
    shared_leave,
    shared_paint,
    level_timer,
    shared_point, // Can hover on: point
    shared_stick, // Can hover on: stick
    shared_angle, // Can hover on: angle
    level_click,
    level_keybd,
    level_buttn
};

struct state st_poser = {
    NULL,
    NULL,
    poser_paint,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    poser_keybd,
    poser_buttn
};

struct state st_nodemo = {
    nodemo_enter,
    shared_leave,
    shared_paint,
    nodemo_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click_basic,
    nodemo_keybd,
    nodemo_buttn
};

struct state st_exit = {
    exit_enter,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
