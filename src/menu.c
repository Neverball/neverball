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

#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gl.h"
#include "text.h"
#include "menu.h"
#include "image.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

struct text
{
    GLuint text;
    GLuint list;

    int x, y;
};

struct item
{
    int state;

    int x0, y0;
    int x1, y1;

    int U, D;
    int L, R;
};

struct menu
{
    struct text *text;
    struct item *item;

    int ntext;
    int nitem;
    int value;
};

/*---------------------------------------------------------------------------*/

static struct menu menu;

/*---------------------------------------------------------------------------*/

void menu_init(int ntext, int nitem, int value)
{
    menu.text = (struct text *) calloc(ntext, sizeof (struct text));
    menu.item = (struct item *) calloc(nitem, sizeof (struct item));

    menu.ntext = (menu.text) ? ntext : 0;
    menu.nitem = (menu.item) ? nitem : 0;

    if (value >= 0) menu.value = value;
}

void menu_free(void)
{
    int i;

    if (menu.text)
    {
        for (i = 0; i < menu.ntext; i++)
        {
            if (glIsList(menu.text[i].list))
                glDeleteLists(menu.text[i].list, 1);
            
            if (glIsTexture(menu.text[i].text))
                glDeleteTextures(1, &menu.text[i].text);
        }
        free(menu.text);
    }

    if (menu.item)
        free(menu.item);
}

void menu_item(int i, int x, int y, int w, int h)
{
    if (menu.item)
    {
        menu.item[i].x0 = x + (config_w() - w) / 2;
        menu.item[i].x1 = x + (config_w() + w) / 2;
        menu.item[i].y0 = y + (config_h() - h) / 2;
        menu.item[i].y1 = y + (config_h() + h) / 2;

        menu.item[i].U = i;
        menu.item[i].D = i;
        menu.item[i].L = i;
        menu.item[i].R = i;

        menu.item[i].state = -1;
    }
}

void menu_link(int i, int U, int D, int L, int R)
{
    if (menu.item)
    {
        menu.item[i].U = U;
        menu.item[i].D = D;
        menu.item[i].L = L;
        menu.item[i].R = R;
    }
}

void menu_stat(int i, int state)
{
    if (menu.item)
        menu.item[i].state = state;
}

void menu_text(int i, int x, int y, const float *c0, const float *c1,
               const char *text, int s)
{
    if (menu.text && text && strlen(text) > 0)
    {
        int w, h;

        text_size(text, s, &w, &h);
        
        menu.text[i].text = make_text(text, s);
        menu.text[i].list = make_list(text, s, c0, c1);

        menu.text[i].x = x + (config_w() - w) / 2;
        menu.text[i].y = y + (config_h() - h) / 2;
    }
}

/*---------------------------------------------------------------------------*/

static void menu_paint_item(struct item *item, int n, int value)
{
    static const GLfloat back[5][4] = {
        { 0.0f, 0.0f, 0.0f, 0.5f },             /* disabled             */
        { 0.1f, 0.1f, 0.1f, 0.5f },             /* normal               */
        { 0.3f, 0.3f, 0.3f, 0.5f },             /* selected             */
        { 0.7f, 0.3f, 0.0f, 0.5f },             /* hilited              */
        { 1.0f, 0.6f, 0.3f, 0.5f },             /* selected and hilited */
    };
    int i, j;

    glDisable(GL_TEXTURE_2D);
    {
        for (i = 0; i < n; i++)
        {
            j = 0;
                
            if (item[i].state >= 0) j  = 1;
            if (i == value)         j  = 2;
            if (item[i].state >  0) j += 2;

            glColor4fv(back[j]);
            glRecti(item[i].x0, item[i].y0, item[i].x1, item[i].y1);
        }
    }
    glEnable(GL_TEXTURE_2D);
}

static void menu_paint_text(struct text *text, int n)
{
    int i;

    for (i = 0; i < n; i++)
    {
        if (glIsTexture(text[i].text))
            glBindTexture(GL_TEXTURE_2D, text[i].text);

        glPushMatrix();
        {
            glTranslated(text[i].x, text[i].y, 0);

            if (glIsList(text[i].list))
                glCallList(text[i].list);
        }
        glPopMatrix();
    }
}

void menu_paint(void)
{
    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    config_push_ortho();
    {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        if (menu.item)
            menu_paint_item(menu.item, menu.nitem, menu.value);

        if (menu.text)
            menu_paint_text(menu.text, menu.ntext);
    }
    config_pop_matrix();
    glPopAttrib();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

void menu_point(int x, int y)
{
    int i;

    for (i = 0; i < menu.nitem; i++)
    {
        struct item *item = menu.item + i;

        if (item->x0 < x && x < item->x1 &&
            item->y0 < y && y < item->y1 && item->state >= 0)
        {
            menu.value = i;
        }
    }
}

void menu_stick(int a, int v)
{
    static int xflag = 0;
    static int yflag = 0;

    if (menu.item)
    {
        if (config_axis_x(a))
        {
            if (v < -JOY_MID && xflag == 0 && menu.item[menu.value].L >= 0)
            {
                menu.value = menu.item[menu.value].L;
                xflag = 1;
            }
            if (v > +JOY_MID && xflag == 0 && menu.item[menu.value].R >= 0)
            {
                menu.value = menu.item[menu.value].R;
                xflag = 1;
            }
            if (-JOY_MID < v && v < +JOY_MID)
                xflag = 0;
        }

        if (config_axis_y(a))
        {
            if (v < -JOY_MID && yflag == 0 && menu.item[menu.value].U >= 0)
            {
                menu.value = menu.item[menu.value].U;
                yflag = 1;
            }
            if (v > +JOY_MID && yflag == 0 && menu.item[menu.value].D >= 0)
            {
                menu.value = menu.item[menu.value].D;
                yflag = 1;
            }
            if (-JOY_MID < v && v < +JOY_MID)
                yflag = 0;
        }
    }
}

int menu_click(void)
{
    if (menu.value >= 0)
        return menu.value;

    return -1;
}

int menu_buttn(void)
{
    if (menu.value >= 0)
        return menu.value;

    return -1;
}

/*---------------------------------------------------------------------------*/
