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
#include "hud.h"
#include "set.h"
#include "demo.h"
#include "progress.h"
#include "audio.h"
#include "solid.h"
#include "config.h"
#include "st_shared.h"
#include "util.h"
#include "common.h"
#include "demo_dir.h"

#include "game_common.h"
#include "game_server.h"
#include "game_client.h"

#include "st_demo.h"
#include "st_title.h"

/*---------------------------------------------------------------------------*/

#define DEMO_LINE 4
#define DEMO_STEP 8

static Array items;

static int first = 0;
static int total = 0;
static int last  = 0;

static int last_viewed = 0;

/*---------------------------------------------------------------------------*/

static int demo_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case GUI_BACK:
        return goto_state(&st_title);

    case GUI_NEXT:
        first += DEMO_STEP;
        return goto_state(&st_demo);
        break;

    case GUI_PREV:
        first -= DEMO_STEP;
        return goto_state(&st_demo);
        break;

    case GUI_NULL:
        return 1;
        break;

    default:
        if (progress_replay(DIR_ITEM_GET(items, i)->path))
        {
            last_viewed = i;
            demo_play_goto(0);
            return goto_state(&st_demo_play);
        }
        break;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static struct thumb
{
    int item;
    int shot;
    int name;
} thumbs[DEMO_STEP];

static int gui_demo_thumbs(int id)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);

    int jd, kd, ld;
    int i, j;

    struct thumb *thumb;

    if ((jd = gui_varray(id)))
        for (i = first; i < first + DEMO_STEP; i += DEMO_LINE)
            if ((kd = gui_harray(jd)))
            {
                for (j = i + DEMO_LINE - 1; j >= i; j--)
                {
                    thumb = &thumbs[j % DEMO_STEP];

                    thumb->item = j;

                    if (j < total)
                    {
                        if ((ld = gui_vstack(kd)))
                        {
                            gui_space(ld);

                            thumb->shot = gui_image(ld, " ", w / 6, h / 6);
                            thumb->name = gui_state(ld, " ", GUI_SML, j, 0);

                            gui_set_trunc(thumb->name, TRUNC_TAIL);

                            gui_active(ld, j, 0);
                        }
                    }
                    else
                    {
                        gui_space(kd);

                        thumb->shot = 0;
                        thumb->name = 0;
                    }
                }
            }

    return jd;
}

static void gui_demo_update_thumbs(void)
{
    struct dir_item *item;
    struct demo *demo;
    int i;

    for (i = 0; i < ARRAYSIZE(thumbs) && thumbs[i].shot && thumbs[i].name; i++)
    {
        item = DIR_ITEM_GET(items, thumbs[i].item);
        demo = item->data;

        gui_set_image(thumbs[i].shot, demo ? demo->shot : "");
        gui_set_label(thumbs[i].name, demo ? demo->name : base_name(item->path));
    }
}

static int name_id;
static int time_id;
static int coin_id;
static int date_id;
static int status_id;
static int player_id;

