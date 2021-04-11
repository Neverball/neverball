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
#include "progress.h"
#include "audio.h"
#include "config.h"
#include "util.h"
#include "common.h"

#include "game_common.h"

#include "st_set.h"
#include "st_title.h"
#include "st_start.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

#define SET_STEP 6

static int total = 0;
static int first = 0;

static int shot_id;
static int desc_id;

static int do_init = 1;

static int *download_ids = NULL;
static int *name_ids = NULL;

enum
{
    SET_SELECT = GUI_LAST
};

struct download_info
{
    char *set_file;
    char label[32];
};

static struct download_info *create_download_info(const char *set_file)
{
    struct download_info *dli = calloc(sizeof (*dli), 1);

    if (dli)
        dli->set_file = strdup(set_file);

    return dli;
}

static void free_download_info(struct download_info *dli)
{
    if (dli)
    {
        if (dli->set_file)
        {
            free(dli->set_file);
            dli->set_file = NULL;
        }

        free(dli);
        dli = NULL;
    }
}

static void download_progress(void *data1, void *data2)
{
    struct download_info *dli = data1;
    struct fetch_progress *pr = data2;

    if (dli)
    {
        /* Sets may change places, so we can't keep set index around. */
        int set_index = set_find(dli->set_file);

        if (download_ids && set_index >= 0 && set_index < total)
        {
            int id = download_ids[set_index];

            if (id)
            {
                char label[32] = GUI_ELLIPSIS;

                if (pr->total > 0)
                    sprintf(label, "%3d%%", (int) (pr->now * 100.0 / pr->total) % 1000);

                /* Compare against current label so we're not calling GL constantly. */
                /* TODO: just do this in gui_set_label. */

                if (strcmp(label, dli->label) != 0)
                {
                    SAFECPY(dli->label, label);
                    gui_set_label(id, label);
                }
            }
        }
    }
}

static void download_done(void *data1, void *data2)
{
    struct download_info *dli = data1;
    struct fetch_done *dn = data2;

    if (dli)
    {
        int set_index = set_find(dli->set_file);

        if (download_ids && set_index >= 0 && set_index < total)
        {
            int id = download_ids[set_index];

            if (id)
            {
                if (dn->finished)
                {
                    gui_remove(id);

                    download_ids[set_index] = 0;

                    if (name_ids && name_ids[set_index])
                    {
                        gui_set_label(name_ids[set_index], set_name(set_index));
                        gui_pulse(name_ids[set_index], 1.2f);
                    }
                }
                else
                {
                    gui_set_label(id, "!");
                    gui_set_color(id, gui_red, gui_red);
                }
            }
        }

        free_download_info(dli);
        dli = NULL;
    }
}

static int set_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
        set_quit();
        return goto_state(&st_title);
        break;

    case GUI_PREV:

        first -= SET_STEP;

        do_init = 0;
        return goto_state(&st_set);

        break;

    case GUI_NEXT:

        first += SET_STEP;

        do_init = 0;
        return goto_state(&st_set);

        break;

    case SET_SELECT:
        if (set_is_installed(val))
        {
            set_goto(val);
            return goto_state(&st_start);
        }
        else if (set_is_downloadable(val))
        {
            struct fetch_callback callback = { 0 };

            callback.progress = download_progress;
            callback.done = download_done;
            callback.data = create_download_info(set_file(val));

            if (!set_download(val, callback))
            {
                free_download_info(callback.data);
                callback.data = NULL;
            }
            else
            {
                if (download_ids && download_ids[val])
                {
                    gui_set_label(download_ids[val], GUI_ELLIPSIS);
                    gui_set_color(download_ids[val], gui_grn, gui_grn);
                }
            }

            return 1;
        }
        else if (set_is_downloading(val))
        {
            return 1;
        }
        break;
    }

    return 1;
}

