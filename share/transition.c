/*
 * Copyright (C) 2024-2025 Jānis Rūcis
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

#include "transition.h"
#include "gui.h"

/*
 * How to create a screen transition:
 *
 * 1. the `leave` handler sets up an "exit" animation (e.g. via `transition_slide`)
 * 2. the `leave` handler DOES NOT use `gui_delete` on its widget ID
 * 3. the `enter` handler of the next screen sets up an "enter" animation (e.g.
 *    via `transition_slide`)
 *
 * You can also set up custom exit animations in the `leave` handler as long as
 * you schedule your widget to be removed at the end with the `GUI_REMOVE` flag.
 *
 * When a screen has an exit animation, the old GUI needs to coexist with the
 * new GUI of the following screen for the duration of the transition. It needs
 * to be painted to be visible and it needs to be stepped forward to animate.
 * That's what all this is for.
 *
 * Only one exit animation is tracked at a time. If a new outgoing widget is
 * registered while one is already animating out (e.g. from rapid page clicks),
 * the previous widget is removed immediately. This prevents widget-pool
 * exhaustion.
 */

/* Widget ID with an exit animation. */
static int out_id;

/*---------------------------------------------------------------------------*/

void transition_init(void)
{
    out_id = 0;
}

void transition_quit(void)
{
    out_id = 0;
}

void transition_add(int id)
{
    if (out_id)
        gui_remove(out_id);

    out_id = id;
}

void transition_remove(int id)
{
    if (out_id == id)
        out_id = 0;
}

void transition_timer(float dt)
{
    if (out_id)
        gui_timer(out_id, dt);
}

void transition_paint(void)
{
    if (out_id)
        gui_paint(out_id);
}

/*---------------------------------------------------------------------------*/

int transition_slide(int id, int in, int intent)
{
    if (in)
    {
        gui_slide(id, (intent == INTENT_BACK ? GUI_W : GUI_E) | GUI_FLING, 0, 0.16f, 0);
    }
    else
    {
        gui_slide(id, (intent == INTENT_BACK ? GUI_E : GUI_W) | GUI_BACKWARD | GUI_FLING | GUI_REMOVE, 0, 0.16f, 0);

        transition_add(id);
    }

    return id;
}

int transition_page(int id, int in, int intent)
{
    const int head_id = gui_child(id, 0);
    const int body_id = gui_child(id, 1);

    if (in)
    {
        // Slide in page content.
        gui_slide(body_id, (intent == INTENT_BACK ? GUI_W : GUI_E) | GUI_FLING, 0, 0.16f, 0);
    }
    else
    {
        // Just hide the header, header from the next page takes over immediately.
        gui_set_hidden(head_id, 1);

        // Remove GUI after timeout (this doesn't do a slide).
        gui_slide(id, GUI_REMOVE, 0, 0.16f, 0);

        // Slide out page content.
        gui_slide(body_id, (intent == INTENT_BACK ? GUI_E : GUI_W) | GUI_BACKWARD | GUI_FLING, 0, 0.16f, 0);

        transition_add(id);
    }

    return id;
}

/*---------------------------------------------------------------------------*/
