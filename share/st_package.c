/*
 * Copyright (C) 2023 Neverball authors
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
#include "audio.h"
#include "config.h"
#include "util.h"
#include "common.h"
#include "package.h"
#include "geom.h"

#include "st_package.h"
#include "st_common.h"

#define AUD_MENU "snd/menu.ogg"

/*---------------------------------------------------------------------------*/

#define PACKAGE_STEP 5

static int total = 0;
static int first = 0;

static int shot_id;
static int desc_id;
static int type_id;

static int do_init = 1;

static struct state *package_back;

static int *download_ids = NULL;
static int *name_ids = NULL;

enum
{
    PACKAGE_SELECT = GUI_LAST
};

struct download_info
{
    char *package_id;
    char label[32];
};

static struct download_info *create_download_info(const char *package_id)
{
    struct download_info *dli = calloc(sizeof (*dli), 1);

    if (dli)
        dli->package_id = strdup(package_id);

    return dli;
}

static void free_download_info(struct download_info *dli)
{
    if (dli)
    {
        if (dli->package_id)
        {
            free(dli->package_id);
            dli->package_id = NULL;
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
        int pi = package_search_id(dli->package_id);

        if (download_ids && pi >= 0 && pi < total)
        {
            int id = download_ids[pi];

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
        int pi = package_search_id(dli->package_id);

        if (download_ids && pi >= 0 && pi < total)
        {
            int id = download_ids[pi];

            if (id)
            {
                if (dn->finished)
                {
                    gui_remove(id);

                    download_ids[pi] = 0;

                    if (name_ids && name_ids[pi])
                    {
                        gui_set_label(name_ids[pi], package_get_name(pi));
                        gui_pulse(name_ids[pi], 1.2f);
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

static int package_action(int tok, int val)
{
    enum package_status status;

    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
        return goto_state(package_back);
        break;

    case GUI_PREV:

        first -= PACKAGE_STEP;

        do_init = 0;
        return goto_state(&st_package);

        break;

    case GUI_NEXT:

        first += PACKAGE_STEP;

        do_init = 0;
        return goto_state(&st_package);

        break;

    case PACKAGE_SELECT:
        status = package_get_status(val);

        if (status == PACKAGE_INSTALLED)
        {
            // TODO:
            return 1;
        }
        else if (status == PACKAGE_AVAILABLE || status == PACKAGE_PARTIAL || status == PACKAGE_ERROR)
        {
            struct fetch_callback callback = { 0 };

            callback.progress = download_progress;
            callback.done = download_done;
            callback.data = create_download_info(package_get_id(val));

            if (!package_fetch(val, callback))
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
        else if (status == PACKAGE_DOWNLOADING)
        {
            return 1;
        }
        break;
    }

    return 1;
}

static void gui_package_download(int id, int pi)
{
    int jd;

    if ((jd = gui_hstack(id)))
    {
        /* Create an illusion of center alignment. */
        char *label = concat_string("         ", package_get_name(pi), NULL);

        int button_id, name_id;

        button_id = gui_label(jd, "100%", GUI_SML, gui_grn, gui_grn);

        if (package_get_status(pi) == PACKAGE_DOWNLOADING)
            gui_set_label(button_id, GUI_ELLIPSIS);
        else
            gui_set_label(button_id, GUI_ARROW_DN);

        if (download_ids)
            download_ids[pi] = button_id;

        name_id = gui_label(jd, "MNOPQRSTUVWXYZ", GUI_SML, gui_wht, gui_wht);

        gui_set_trunc(name_id, TRUNC_TAIL);
        gui_set_label(name_id, label);
        gui_set_fill(name_id);

        if (name_ids)
            name_ids[pi] = name_id;

        gui_set_state(jd, PACKAGE_SELECT, pi);
        gui_set_rect(jd, GUI_ALL);

        free(label);
    }
}

static void gui_package(int id, int pi)
{
    if (pi >= 0 && pi < package_count())
        if (package_get_status(pi) == PACKAGE_INSTALLED)
        {
            int name_id;

            if (pi % PACKAGE_STEP == 0)
                name_id = gui_start(id, "IJKLMNOPQRSTUVWXYZ", GUI_SML, PACKAGE_SELECT, pi);
            else
                name_id = gui_state(id, "IJKLMNOPQRSTUVWXYZ", GUI_SML, PACKAGE_SELECT, pi);

            gui_set_trunc(name_id, TRUNC_TAIL);
            gui_set_label(name_id, package_get_name(pi));

            if (name_ids)
                name_ids[pi] = name_id;
        }
        else
        {
            gui_package_download(id, pi);
        }
    else
        gui_label(id, "", GUI_SML, 0, 0);
}

static int package_gui(void)
{
    int w = video.device_w;
    int h = video.device_h;

    int id, jd, kd;

    int i;

    if (total < 0)
    {
        if ((id = gui_vstack(0)))
        {
            gui_label(id, _("No packages available"), GUI_SML, 0, 0);
            gui_space(id);
            gui_state(id, _("Back"), GUI_SML, GUI_BACK, 0);
            gui_layout(id, 0, 0);
        }

        return id;
    }

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            gui_label(jd, _("Packages"), GUI_SML, gui_yel, gui_red);
            gui_filler(jd);
            gui_navig(jd, total, first, PACKAGE_STEP);
        }

        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            if ((kd = gui_varray(jd)))
            {
                for (i = first; i < first + PACKAGE_STEP; i++)
                    gui_package(kd, i);
            }


            if ((kd = gui_vstack(jd)))
            {
                shot_id = gui_image(kd, package_get_shot_filename(first), 7 * w / 16, 7 * h / 16);
                type_id = gui_label(kd, "ABCDEF", GUI_SML, gui_yel, gui_wht);
            }

        }

        gui_space(id);

        desc_id = gui_multi(id, " \\ \\ \\ \\ \\", GUI_SML, gui_yel, gui_wht);

        gui_layout(id, 0, 0);

        gui_set_label(type_id, package_get_formatted_type(first));
    }

    return id;
}

static int package_enter(struct state *st, struct state *prev)
{
    common_init(package_action);

    back_init("back/gui.png");

    if (do_init)
    {
        package_back = prev;

        total = -1;//package_count(); // TODO: handle -1
        first = MIN(first, (total - 1) - ((total - 1) % PACKAGE_STEP));

        audio_music_fade_to(0.5f, "bgm/inter.ogg");
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

    return package_gui();
}

static void package_leave(struct state *st, struct state *next, int id)
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

void package_paint(int id, float st)
{
    video_push_persp((float) config_get_d(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
    {
        back_draw_easy();
    }
    video_pop_matrix();

    gui_paint(id);
}

static void package_hover(int pi)
{
    gui_set_image(shot_id, package_get_shot_filename(pi));
    gui_set_multi(desc_id, package_get_desc(pi));
    gui_set_label(type_id, package_get_formatted_type(pi));
}

static void package_point(int id, int x, int y, int dx, int dy)
{
    int jd = gui_point(id, x, y);

    if (jd)
        gui_pulse(jd, 1.2f);

    if (gui_token(jd) == PACKAGE_SELECT)
        package_hover(gui_value(jd));
}

static void package_stick(int id, int a, float v, int bump)
{
    int jd = gui_stick(id, a, v, bump);

    if (id)
        gui_pulse(jd, 1.2f);

    if (gui_token(jd) == PACKAGE_SELECT)
        package_hover(gui_value(jd));
}

/*---------------------------------------------------------------------------*/

struct state st_package = {
    package_enter,
    package_leave,
    package_paint,
    common_timer,
    package_point,
    package_stick,
    NULL,
    common_click,
    common_keybd,
    common_buttn
};