static int gui_demo_status(int id)
{
    const char *status;
    int jd, kd, ld;
    int s;

    /* Find the longest status string. */

    for (status = "", s = GAME_NONE; s < GAME_MAX; s++)
        if (strlen(status_to_str(s)) > strlen(status))
            status = status_to_str(s);

    /* Build info bar with dummy values. */

    if ((jd = gui_hstack(id)))
    {
        gui_filler(jd);

        if ((kd = gui_hstack(jd)))
        {
            if ((ld = gui_vstack(kd)))
            {
                gui_filler(ld);

                time_id   = gui_clock(ld, 35000,  GUI_SML, GUI_NE);
                coin_id   = gui_count(ld, 100,    GUI_SML, 0);
                status_id = gui_label(ld, status, GUI_SML, GUI_SE,
                                      gui_red, gui_red);

                gui_filler(ld);
            }

            if ((ld = gui_vstack(kd)))
            {
                gui_filler(ld);

                gui_label(ld, _("Time"),   GUI_SML, GUI_NW, gui_wht, gui_wht);
                gui_label(ld, _("Coins"),  GUI_SML, 0,      gui_wht, gui_wht);
                gui_label(ld, _("Status"), GUI_SML, GUI_SW, gui_wht, gui_wht);

                gui_filler(ld);
            }
        }

        gui_space(jd);

        if ((kd = gui_vstack(jd)))
        {
            gui_filler(kd);

            name_id   = gui_label(kd, " ", GUI_SML, GUI_NE, 0, 0);
            player_id = gui_label(kd, " ", GUI_SML, 0,      0, 0);
            date_id   = gui_label(kd, date_to_str(time(NULL)),
                                  GUI_SML, GUI_SE, 0, 0);

            gui_filler(kd);

            gui_set_trunc(name_id,   TRUNC_TAIL);
            gui_set_trunc(player_id, TRUNC_TAIL);
        }

        if ((kd = gui_vstack(jd)))
        {
            gui_filler(kd);

            gui_label(kd, _("Replay"), GUI_SML, GUI_NW, gui_wht, gui_wht);
            gui_label(kd, _("Player"), GUI_SML, 0,      gui_wht, gui_wht);
            gui_label(kd, _("Date"),   GUI_SML, GUI_SW, gui_wht, gui_wht);

            gui_filler(kd);
        }

        gui_filler(jd);
    }

    return jd;
}

static void gui_demo_update_status(int i)
{
    const struct demo *d;

    if (!total)
        return;

    d = DEMO_GET(items, i < total ? i : 0);

    if (!d)
        return;

    gui_set_label(name_id,   d->name);
    gui_set_label(date_id,   date_to_str(d->date));
    gui_set_label(player_id, d->player);

    if (d->status == GAME_GOAL)
        gui_set_color(status_id, gui_grn, gui_grn);
    else
        gui_set_color(status_id, gui_red, gui_red);

    gui_set_label(status_id, status_to_str(d->status));
    gui_set_count(coin_id, d->coins);
    gui_set_clock(time_id, d->timer);
}

/*---------------------------------------------------------------------------*/

static int demo_gui(void)
{
    int id, jd;

    id = gui_vstack(0);

    if (total)
    {
        if ((jd = gui_hstack(id)))
        {

            gui_label(jd, _("Select Replay"), GUI_SML, GUI_ALL, 0,0);
            gui_filler(jd);
            gui_navig(jd, first > 0, first + DEMO_STEP < total);
        }

        gui_demo_thumbs(id);
        gui_space(id);
        gui_demo_status(id);

        gui_layout(id, 0, 0);

        gui_demo_update_thumbs();
        gui_demo_update_status(last_viewed);
    }
    else
    {
        gui_label(id, _("No Replays"), GUI_MED, GUI_ALL, 0, 0);
        gui_layout(id, 0, 0);
    }

    return id;
}

static int demo_enter(struct state *st, struct state *prev)
{
    if (!items || (prev == &st_demo_del))
    {
        if (items)
        {
            demo_dir_free(items);
            items = NULL;
        }

        items = demo_dir_scan();
        total = array_len(items);
    }

    first       = first < total ? first : 0;
    last        = MIN(first + DEMO_STEP - 1, total - 1);
    last_viewed = MIN(MAX(first, last_viewed), last);

    if (total)
        demo_dir_load(items, first, last);

    audio_music_fade_to(0.5f, "bgm/inter.ogg");

    return demo_gui();
}

static void demo_leave(struct state *st, struct state *next, int id)
{
    if (next == &st_title)
    {
        demo_dir_free(items);
        items = NULL;
    }

    gui_delete(id);
}

static void demo_timer(int id, float dt)
{
    if (total == 0 && time_state() > 4.0f)
        goto_state(&st_title);

    gui_timer(id, dt);
}