static void gui_set_download(int id, int i)
{
    int jd;

    if ((jd = gui_hstack(id)))
    {
        /* Create an illusion of center alignment. */
        char *label = concat_string("         ", set_name(i), NULL);

        int button_id, name_id;

        button_id = gui_label(jd, "100%", GUI_SML, gui_grn, gui_grn);

        if (set_is_downloading(i))
            gui_set_label(button_id, GUI_ELLIPSIS);
        else
            gui_set_label(button_id, GUI_ARROW_DN);

        if (download_ids)
            download_ids[i] = button_id;

        name_id = gui_label(jd, "MNOPQRSTUVWXYZ", GUI_SML, gui_wht, gui_wht);

        gui_set_trunc(name_id, TRUNC_TAIL);
        gui_set_label(name_id, label);
        gui_set_fill(name_id);

        if (name_ids)
            name_ids[i] = name_id;

        gui_set_state(jd, SET_SELECT, i);
        gui_set_rect(jd, GUI_ALL);

        free(label);
    }
}

static void gui_set(int id, int i)
{
    if (set_exists(i))
    {
        if (set_is_downloadable(i) || set_is_downloading(i))
        {
            gui_set_download(id, i);
        }
        else
        {
            int name_id;

            if (i % SET_STEP == 0)
                name_id = gui_start(id, "IJKLMNOPQRSTUVWXYZ", GUI_SML, SET_SELECT, i);
            else
                name_id = gui_state(id, "IJKLMNOPQRSTUVWXYZ", GUI_SML, SET_SELECT, i);

            gui_set_trunc(name_id, TRUNC_TAIL);
            gui_set_label(name_id, set_name(i));

            if (name_ids)
                name_ids[i] = name_id;
        }
    }
    else
        gui_label(id, "", GUI_SML, 0, 0);
}

static int set_gui(void)
{
    int w = video.device_w;
    int h = video.device_h;

    int id, jd, kd;

    int i;

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            gui_label(jd, _("Level Set"), GUI_SML, gui_yel, gui_red);
            gui_filler(jd);
            gui_navig(jd, total, first, SET_STEP);
        }

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            shot_id = gui_image(jd, set_shot(first), 7 * w / 16, 7 * h / 16);

            if ((kd = gui_varray(jd)))
            {
                for (i = first; i < first + SET_STEP; i++)
                    gui_set(kd, i);
            }
        }

        gui_space(id);
        desc_id = gui_multi(id, " \\ \\ \\ \\ \\", GUI_SML, gui_yel, gui_wht);

        gui_layout(id, 0, 0);
    }

    return id;
}

static int set_enter(struct state *st, struct state *prev)
{
    if (do_init)
    {
        total = set_init();
        first = MIN(first, (total - 1) - ((total - 1) % SET_STEP));

        audio_music_fade_to(0.5f, "bgm/inter.ogg");
        audio_play(AUD_START, 1.f);
    }
    else do_init = 1;

    if (download_ids)
    {
        free(download_ids);
        download_ids = NULL;
    }

    download_ids = calloc(total, sizeof (*download_ids));

    if (name_ids)
    {
        free(name_ids);
        name_ids = NULL;
    }

    name_ids = calloc(total, sizeof (*name_ids));

    return set_gui();
}

static void set_leave(struct state *st, struct state *next, int id)
{
    gui_delete(id);

    if (download_ids)
    {
        free(download_ids);
        download_ids = NULL;
    }

    if (name_ids)
    {
        free(name_ids);
        name_ids = NULL;
    }
}

static void set_over(int i)
{
    gui_set_image(shot_id, set_shot(i));
    gui_set_multi(desc_id, set_desc(i));
}

static void set_point(int id, int x, int y, int dx, int dy)
{
    int jd = shared_point_basic(id, x, y);

    if (jd && gui_token(jd) == SET_SELECT)
        set_over(gui_value(jd));
}

static void set_stick(int id, int a, float v, int bump)
{
    int jd = shared_stick_basic(id, a, v, bump);

    if (jd && gui_token(jd) == SET_SELECT)
        set_over(gui_value(jd));
}

static int set_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return set_action(GUI_BACK, 0);
    }
    return 1;
}

static int set_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return set_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return set_action(GUI_BACK, 0);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_L1, b) && first > 0)
            return set_action(GUI_PREV, 0);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_R1, b) && first + SET_STEP < total)
            return set_action(GUI_NEXT, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_set = {
    set_enter,
    set_leave,
    shared_paint,
    shared_timer,
    set_point,
    set_stick,
    shared_angle,
    shared_click,
    set_keybd,
    set_buttn
};
