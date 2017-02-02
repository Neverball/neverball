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
#include "audio.h"
#include "config.h"
#include "demo.h"

#include "game_server.h"
#include "game_client.h"

#include "st_level.h"
#include "st_play.h"
#include "st_start.h"
#include "st_over.h"
#include "st_done.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

static int check_nodemo = 1;

static int level_gui(void)
{
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

                if (curr_mode() == MODE_CHALLENGE)
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

        gui_multi(id, level_msg(curr_level()), GUI_SML, gui_wht, gui_wht);

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
    game_step_fade(dt);
}

static int level_click(int b, int d)
{
    return (b == SDL_BUTTON_LEFT && d == 1) ? goto_state(&st_play_ready) : 1;
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
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b) ||
            config_tst_d(CONFIG_JOYSTICK_BUTTON_START, b))
        {
            return goto_state(&st_play_ready);
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
    else if (curr_mode() == MODE_CHALLENGE)
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
    NULL,
    NULL,
    NULL,
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