static void demo_point(int id, int x, int y, int dx, int dy)
{
    int jd = shared_point_basic(id, x, y);
    int i  = gui_token(jd);

    if (jd && i >= 0 && !GUI_ISMSK(i))
        gui_demo_update_status(i);
}

static void demo_stick(int id, int a, int v)
{
    int jd = shared_stick_basic(id, a, v);
    int i  = gui_token(jd);

    if (jd && i >= 0 && !GUI_ISMSK(i))
        gui_demo_update_status(i);
}

static int demo_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return demo_action(total ? gui_token(gui_click()) : GUI_BACK);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return demo_action(GUI_BACK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int standalone;
static int demo_paused;
static int show_hud;
static int check_compat;

static float prelude;

void demo_play_goto(int s)
{
    standalone   = s;
    check_compat = 1;
}

static int demo_play_gui(void)
{
    int id;

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Replay"), GUI_LRG, GUI_ALL, gui_blu, gui_grn);
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    return id;
}

static int demo_play_enter(struct state *st, struct state *prev)
{
    if (demo_paused)
    {
        demo_paused = 0;
        prelude = 0;
        audio_music_fade_in(0.5f);
        return 0;
    }

    /*
     * Post-1.5.1 replays include view data in the first update, this
     * line is currently left in for compatibility with older replays.
     */
    game_client_fly(0.0f);

    if (check_compat && !game_compat_map)
    {
        goto_state(&st_demo_compat);
        return 0;
    }

    prelude = 1.0f;

    show_hud = 1;
    hud_update(0);

    return demo_play_gui();
}

static void demo_play_paint(int id, float t)
{
    game_client_draw(0, t);

    if (show_hud)
        hud_paint();

    if (time_state() < prelude)
        gui_paint(id);
}

static void demo_play_timer(int id, float dt)
{
    game_step_fade(dt);
    gui_timer(id, dt);
    hud_timer(dt);

    /* Pause briefly before starting playback. */

    if (time_state() < prelude)
        return;

    if (!demo_replay_step(dt))
    {
        demo_paused = 0;
        goto_state(&st_demo_end);
    }
    else
        progress_step();
}

static int demo_play_keybd(int c, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_KEY_PAUSE, c))
        {
            demo_paused = 1;
            return goto_state(&st_demo_end);
        }

        if (c == SDLK_F6)
            show_hud = !show_hud;
    }
    return 1;
}

static int demo_play_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
        {
            if (config_tst_d(CONFIG_KEY_PAUSE, SDLK_ESCAPE))
                demo_paused = 1;

            return goto_state(&st_demo_end);
        }
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

#define DEMO_KEEP      0
#define DEMO_DEL       1
#define DEMO_QUIT      2
#define DEMO_REPLAY    3
#define DEMO_CONTINUE  4

static int demo_end_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case DEMO_DEL:
        demo_paused = 0;
        return goto_state(&st_demo_del);
    case DEMO_KEEP:
        demo_paused = 0;
        demo_replay_stop(0);
        return goto_state(&st_demo);
    case DEMO_QUIT:
        demo_replay_stop(0);
        return 0;
    case DEMO_REPLAY:
        demo_replay_stop(0);
        progress_replay(curr_demo());
        return goto_state(&st_demo_play);
    case DEMO_CONTINUE:
        return goto_state(&st_demo_play);
    }
    return 1;
}

