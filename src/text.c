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

#include "gl.h"
#include "text.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

const GLfloat c_white[4]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat c_black[4]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat c_red[4]    = { 1.0f, 0.0f, 0.0f, 1.0f };
const GLfloat c_yellow[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
const GLfloat c_green[4]  = { 0.0f, 1.0f, 0.0f, 1.0f };
const GLfloat c_blue[4]   = { 0.0f, 0.0f, 1.0f, 1.0f };
const GLfloat c_grey[4]   = { 0.0f, 0.0f, 0.0f, 0.5f };

static TTF_Font *text_font[3] = { NULL, NULL, NULL };
static int       text_drop[3] = { 0, 0, 0 };

/*---------------------------------------------------------------------------*/

int text_init(int h)
{
    if (TTF_Init() == 0)
    {
        text_font[0] = TTF_OpenFont(TEXT_FACE, h / 24);
        text_font[1] = TTF_OpenFont(TEXT_FACE, h / 12);
        text_font[2] = TTF_OpenFont(TEXT_FACE, h /  6);

        text_drop[0] = 2;
        text_drop[1] = 4;
        text_drop[2] = 8;

        return 1;
    }
    return 0;
}

void text_free(void)
{
    if (TTF_WasInit())
    {
        if (text_font[2]) TTF_CloseFont(text_font[2]);
        if (text_font[1]) TTF_CloseFont(text_font[1]);
        if (text_font[0]) TTF_CloseFont(text_font[0]);

        TTF_Quit();
    }
}

/*---------------------------------------------------------------------------*/

void text_size(const char *text, int i, int *w, int *h)
{
    TTF_SizeText(text_font[i], text, w, h);
}

GLuint make_list(const char *text, int i, const float *c0, const float *c1)
{
    GLuint list = glGenLists(1);

    int W, H;
    int w, h, d = text_drop[i];

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
    return make_image_from_font(NULL, NULL, NULL, NULL, text, text_font[i]);
}

/*---------------------------------------------------------------------------*/

