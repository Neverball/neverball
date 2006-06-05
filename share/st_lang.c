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

#include "gui.h"
#include "back.h"
#include "geom.h"
#include "part.h"
#include "audio.h"
#include "config.h"

#include "st_lang.h"
#include "i18n.h"

extern struct state st_conf;
extern struct state st_null;

static char **lang_names;

/*---------------------------------------------------------------------------*/

#define LANG_BACK 100

static int lang_action(int i)
{
    int r = 1;

    switch (i)
    {
    case LANG_BACK:
        goto_state(&st_conf);
        break;

    default:
        goto_state(&st_null);
        language_set(i - 1);
        config_set_s(CONFIG_LANG, language_get_code(i - 1));
        gui_init();

        goto_state(&st_conf);
        break;
    }

    return r;
}

static int lang_enter(void)
{
    int id, jd, kd;
    int i;
    int l;
    char *ln, *ln1, *ln2;

    back_init("back/gui.png", config_get_d(CONFIG_GEOMETRY));

    /* Initialize the language selection GUI. */
    l = language_from_code(config_simple_get_s(CONFIG_LANG));

    if ((id = gui_harray(0)))
    {
        if ((jd = gui_varray(id)))
        {
            if ((kd = gui_harray(jd)))
            {
                gui_label(kd, _("Language"), GUI_SML, GUI_ALL, 0, 0);
                gui_filler(kd);
                gui_start(kd, _("Back"), GUI_SML, LANG_BACK, 0);
            }

            lang_names = calloc(language_count(), sizeof(char *));

            gui_state(jd, _(language_get_name(0)), GUI_SML, 1, (l == 0));
            for (i = 1; i <= language_count(); i++)
            {
                language_set(i);
                ln1 = _(language_get_name(i));
                language_set(l);
                ln2 = _(language_get_name(i));
                if (strcmp(ln1, ln2) == 0)
                    ln = ln1;
                else
                {
                    ln = malloc(sizeof(char) * (strlen(ln1) + strlen(ln2) + 4));
                    lang_names[i - 1] = ln;
                    strcpy(ln, ln1);
                    strcat(ln, " (");
                    strcat(ln, ln2);
                    strcat(ln, ")");
                }

                gui_state(jd, ln, GUI_SML, i + 1, (l == i));
            }
        }
        gui_layout(id, 0, 0);
    }

    audio_music_fade_to(0.5f, "bgm/inter.ogg");

    return id;
}

static void lang_leave(int id)
{
    int i;

    for (i = 0; i < language_count(); i++)
        if (lang_names[i] != NULL)
            free(lang_names[i]);
    free(lang_names);
    back_free();
    gui_delete(id);
}

static void lang_paint(int id, float st)
{
    config_push_persp((float) config_get_d(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
    {
        back_draw(0);
    }
    config_pop_matrix();
    gui_paint(id);
}

static void lang_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static void lang_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void lang_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int lang_click(int b, int d)
{
    if (b < 0 && d == 1)
        return lang_action(gui_token(gui_click()));
    return 1;
}

static int lang_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_conf) : 1;
}

static int lang_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return lang_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_conf);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_conf);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/


struct state st_lang = {
    lang_enter,
    lang_leave,
    lang_paint,
    lang_timer,
    lang_point,
    lang_stick,
    lang_click,
    lang_keybd,
    lang_buttn,
    1, 0
};