static int demo_end_gui(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        if (demo_paused)
            kd = gui_label(id, _("Replay Paused"), GUI_LRG, GUI_ALL,
                           gui_gry, gui_red);
        else
            kd = gui_label(id, _("Replay Ends"),   GUI_LRG, GUI_ALL,
                           gui_gry, gui_red);

        if ((jd = gui_harray(id)))
        {
            int start_id = 0;

            if (standalone)
            {
                start_id = gui_start(jd, _("Quit"), GUI_SML, DEMO_QUIT, 1);
            }
            else
            {
                start_id = gui_start(jd, _("Keep"), GUI_SML, DEMO_KEEP, 1);
                gui_state(jd, _("Delete"), GUI_SML, DEMO_DEL, 0);
            }

            if (demo_paused)
            {
                gui_start(jd, _("Continue"), GUI_SML, DEMO_CONTINUE, 1);
                gui_toggle(start_id);
            }
            else
                gui_state(jd, _("Repeat"),   GUI_SML, DEMO_REPLAY,   0);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }

    return id;
}

static int demo_end_enter(struct state *st, struct state *prev)
{
    audio_music_fade_out(demo_paused ? 0.2f : 2.0f);

    return demo_end_gui();
}

static void demo_end_paint(int id, float t)
{
    game_client_draw(0, t);
    gui_paint(id);

    if (demo_paused)
        hud_paint();
}

static int demo_end_keybd(int c, int d)
{
    if (d)
    {
        if (demo_paused && config_tst_d(CONFIG_KEY_PAUSE, c))
            return demo_end_action(DEMO_CONTINUE);
    }
    return 1;
}

static int demo_end_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return demo_end_action(gui_token(gui_click()));

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
        {
            if (demo_paused)
                return demo_end_action(DEMO_CONTINUE);
            else
                return demo_end_action(standalone ? DEMO_QUIT : DEMO_KEEP);
        }
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int demo_del_action(int i)
{
    audio_play(AUD_MENU, 1.0f);
    demo_replay_stop(i == DEMO_DEL);
    return goto_state(&st_demo);
}

static int demo_del_gui(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, _("Delete Replay?"), GUI_MED, GUI_ALL, gui_red, gui_red);

        if ((jd = gui_harray(id)))
        {
            gui_start(jd, _("No"),  GUI_SML, DEMO_KEEP, 1);
            gui_state(jd, _("Yes"), GUI_SML, DEMO_DEL,  0);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }

    return id;
}

static int demo_del_enter(struct state *st, struct state *prev)
{
    audio_music_fade_out(2.0f);

    return demo_del_gui();
}

static int demo_del_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return demo_del_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return demo_del_action(DEMO_KEEP);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int demo_compat_gui(void)
{
    int id;

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Warning!"), GUI_MED, GUI_ALL, 0, 0);
        gui_space(id);
        gui_multi(id, _("The current replay was recorded with a\\"
                        "different (or unknown) version of this level.\\"
                        "Be prepared to encounter visual errors.\\"),
                  GUI_SML, GUI_ALL, gui_wht, gui_wht);

        gui_layout(id, 0, 0);
    }

    return id;
}

static int demo_compat_enter(struct state *st, struct state *prev)
{
    check_compat = 0;

    return demo_compat_gui();
}

static void demo_compat_timer(int id, float dt)
{
    game_step_fade(dt);
    gui_timer(id, dt);
}

static int demo_compat_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_demo_play);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_demo_end);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_demo = {
    demo_enter,
    demo_leave,
    shared_paint,
    demo_timer,
    demo_point,
    demo_stick,
    shared_angle,
    shared_click,
    NULL,
    demo_buttn,
    1, 0
};

struct state st_demo_play = {
    demo_play_enter,
    shared_leave,
    demo_play_paint,
    demo_play_timer,
    NULL,
    NULL,
    NULL,
    NULL,
    demo_play_keybd,
    demo_play_buttn,
    1, 0
};

struct state st_demo_end = {
    demo_end_enter,
    shared_leave,
    demo_end_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    demo_end_keybd,
    demo_end_buttn,
    1, 0
};

struct state st_demo_del = {
    demo_del_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    NULL,
    demo_del_buttn,
    1, 0
};

struct state st_demo_compat = {
    demo_compat_enter,
    shared_leave,
    shared_paint,
    demo_compat_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    NULL,
    demo_compat_buttn,
    1, 0
};
