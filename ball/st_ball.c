/*
 * Copyright (C) 2003-2010 Neverball authors
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
#include "state.h"
#include "array.h"
#include "dir.h"
#include "config.h"
#include "fs.h"
#include "common.h"
#include "ball.h"
#include "cmd.h"
#include "audio.h"
#include "geom.h"
#include "video.h"
#include "demo.h"

#include "game_server.h"
#include "game_proxy.h"
#include "game_client.h"
#include "game_common.h"

#include "st_ball.h"
#include "st_conf.h"
#include "st_shared.h"

static Array balls;
static int   curr_ball;
static char  ball_file[64];

#define PAGE_ROWS 4
#define PAGE_COLS 3
#define PAGE_COUNT (PAGE_ROWS * PAGE_COLS)  /* Maximum balls per page */

static int first = 0;

enum
{
    BALL_SELECT = GUI_LAST
};

static int highlight_btn;

static int has_ball_sols(struct dir_item *item)
{
    char *solid, *inner, *outer;
    int yes;

    solid = concat_string(item->path,
                          "/",
                          base_name(item->path),
                          "-solid.sol",
                          NULL);
    inner = concat_string(item->path,
                          "/",
                          base_name(item->path),
                          "-inner.sol",
                          NULL);
    outer = concat_string(item->path,
                          "/",
                          base_name(item->path),
                          "-outer.sol",
                          NULL);

    yes = (fs_exists(solid) || fs_exists(inner) || fs_exists(outer));

    free(solid);
    free(inner);
    free(outer);

    return yes;
}

static int cmp_dir_items(const void *A, const void *B)
{
    const struct dir_item *a = A, *b = B;
    return strcmp(a->path, b->path);
}

static void scan_balls(void)
{
    int i;

    SAFECPY(ball_file, config_get_s(CONFIG_BALL_FILE));

    if ((balls = fs_dir_scan("ball", has_ball_sols)))
    {
        array_sort(balls, cmp_dir_items);

        for (i = 0; i < array_len(balls); i++)
        {
            const char *path = DIR_ITEM_GET(balls, i)->path;

            if (strncmp(ball_file, path, strlen(path)) == 0)
            {
                curr_ball = i;
                break;
            }
        }
    }
}

static void free_balls(void)
{
    fs_dir_free(balls);
    balls = NULL;
}

static void set_curr_ball(void)
{
    sprintf(ball_file, "%s/%s",
            DIR_ITEM_GET(balls, curr_ball)->path,
            base_name(DIR_ITEM_GET(balls, curr_ball)->path));

    config_set_s(CONFIG_BALL_FILE, ball_file);

    ball_free();
    ball_init();
}

static int ball_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_PREV:
        first -= PAGE_COUNT;
        return goto_state(&st_ball);

    case GUI_NEXT:
        first += PAGE_COUNT;
        return goto_state(&st_ball);

    case BALL_SELECT:
        if (highlight_btn)
            gui_set_hilite(highlight_btn, 0);
        highlight_btn = gui_active();
        gui_set_hilite(highlight_btn, 1);

        curr_ball = val;
        set_curr_ball();
        break;

    case GUI_BACK:
        goto_state(&st_conf);
        break;
    }

    return 1;
}

static void load_ball_demo(void)
{
    int g;

    /* "g" is a stupid hack to keep the goal locked. */

    if (!demo_replay_init("gui/ball.nbr", &g, NULL, NULL, NULL, NULL))
    {
        ball_action(GUI_BACK, 0);
        return;
    }

    audio_music_fade_to(0.0f, "bgm/inter.ogg");
    game_client_fly(0);
    game_kill_fade();
    back_init("back/gui.png");
}

static void ball_item_gui(int id, int i)
{
    if (i >= array_len(balls))
    {
        gui_label(id, " ", GUI_SML, 0, 0);
    }
    else
    {
        const char *name = base_name(DIR_ITEM_GET(balls, i)->path);

        if (i == curr_ball)
        {
            highlight_btn = gui_start(id, name, GUI_SML, BALL_SELECT, i);
            gui_set_hilite(highlight_btn, 1);
        }
        else
        {
            gui_state(id, name, GUI_SML, BALL_SELECT, i);
        }
    }
}

static void ball_list_page(int id)
{
    int jd;
    int i, j;

    for (i = 0; i < PAGE_ROWS; i++)
    {
        if ((jd = gui_harray(id)))
        {
            for (j = PAGE_COLS - 1; j >= 0; j--)
                ball_item_gui(jd, first + i * PAGE_COLS + j);
        }
    }
}

static int ball_gui(void)
{
    int id, jd, i;

    highlight_btn = 0;

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            gui_label(jd, _("Ball Model"), GUI_SML, 0, 0);
            gui_filler(jd);

            /* Expand the UI to prevent it from changing size when navigating pages */
            for (i = 0; i < 10; i++)
                gui_space(jd);

            gui_navig(jd, array_len(balls), first, PAGE_COUNT);
        }

        gui_space(id);

        ball_list_page(id);

        for (i = 0; i < 13; i++)
            gui_space(id);

        gui_layout(id, 0, 0);
    }

    return id;
}

static int ball_enter(struct state *st, struct state *prev)
{
    scan_balls();
    load_ball_demo();

    return ball_gui();
}

static void ball_leave(struct state *st, struct state *next, int id)
{
    gui_delete(id);
    back_free();
    demo_replay_stop(0);
    free_balls();
}

static void ball_paint(int id, float t)
{
    video_push_persp((float) config_get_d(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
    {
        back_draw_easy();
    }
    video_pop_matrix();

    game_client_draw(POSE_BALL, t);
    gui_paint(id);
}

static void ball_timer(int id, float dt)
{
    gui_timer(id, dt);

    if (!demo_replay_step(dt))
    {
        demo_replay_stop(0);
        load_ball_demo();
    }

    game_client_blend(demo_replay_blend());
}

static int ball_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return ball_action(GUI_BACK, 0);
    }
    return 1;
}

static int ball_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return ball_action(gui_token(active), gui_value(active));

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return ball_action(GUI_BACK, 0);
    }
    return 1;
}

struct state st_ball = {
    ball_enter,
    ball_leave,
    ball_paint,
    ball_timer,
    shared_point,
    shared_stick,
    NULL,
    shared_click,
    ball_keybd,
    ball_buttn
};
