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
#include <SDL_ttf.h>
#include <math.h>

#include "glext.h"
#include "text.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

static int text_state = 0;

const GLfloat c_white[4]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat c_black[4]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat c_red[4]    = { 1.0f, 0.0f, 0.0f, 1.0f };
const GLfloat c_yellow[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
const GLfloat c_green[4]  = { 0.0f, 1.0f, 0.0f, 1.0f };
const GLfloat c_blue[4]   = { 0.0f, 0.0f, 1.0f, 1.0f };
const GLfloat c_grey[4]   = { 0.0f, 0.0f, 0.0f, 0.5f };

const GLfloat c_play[5][4] = {
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f },
};

static TTF_Font *text_font[3] = { NULL, NULL, NULL };
static int       text_drop[3] = { 0, 0, 0 };
static int       radius = 8;

/*---------------------------------------------------------------------------*/

int text_init(int h)
{
    if (text_state == 0)
    {
        if (TTF_Init() == 0)
        {
            radius = h / 56;

            text_font[0] = TTF_OpenFont(TEXT_FACE, h / 24);
            text_font[1] = TTF_OpenFont(TEXT_FACE, h / 12);
            text_font[2] = TTF_OpenFont(TEXT_FACE, h /  6);

            text_drop[0] = 2;
            text_drop[1] = 4;
            text_drop[2] = 8;

            text_state = 1;

            return 1;
        }
        return 0;
    }
    return 1;
}

void text_free(void)
{
    if (text_state == 1)
    {
        if (text_font[2]) TTF_CloseFont(text_font[2]);
        if (text_font[1]) TTF_CloseFont(text_font[1]);
        if (text_font[0]) TTF_CloseFont(text_font[0]);

        TTF_Quit();

        text_state = 0;
    }
}

/*---------------------------------------------------------------------------*/

void text_size(const char *text, int i, int *w, int *h)
{
    if (text_state == 1 && text_font[i])
    {
        TTF_SizeText(text_font[i], text, w, h);

        /*
         * Small hack  here: round  up to the  nearest multiple  of 4.
         * This prevents text-size-based layout from cracking.
         */

        if (w) *w = ((*w + 3) / 4) * 4;
        if (h) *h = ((*h + 3) / 4) * 4;
    }
}

GLuint make_list(const char *text, int i, const float *c0, const float *c1)
{
    GLuint list = glGenLists(1);

    int W, H;
    int w = 0;
    int h = 0;
    int d = text_drop[i];

    GLfloat s0, t0;
    GLfloat s1, t1;

    text_size(text, i, &w, &h);
    image_size(&W, &H, w, h);

    s0 = 0.5f * (W - w) / W;
    t0 = 0.5f * (H - h) / H;
    s1 = 1.0f - s0;
    t1 = 1.0f - t0;

    glNewList(list, GL_COMPILE);
    {
        glBegin(GL_QUADS);
        {
            glColor4f(0.0, 0.0, 0.0, 0.5);
            glTexCoord2f(s0, t1); glVertex2i(0 + d, 0 - d);
            glTexCoord2f(s1, t1); glVertex2i(w + d, 0 - d);
            glTexCoord2f(s1, t0); glVertex2i(w + d, h - d);
            glTexCoord2f(s0, t0); glVertex2i(0 + d, h - d);

            glColor4fv(c0);
            glTexCoord2f(s0, t1); glVertex2i(0, 0);
            glTexCoord2f(s1, t1); glVertex2i(w, 0);

            glColor4fv(c1);
            glTexCoord2f(s1, t0); glVertex2i(w, h);
            glTexCoord2f(s0, t0); glVertex2i(0, h);
        }
        glEnd();
    }
    glEndList();

    return list;
}

GLuint make_text(const char *text, int i)
{
    if (text_state == 1 && text_font[i])
        return make_image_from_font(NULL, NULL, NULL, NULL,
                                    text, text_font[i]);
    else
        return 0;
}

/*---------------------------------------------------------------------------*/

#define PI 3.1415926535897932

/*
 * Draw a rounded  rectangle for text backing and  level shot display.
 * There's some  sub-obtimal trig  here, but it's  all stuffed  into a
 * display list so it's all setup cost.
 */ 

GLuint make_rect(int x0, int y0, int x1, int y1)
{
    double r = (double) radius;
    GLuint list = glGenLists(1);
    int i, n = 8;

    glNewList(list, GL_COMPILE);
    {
        glBegin(GL_QUAD_STRIP);
        {
            for (i = 0; i <= n; i++)
            {
                double a = 0.5 * PI * (double) i / (double) n;

                double x  = x0 + r - r * cos(a);
                double ya = y1 - r + r * sin(a);
                double yb = y0 + r - r * sin(a);

                glTexCoord2d((x - x0) / (x1 - x0), 1 - (ya - y0) / (y1 - y0));
                glVertex2d(x, ya);

                glTexCoord2d((x - x0) / (x1 - x0), 1 - (yb - y0) / (y1 - y0));
                glVertex2d(x, yb);
            }

            for (i = 0; i <= n; i++)
            {
                double a = 0.5 * PI * (double) i / (double) n;

                double x  = x1 - r + r * sin(a);
                double ya = y1 - r + r * cos(a);
                double yb = y0 + r - r * cos(a);

                glTexCoord2d((x - x0) / (x1 - x0), 1 - (ya - y0) / (y1 - y0));
                glVertex2d(x, ya);

                glTexCoord2d((x - x0) / (x1 - x0), 1 - (yb - y0) / (y1 - y0));
                glVertex2d(x, yb);
            }
        }
        glEnd();
    }
    glEndList();

    return list;
}

/*---------------------------------------------------------------------------*/
