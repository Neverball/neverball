/*   
 *   Copyright (C) 2003 Robert Kooima
 *
 *   SUPER EMPTY BALL is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by the
 *   Free Software Foundation;  either version 2 of the  License, or (at your
 *   option) any later version.
 *
 *   This program  is distributed  in the  hope that it  will be  useful, but
 *   WITHOUT   ANY   WARRANTY;  without   even   the   implied  warranty   of
 *   MERCHANTABILITY  or  FITNESS FOR  A  PARTICULAR  PURPOSE.   See the  GNU
 *   General Public License for more details.
 */

#include "gl.h"
#include "main.h"
#include "menu.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

void menu_init(struct menu *menu, const char *file,
               struct item *item, int num, int val)
{
    image_load(&menu->image, file);
    menu->w = image_w(&menu->image) * image_scale;
    menu->h = image_h(&menu->image) * image_scale;

    menu->x = (main_width  - menu->w) / 2;
    menu->y = (main_height - menu->h) / 2;

    menu->item = item;
    menu->num  = num;
    menu->lit  = -1;
}

void menu_free(struct menu *menu)
{
    image_free(&menu->image);
}

/*---------------------------------------------------------------------------*/

static void push_ortho(void)
{
    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, main_width, 0.0, main_height, 0.0, 1.0);
    }
    glMatrixMode(GL_MODELVIEW);
}

static void pop_ortho(void)
{
    glMatrixMode(GL_PROJECTION);
    {
        glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------------*/

void menu_paint(struct menu *menu, double t)
{
    int i;
    int x0, x1;
    int y0, y1;

    push_ortho();
    {
        glPushAttrib(GL_ENABLE_BIT);
        {
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_COLOR_MATERIAL);

            x0 = menu->x;
            y0 = menu->y;
            x1 = menu->x + menu->w;
            y1 = menu->y + menu->h;

            glColor4d(1.0, 1.0, 1.0, t);

            image_bind(&menu->image);

            glBegin(GL_QUADS);
            {
                glTexCoord2i(0, 1); glVertex2i(x0, y0);
                glTexCoord2i(1, 1); glVertex2i(x1, y0);
                glTexCoord2i(1, 0); glVertex2i(x1, y1);
                glTexCoord2i(0, 0); glVertex2i(x0, y1);
            }
            glEnd();

            glDisable(GL_TEXTURE_2D);

            glBegin(GL_QUADS);
            {
                for (i = 0; i < menu->num; i++)
                {
                    double r = 1.0;
                    double g = 1.0;
                    double b = 1.0;
                    double a = 0.2;

                    x0 = menu->x + menu->item[i].x;
                    y0 = menu->y + menu->item[i].y;
                    x1 = menu->x + menu->item[i].x + menu->item[i].w;
                    y1 = menu->y + menu->item[i].y + menu->item[i].h;

                    if (menu->item[i].lit == -1) r = g = b = a = 0.4;
                    if (menu->item[i].lit == +1) a = 0.4;
                    if (menu->lit         ==  i) a = 0.5;

                    glColor4d(r, g, b, a);
                    glVertex2i(x0, y0);
                    glVertex2i(x1, y0);
                    glVertex2i(x1, y1);
                    glVertex2i(x0, y1);
                }
            }
            glEnd();
        }
        glPopAttrib();
    }
    pop_ortho();
}

void menu_point(struct menu *menu, int x, int y)
{
    int i;

    menu->lit = -1;

    for (i = 0; i < menu->num; i++)
    {
        struct item *item = menu->item + i;

        if (menu->x + item->x < x && x < menu->x + item->x + item->w &&
            menu->y + item->y < y && y < menu->y + item->y + item->h &&
            item->lit >= 0)
            menu->lit = i;
    }
}

int  menu_click(struct menu *menu, int d)
{
    if (d == 0 && menu->lit >= 0)
    {
        menu->item[menu->lit].lit = 1;
        return menu->item[menu->lit].val;
    }
    return -1;
}

/*---------------------------------------------------------------------------*/
