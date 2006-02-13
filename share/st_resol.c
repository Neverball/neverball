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
#include "back.h"
#include "geom.h"
#include "part.h"
#include "audio.h"
#include "config.h"

#include "st_resol.h"

extern struct state st_conf;
extern struct state st_null;

SDL_Rect ** resolutions;

/*---------------------------------------------------------------------------*/

#define LANG_BACK 100

static int resol_action(int i)
{
    int f = config_get_d(CONFIG_FULLSCREEN);
    int r = 1;
    
    switch (i)
    {
    case LANG_BACK:
        goto_state(&st_conf);
        break;

    default:
	goto_state(&st_null);
	r = config_mode(f, resolutions[i-1]->w, resolutions[i-1]->h);
	goto_state(&st_conf);
        break;
    }

    return r;
}

static int resol_enter(void)
{
    int id, jd, kd;
    int i;
    int w, h;
    int wp, hp;
    int c;

    back_init("back/gui.png", config_get_d(CONFIG_GEOMETRY));

    /* Get the current resolution. */
    w = config_get_d(CONFIG_WIDTH);
    h = config_get_d(CONFIG_HEIGHT);
    
    /* Get the resolution list. */
    resolutions = SDL_ListModes(NULL, SDL_OPENGL | SDL_FULLSCREEN);

    if ((int)resolutions == -1)
    {
	resolutions = NULL;
	printf("Any resolution\n");
    }
    else if (resolutions == NULL)
    {
	printf("No resolution\n");
    }

    if ((id = gui_harray(0)))
    {
        if ((jd = gui_varray(id)))
        {
            if ((kd = gui_harray(jd)))
            {
                gui_label(kd, _("Resolution"), GUI_SML, GUI_ALL, 0, 0);
		gui_filler(kd);
                gui_start(kd, _("Back"),     GUI_SML, LANG_BACK, 0);
            }
	    
	    if (resolutions != NULL)
	    {
		hp = wp = -1;
		c = 0;
		for(i=0; resolutions[i]; i++)
		{
		    if (wp!=resolutions[i]->w || hp!=resolutions[i]->h)
		    {
			static char st[100];
			wp = resolutions[i]->w;
			hp = resolutions[i]->h;
			sprintf(st, "%d x %d", wp, hp);
			if (c % 4 == 0)
				kd = gui_harray(jd);
			gui_state(kd, st,   GUI_SML, i+1, (w==wp) && (h==hp));
			c++;
		    }
		}
		
		for(; c%4!=0; c++)
			gui_filler(kd);
	    }
        }
        gui_layout(id, 0, 0);
    }

    audio_music_fade_to(0.5f, "bgm/inter.ogg");

    return id;
}

static void resol_leave(int id)
{
    back_free();
    gui_delete(id);
}

static void resol_paint(int id, float st)
{
    config_push_persp((float) config_get_d(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
    {
        back_draw(0);
    }
    config_pop_matrix();
    gui_paint(id);
}

static void resol_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static void resol_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void resol_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int resol_click(int b, int d)
{
    if (b < 0 && d == 1)
        return resol_action(gui_token(gui_click()));
    return 1;
}

static int resol_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? goto_state(&st_conf) : 1;
}

static int resol_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return resol_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_conf);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_conf);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/


struct state st_resol = {
    resol_enter,
    resol_leave,
    resol_paint,
    resol_timer,
    resol_point,
    resol_stick,
    resol_click,
    resol_keybd,
    resol_buttn,
    1, 0
};

