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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "config.h"
#include "video.h"
#include "glext.h"
#include "image.h"
#include "vec3.h"
#include "gui.h"
#include "common.h"
#include "font.h"
#include "theme.h"

#include "fs.h"

/*---------------------------------------------------------------------------*/

/* Very pure colors for the GUI. I was watching BANZAI when I designed this. */

const GLubyte gui_wht[4] = { 0xFF, 0xFF, 0xFF, 0xFF };  /* White  */
const GLubyte gui_yel[4] = { 0xFF, 0xFF, 0x00, 0xFF };  /* Yellow */
const GLubyte gui_red[4] = { 0xFF, 0x00, 0x00, 0xFF };  /* Red    */
const GLubyte gui_grn[4] = { 0x00, 0xFF, 0x00, 0xFF };  /* Green  */
const GLubyte gui_blu[4] = { 0x00, 0x00, 0xFF, 0xFF };  /* Blue   */
const GLubyte gui_blk[4] = { 0x00, 0x00, 0x00, 0xFF };  /* Black  */
const GLubyte gui_gry[4] = { 0x55, 0x55, 0x55, 0xFF };  /* Gray   */
const GLubyte gui_shd[4] = { 0x00, 0x00, 0x00, 0x80 };  /* Shadow */

/*---------------------------------------------------------------------------*/

#define WIDGET_MAX 256

#define GUI_FREE   0
#define GUI_HARRAY 1
#define GUI_VARRAY 2
#define GUI_HSTACK 3
#define GUI_VSTACK 4
#define GUI_FILLER 5
#define GUI_IMAGE  6
#define GUI_LABEL  7
#define GUI_COUNT  8
#define GUI_CLOCK  9
#define GUI_SPACE  10
#define GUI_BUTTON 11

#define GUI_STATE  1
#define GUI_FILL   2
#define GUI_HILITE 4
#define GUI_RECT   8

#define GUI_LINES 8

/*---------------------------------------------------------------------------*/

struct widget
{
    int     type;
    int     flags;
    int     token;
    int     value;
    int     font;
    int     size;
    int     rect;

    const GLubyte *color0;
    const GLubyte *color1;

    int     x, y;
    int     w, h;
    int     car;
    int     cdr;

    GLuint  image;
    GLfloat scale;

    int     text_w;
    int     text_h;

    enum trunc trunc;
};

/*---------------------------------------------------------------------------*/

/* GUI widget state */

static struct widget widget[WIDGET_MAX];
static int           active;
static int           hovered;
static int           clicked;
static int           padding;
static int           borders[4];

/* Digit widgets for the HUD. */

static int digit_id[3][11];

/* Cursor image. */

static int cursor_id = 0;
static int cursor_st = 0;

/* GUI theme. */

static struct theme curr_theme;

/*---------------------------------------------------------------------------*/

static int gui_hot(int id)
{
    return (widget[id].flags & GUI_STATE);
}

static int gui_size(void)
{
    const int w = video.device_w;
    const int h = video.device_h;

    return MIN(w, h);
}

/*---------------------------------------------------------------------------*/

/* Vertex buffer definitions for widget rendering. */

/* Vertex count */

#define RECT_VERT 16
#define TEXT_VERT  8
#define IMAGE_VERT 4

#define WIDGET_VERT (RECT_VERT + MAX(TEXT_VERT, IMAGE_VERT))

/* Element count */

#define RECT_ELEM 28

#define WIDGET_ELEM (RECT_ELEM)

struct vert
{
    GLubyte c[4];
    GLfloat u[2];
    GLshort p[2];
};

static struct vert vert_buf[WIDGET_MAX * WIDGET_VERT];
static GLuint      vert_vbo = 0;
static GLuint      vert_ebo = 0;

/*---------------------------------------------------------------------------*/

static void set_vert(struct vert *v, int x, int y,
                     GLfloat s, GLfloat t, const GLubyte *c)
{
    v->c[0] = c[0];
    v->c[1] = c[1];
    v->c[2] = c[2];
    v->c[3] = c[3];
    v->u[0] = s;
    v->u[1] = t;
    v->p[0] = x;
    v->p[1] = y;
}

/*---------------------------------------------------------------------------*/

static void draw_enable(GLboolean c, GLboolean u, GLboolean p)
{
    glBindBuffer_(GL_ARRAY_BUFFER,         vert_vbo);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, vert_ebo);

    if (c)
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer   (4, GL_UNSIGNED_BYTE, sizeof (struct vert),
                                  (GLvoid *) offsetof (struct vert, c));
    }
    if (u)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT,         sizeof (struct vert),
                                  (GLvoid *) offsetof (struct vert, u));
    }
    if (p)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer  (2, GL_SHORT,         sizeof (struct vert),
                                  (GLvoid *) offsetof (struct vert, p));
    }
}

static void draw_rect(int id)
{
    glDrawElements(GL_TRIANGLE_STRIP, RECT_ELEM, GL_UNSIGNED_SHORT,
                   (const GLvoid *) (id * WIDGET_ELEM * sizeof (GLushort)));
}

static void draw_text(int id)
{
    glDrawArrays(GL_TRIANGLE_STRIP, id * WIDGET_VERT + RECT_VERT, TEXT_VERT);
}

static void draw_image(int id)
{
    glDrawArrays(GL_TRIANGLE_STRIP, id * WIDGET_VERT + RECT_VERT, IMAGE_VERT);
}

static void draw_disable(void)
{
    glBindBuffer_(GL_ARRAY_BUFFER,         0);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

/*---------------------------------------------------------------------------*/

/*
 * Generate vertices for a 3x3 rectangle. Vertices are arranged
 * top-to-bottom and left-to-right, triangle strips are arranged
 * left-to-right and top-to-bottom (one strip per row). Degenerate
 * triangles (two extra indices per stitch) are inserted for a
 * continous strip.
 */

static const GLushort rect_elem_base[RECT_ELEM] = {
    0, 1, 4, 5, 8, 9, 12, 13, 13, 1,    /* Top    */
    1, 2, 5, 6, 9, 10, 13, 14, 14, 2,   /* Middle */
    2, 3, 6, 7, 10, 11, 14, 15          /* Bottom */
};

static void gui_geom_rect(int id, int x, int y, int w, int h, int f)
{
    GLushort rect_elem[RECT_ELEM];

    struct vert *v = vert_buf + id * WIDGET_VERT;
    struct vert *p = v;

    int X[4];
    int Y[4];

    int i, j;

    /* Generate vertex and element data for the widget's rectangle. */

    X[0] = x;
    X[1] = x +     ((f & GUI_W) ? borders[0] : 0);
    X[2] = x + w - ((f & GUI_E) ? borders[1] : 0);
    X[3] = x + w;

    Y[0] = y + h;
    Y[1] = y + h - ((f & GUI_N) ? borders[2] : 0);
    Y[2] = y +     ((f & GUI_S) ? borders[3] : 0);
    Y[3] = y;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            set_vert(p++, X[i], Y[j], curr_theme.s[i], curr_theme.t[j], gui_wht);

    for (i = 0; i < RECT_ELEM; i++)
        rect_elem[i] = id * WIDGET_VERT + rect_elem_base[i];

    /* Copy this off to the VBOs. */

    glBindBuffer_   (GL_ARRAY_BUFFER, vert_vbo);
    glBufferSubData_(GL_ARRAY_BUFFER,
                     id * WIDGET_VERT * sizeof (struct vert),
                            RECT_VERT * sizeof (struct vert), v);
    glBindBuffer_   (GL_ARRAY_BUFFER, 0);

    glBindBuffer_   (GL_ELEMENT_ARRAY_BUFFER, vert_ebo);
    glBufferSubData_(GL_ELEMENT_ARRAY_BUFFER,
                     id * WIDGET_ELEM * sizeof (GLushort),
                            RECT_ELEM * sizeof (GLushort), rect_elem);
    glBindBuffer_   (GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void gui_geom_text(int id, int x, int y, int w, int h,
                          const GLubyte *c0, const GLubyte *c1)
{
    struct vert *v = vert_buf + id * WIDGET_VERT + RECT_VERT;

    /* Assume the applied texture size is rect size rounded to power-of-two. */

    int W;
    int H;

    image_size(&W, &H, w, h);

    if (w > 0 && h > 0 && W > 0 && H > 0)
    {
        const int d = h / 16;  /* Shadow offset */

        const int ww = ((W - w) % 2) ? w + 1 : w;
        const int hh = ((H - h) % 2) ? h + 1 : h;

        const GLfloat s0 = 0.5f * (W - ww) / W;
        const GLfloat t0 = 0.5f * (H - hh) / H;
        const GLfloat s1 = 1.0f - s0;
        const GLfloat t1 = 1.0f - t0;

        /* Generate vertex data for the colored text and its shadow. */

        set_vert(v + 0, x      + d, y + hh - d, s0, t0, gui_shd);
        set_vert(v + 1, x      + d, y      - d, s0, t1, gui_shd);
        set_vert(v + 2, x + ww + d, y + hh - d, s1, t0, gui_shd);
        set_vert(v + 3, x + ww + d, y      - d, s1, t1, gui_shd);

        set_vert(v + 4, x,          y + hh,     s0, t0, c1);
        set_vert(v + 5, x,          y,          s0, t1, c0);
        set_vert(v + 6, x + ww,     y + hh,     s1, t0, c1);
        set_vert(v + 7, x + ww,     y,          s1, t1, c0);

    }
    else memset(v, 0, TEXT_VERT * sizeof (struct vert));

    /* Copy this off to the VBO. */

    glBindBuffer_   (GL_ARRAY_BUFFER, vert_vbo);
    glBufferSubData_(GL_ARRAY_BUFFER,
                     (id * WIDGET_VERT + RECT_VERT) * sizeof (struct vert),
                                         TEXT_VERT  * sizeof (struct vert), v);
    glBindBuffer_   (GL_ARRAY_BUFFER, 0);
}

static void gui_geom_image(int id, int x, int y, int w, int h, int f)
{
    struct vert *v = vert_buf + id * WIDGET_VERT + RECT_VERT;

    int X[2];
    int Y[2];

    /* Trace inner vertices of the background rectangle. */

    X[0] = x +     ((f & GUI_W) ? borders[0] : 0);
    X[1] = x + w - ((f & GUI_E) ? borders[1] : 0);

    Y[0] = y + h - ((f & GUI_N) ? borders[2] : 0);
    Y[1] = y +     ((f & GUI_S) ? borders[3] : 0);

    set_vert(v + 0, X[0], Y[0], 0.0f, 1.0f, gui_wht);
    set_vert(v + 1, X[0], Y[1], 0.0f, 0.0f, gui_wht);
    set_vert(v + 2, X[1], Y[0], 1.0f, 1.0f, gui_wht);
    set_vert(v + 3, X[1], Y[1], 1.0f, 0.0f, gui_wht);

    /* Copy this off to the VBO. */

    glBindBuffer_   (GL_ARRAY_BUFFER, vert_vbo);
    glBufferSubData_(GL_ARRAY_BUFFER,
                     (id * WIDGET_VERT + RECT_VERT) * sizeof (struct vert),
                     IMAGE_VERT * sizeof (struct vert), v);
    glBindBuffer_   (GL_ARRAY_BUFFER, 0);
}

static void gui_geom_widget(int id, int flags)
{
    int jd;

    int w = widget[id].w;
    int h = widget[id].h;

    int W = widget[id].text_w;
    int H = widget[id].text_h;
    int R = widget[id].rect;

    const GLubyte *c0 = widget[id].color0;
    const GLubyte *c1 = widget[id].color1;

    if ((widget[id].flags & GUI_RECT) && !(flags & GUI_RECT))
    {
        gui_geom_rect(id, -w / 2, -h / 2, w, h, R);
        flags |= GUI_RECT;
    }

    switch (widget[id].type)
    {
    case GUI_FILLER:
    case GUI_SPACE:
        break;

    case GUI_HARRAY:
    case GUI_VARRAY:
    case GUI_HSTACK:
    case GUI_VSTACK:

        for (jd = widget[id].car; jd; jd = widget[jd].cdr)
            gui_geom_widget(jd, flags);

        break;

    case GUI_IMAGE:
        gui_geom_image(id, -w / 2, -h / 2, w, h, R);
        break;

    default:
        gui_geom_text(id, -W / 2, -H / 2, W, H, c0, c1);
        break;
    }
}

/*---------------------------------------------------------------------------*/

#define FONT_MAX 4

static struct font fonts[FONT_MAX];
static int         fontc;

static int font_sizes[3];

static int gui_font_load(const char *path)
{
    int i;

    /* Find a previously loaded font. */

    for (i = 0; i < fontc; i++)
        if (strcmp(fonts[i].path, path) == 0)
            return i;

    /* Load a new font. */

    if (fontc < FONT_MAX)
    {
        if (font_load(&fonts[fontc], path, font_sizes))
        {
            fontc++;
            return fontc - 1;
        }
    }

    /* Return index of default font. */

    return 0;
}

static void gui_font_quit(void);

static void gui_font_init(void)
{
    gui_font_quit();

    if (font_init())
    {
        int s = gui_size();

        font_sizes[0] = s / 26;
        font_sizes[1] = s / 13;
        font_sizes[2] = s /  7;

        /* Load the default font at index 0. */

        gui_font_load(*curr_lang.font ? curr_lang.font : GUI_FACE);
    }
}

static void gui_font_quit(void)
{
    int i;

    for (i = 0; i < fontc; i++)
        font_free(&fonts[i]);

    fontc = 0;

    font_quit();
}

/*---------------------------------------------------------------------------*/

static void gui_theme_quit(void)
{
    theme_free(&curr_theme);
}

static void gui_theme_init(void)
{
    gui_theme_quit();

    theme_load(&curr_theme, config_get_s(CONFIG_THEME));
}

/*---------------------------------------------------------------------------*/

void gui_init(void)
{
    const int s = gui_size();

    int i, j;

    memset(widget, 0, sizeof (struct widget) * WIDGET_MAX);

    /* Compute default widget/text padding. */

    padding = s / 60;

    for (i = 0; i < 4; i++)
        borders[i] = padding;

    /* Initialize font rendering. */

    gui_font_init();

    /* Initialize GUI theme. */

    gui_theme_init();

    /* Initialize the VBOs. */

    memset(vert_buf, 0, sizeof (vert_buf));

    glGenBuffers_(1,              &vert_vbo);
    glBindBuffer_(GL_ARRAY_BUFFER, vert_vbo);
    glBufferData_(GL_ARRAY_BUFFER, sizeof (vert_buf), vert_buf, GL_STATIC_DRAW);
    glBindBuffer_(GL_ARRAY_BUFFER, 0);

    glGenBuffers_(1,                      &vert_ebo);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, vert_ebo);
    glBufferData_(GL_ELEMENT_ARRAY_BUFFER,
                  WIDGET_MAX * WIDGET_ELEM * sizeof (GLushort),
                  NULL, GL_STATIC_DRAW);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* Cache digit glyphs for HUD rendering. */

    for (i = 0; i < 3; i++)
    {
        digit_id[i][ 0] = gui_label(0, "0", i, 0, 0);
        digit_id[i][ 1] = gui_label(0, "1", i, 0, 0);
        digit_id[i][ 2] = gui_label(0, "2", i, 0, 0);
        digit_id[i][ 3] = gui_label(0, "3", i, 0, 0);
        digit_id[i][ 4] = gui_label(0, "4", i, 0, 0);
        digit_id[i][ 5] = gui_label(0, "5", i, 0, 0);
        digit_id[i][ 6] = gui_label(0, "6", i, 0, 0);
        digit_id[i][ 7] = gui_label(0, "7", i, 0, 0);
        digit_id[i][ 8] = gui_label(0, "8", i, 0, 0);
        digit_id[i][ 9] = gui_label(0, "9", i, 0, 0);
        digit_id[i][10] = gui_label(0, ":", i, 0, 0);
    }

    for (i = 0; i < 3; i++)
        for (j = 0; j < 11; ++j)
            gui_layout(digit_id[i][j], 0, 0);

    /* Cache an image for the cursor. Scale it to the same size as a digit. */

    if ((cursor_id = gui_image(0, "gui/cursor.png", widget[digit_id[1][0]].w,
                                                    widget[digit_id[1][0]].h)))
        gui_layout(cursor_id, 0, 0);

    active = 0;
}

void gui_free(void)
{
    int id;

    /* Release the VBOs. */

    glDeleteBuffers_(1, &vert_vbo);
    glDeleteBuffers_(1, &vert_ebo);

    /* Release any remaining widget texture and display list indices. */

    for (id = 1; id < WIDGET_MAX; id++)
    {
        if (widget[id].image)
            glDeleteTextures(1, &widget[id].image);

        widget[id].type  = GUI_FREE;
        widget[id].flags = 0;
        widget[id].image = 0;
        widget[id].cdr   = 0;
        widget[id].car   = 0;
    }

    /* Release all loaded fonts and finalize font rendering. */

    gui_font_quit();

    /* Release theme resources. */

    gui_theme_quit();
}

/*---------------------------------------------------------------------------*/

static int gui_widget(int pd, int type)
{
    int id;

    /* Find an unused entry in the widget table. */

    for (id = 1; id < WIDGET_MAX; id++)
        if (widget[id].type == GUI_FREE)
        {
            /* Set the type and default properties. */

            widget[id].type   = type;
            widget[id].flags  = 0;
            widget[id].token  = 0;
            widget[id].value  = 0;
            widget[id].font   = 0;
            widget[id].size   = 0;
            widget[id].rect   = GUI_ALL;
            widget[id].w      = 0;
            widget[id].h      = 0;
            widget[id].image  = 0;
            widget[id].color0 = gui_wht;
            widget[id].color1 = gui_wht;
            widget[id].scale  = 1.0f;
            widget[id].trunc  = TRUNC_NONE;
            widget[id].text_w = 0;
            widget[id].text_h = 0;

            /* Insert the new widget into the parent's widget list. */

            if (pd)
            {
                widget[id].car = 0;
                widget[id].cdr = widget[pd].car;
                widget[pd].car = id;
            }
            else
            {
                widget[id].car = 0;
                widget[id].cdr = 0;
            }

            return id;
        }

    log_printf("Out of widget IDs\n");

    return 0;
}

int gui_harray(int pd) { return gui_widget(pd, GUI_HARRAY); }
int gui_varray(int pd) { return gui_widget(pd, GUI_VARRAY); }
int gui_hstack(int pd) { return gui_widget(pd, GUI_HSTACK); }
int gui_vstack(int pd) { return gui_widget(pd, GUI_VSTACK); }
int gui_filler(int pd) { return gui_widget(pd, GUI_FILLER); }

/*---------------------------------------------------------------------------*/

static struct size gui_measure_ttf(const char *text, TTF_Font *font)
{
    struct size size = { 0, 0 };

    if (font)
        TTF_SizeUTF8(font, text, &size.w, &size.h);

    return size;
}

struct size gui_measure(const char *text, int size)
{
    return gui_measure_ttf(text, fonts[0].ttf[size]);
}

/*---------------------------------------------------------------------------*/

static char *gui_trunc_head(const char *text,
                            const int maxwidth,
                            TTF_Font *font)
{
    int left, right, mid;
    char *str = NULL;

    left  = 0;
    right = strlen(text);

    while (right - left > 1)
    {
        mid = (left + right) / 2;

        str = concat_string("...", text + mid, NULL);

        if (gui_measure_ttf(str, font).w <= maxwidth)
            right = mid;
        else
            left = mid;

        free(str);
    }

    return concat_string("...", text + right, NULL);
}

static char *gui_trunc_tail(const char *text,
                            const int maxwidth,
                            TTF_Font *font)
{
    int left, right, mid;
    char *str = NULL;

    left  = 0;
    right = strlen(text);

    while (right - left > 1)
    {
        mid = (left + right) / 2;

        str = malloc(mid + sizeof ("..."));

        memcpy(str,       text,  mid);
        memcpy(str + mid, "...", sizeof ("..."));

        if (gui_measure_ttf(str, font).w <= maxwidth)
            left = mid;
        else
            right = mid;

        free(str);
    }

    str = malloc(left + sizeof ("..."));

    memcpy(str,        text,  left);
    memcpy(str + left, "...", sizeof ("..."));

    return str;
}

static char *gui_truncate(const char *text,
                          const int maxwidth,
                          TTF_Font *font,
                          enum trunc trunc)
{
    if (gui_measure_ttf(text, font).w <= maxwidth)
        return strdup(text);

    switch (trunc)
    {
    case TRUNC_NONE: return strdup(text);                         break;
    case TRUNC_HEAD: return gui_trunc_head(text, maxwidth, font); break;
    case TRUNC_TAIL: return gui_trunc_tail(text, maxwidth, font); break;
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

void gui_set_image(int id, const char *file)
{
    glDeleteTextures(1, &widget[id].image);

    widget[id].image = make_image_from_file(file, IF_MIPMAP);
}

void gui_set_label(int id, const char *text)
{
    TTF_Font *ttf = fonts[widget[id].font].ttf[widget[id].size];

    int w = 0;
    int h = 0;

    char *str;

    glDeleteTextures(1, &widget[id].image);

    str = gui_truncate(text, widget[id].w - padding, ttf, widget[id].trunc);

    widget[id].image = make_image_from_font(NULL, NULL,
                                            &widget[id].text_w,
                                            &widget[id].text_h,
                                            str, ttf, 0);
    w = widget[id].text_w;
    h = widget[id].text_h;

    gui_geom_text(id, -w / 2, -h / 2, w, h,
                  widget[id].color0,
                  widget[id].color1);

    free(str);
}

void gui_set_count(int id, int value)
{
    widget[id].value = value;
}

void gui_set_clock(int id, int value)
{
    widget[id].value = value;
}

void gui_set_color(int id, const GLubyte *c0,
                           const GLubyte *c1)
{
    if (id)
    {
        c0 = c0 ? c0 : gui_yel;
        c1 = c1 ? c1 : gui_red;

        if (widget[id].color0 != c0 || widget[id].color1 != c1)
        {
            int w = widget[id].text_w;
            int h = widget[id].text_h;

            widget[id].color0 = c0;
            widget[id].color1 = c1;

            gui_geom_text(id, -w / 2, -h / 2, w, h, c0, c1);
        }
    }
}

void gui_set_multi(int id, const char *text)
{
    const char *p;

    char s[GUI_LINES][MAXSTR];
    int i, sc, lc, jd;

    size_t n = 0;

    /* Count available labels. */

    for (lc = 0, jd = widget[id].car; jd; lc++, jd = widget[jd].cdr);

    /* Copy each delimited string to a line buffer. */

    for (p = text, sc = 0; *p && sc < lc; sc++)
    {
        strncpy(s[sc], p, (n = strcspn(p, "\\")));
        s[sc][n] = 0;

        if (*(p += n) == '\\') p++;
    }

    /* Set the label value for each line. */

    for (i = lc - 1, jd = widget[id].car; i >= 0; i--, jd = widget[jd].cdr)
        gui_set_label(jd, i < sc ? s[i] : "");
}

void gui_set_trunc(int id, enum trunc trunc)
{
    widget[id].trunc = trunc;
}

void gui_set_font(int id, const char *path)
{
    widget[id].font = gui_font_load(path);
}

void gui_set_fill(int id)
{
    widget[id].flags |= GUI_FILL;
}

/*
 * Activate a widget, allowing it  to behave as a normal state widget.
 * This may  be used  to create  image buttons, or  cause an  array of
 * widgets to behave as a single state widget.
 */
int gui_set_state(int id, int token, int value)
{
    widget[id].flags |= GUI_STATE;
    widget[id].token  = token;
    widget[id].value  = value;

    return id;
}

void gui_set_hilite(int id, int hilite)
{
    if (hilite)
        widget[id].flags |= GUI_HILITE;
    else
        widget[id].flags &= ~GUI_HILITE;
}

void gui_set_rect(int id, int rect)
{
    widget[id].rect   = rect;
    widget[id].flags |= GUI_RECT;
}

void gui_set_cursor(int st)
{
    cursor_st = st;
}

/*---------------------------------------------------------------------------*/

int gui_image(int pd, const char *file, int w, int h)
{
    int id;

    if ((id = gui_widget(pd, GUI_IMAGE)))
    {
        widget[id].image  = make_image_from_file(file, IF_MIPMAP);
        widget[id].w      = w;
        widget[id].h      = h;
        widget[id].flags |= GUI_RECT;
    }
    return id;
}

int gui_start(int pd, const char *text, int size, int token, int value)
{
    int id;

    if ((id = gui_state(pd, text, size, token, value)))
        active = id;

    return id;
}

int gui_state(int pd, const char *text, int size, int token, int value)
{
    int id;

    if ((id = gui_widget(pd, GUI_BUTTON)))
    {
        TTF_Font *ttf = fonts[widget[id].font].ttf[size];

        widget[id].flags |= (GUI_STATE | GUI_RECT);

        widget[id].image = make_image_from_font(NULL, NULL,
                                                &widget[id].text_w,
                                                &widget[id].text_h,
                                                text, ttf, 0);
        widget[id].w     = widget[id].text_w;
        widget[id].h     = widget[id].text_h;
        widget[id].size  = size;
        widget[id].token = token;
        widget[id].value = value;
    }
    return id;
}

int gui_label(int pd, const char *text, int size, const GLubyte *c0,
                                                  const GLubyte *c1)
{
    int id;

    if ((id = gui_widget(pd, GUI_LABEL)))
    {
        TTF_Font *ttf = fonts[widget[id].font].ttf[size];

        widget[id].image = make_image_from_font(NULL, NULL,
                                                &widget[id].text_w,
                                                &widget[id].text_h,
                                                text, ttf, 0);
        widget[id].w      = widget[id].text_w;
        widget[id].h      = widget[id].text_h;
        widget[id].size   = size;
        widget[id].color0 = c0 ? c0 : gui_yel;
        widget[id].color1 = c1 ? c1 : gui_red;
        widget[id].flags |= GUI_RECT;
    }
    return id;
}

int gui_count(int pd, int value, int size)
{
    int i, id;

    if ((id = gui_widget(pd, GUI_COUNT)))
    {
        for (i = value; i; i /= 10)
            widget[id].w += widget[digit_id[size][0]].text_w;

        widget[id].h      = widget[digit_id[size][0]].text_h;
        widget[id].value  = value;
        widget[id].size   = size;
        widget[id].color0 = gui_yel;
        widget[id].color1 = gui_red;
        widget[id].flags |= GUI_RECT;
    }
    return id;
}

int gui_clock(int pd, int value, int size)
{
    int id;

    if ((id = gui_widget(pd, GUI_CLOCK)))
    {
        widget[id].w      = widget[digit_id[size][0]].text_w * 6;
        widget[id].h      = widget[digit_id[size][0]].text_h;
        widget[id].value  = value;
        widget[id].size   = size;
        widget[id].color0 = gui_yel;
        widget[id].color1 = gui_red;
        widget[id].flags |= GUI_RECT;
    }
    return id;
}

int gui_space(int pd)
{
    int id;

    if ((id = gui_widget(pd, GUI_SPACE)))
    {
        widget[id].w = 0;
        widget[id].h = 0;
    }
    return id;
}

/*---------------------------------------------------------------------------*/

/*
 * Create  a multi-line  text box  using a  vertical array  of labels.
 * Parse the  text for '\'  characters and treat them  as line-breaks.
 * Preserve the rect specification across the entire array.
 */

int gui_multi(int pd, const char *text, int size, const GLubyte *c0,
                                                  const GLubyte *c1)
{
    int id = 0;

    if (text && *text && (id = gui_varray(pd)))
    {
        const char *p;

        char s[GUI_LINES][MAXSTR];
        int  i, j;

        size_t n = 0;

        /* Copy each delimited string to a line buffer. */

        for (p = text, j = 0; *p && j < GUI_LINES; j++)
        {
            strncpy(s[j], p, (n = strcspn(p, "\\")));
            s[j][n] = 0;

            if (*(p += n) == '\\') p++;
        }

        /* Create a label widget for each line. */

        for (i = 0; i < j; i++)
            gui_label(id, s[i], size, c0, c1);

        /* Set rectangle on the container. */

        widget[id].flags |= GUI_RECT;
    }
    return id;
}

/*---------------------------------------------------------------------------*/
/*
 * The bottom-up pass determines the area of all widgets.  The minimum
 * width  and height of  a leaf  widget is  given by  the size  of its
 * contents.   Array  and  stack   widths  and  heights  are  computed
 * recursively from these.
 */

static void gui_widget_up(int id);

static void gui_harray_up(int id)
{
    int jd, c = 0;

    /* Find the widest child width and the highest child height. */

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        gui_widget_up(jd);

        if (widget[id].h < widget[jd].h)
            widget[id].h = widget[jd].h;
        if (widget[id].w < widget[jd].w)
            widget[id].w = widget[jd].w;

        c++;
    }

    /* Total width is the widest child width times the child count. */

    widget[id].w *= c;
}

static void gui_varray_up(int id)
{
    int jd, c = 0;

    /* Find the widest child width and the highest child height. */

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        gui_widget_up(jd);

        if (widget[id].h < widget[jd].h)
            widget[id].h = widget[jd].h;
        if (widget[id].w < widget[jd].w)
            widget[id].w = widget[jd].w;

        c++;
    }

    /* Total height is the highest child height times the child count. */

    widget[id].h *= c;
}

static void gui_hstack_up(int id)
{
    int jd;

    /* Find the highest child height.  Sum the child widths. */

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        gui_widget_up(jd);

        if (widget[id].h < widget[jd].h)
            widget[id].h = widget[jd].h;

        widget[id].w += widget[jd].w;
    }
}

static void gui_vstack_up(int id)
{
    int jd;

    /* Find the widest child width.  Sum the child heights. */

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        gui_widget_up(jd);

        if (widget[id].w < widget[jd].w)
            widget[id].w = widget[jd].w;

        widget[id].h += widget[jd].h;
    }
}

static void gui_button_up(int id)
{
    if (widget[id].w < widget[id].h && widget[id].w > 0)
        widget[id].w = widget[id].h;

    /* Padded text elements look a little nicer. */

    if (widget[id].w < video.device_w)
        widget[id].w += padding;
    if (widget[id].h < video.device_h)
        widget[id].h += padding;

    /* A button should be at least wide enough to accomodate the borders. */

    if (widget[id].w < borders[0] + borders[1])
        widget[id].w = borders[0] + borders[1];
    if (widget[id].h < borders[2] + borders[3])
        widget[id].h = borders[2] + borders[3];
}

static void gui_widget_up(int id)
{
    if (id)
        switch (widget[id].type)
        {
        case GUI_HARRAY: gui_harray_up(id); break;
        case GUI_VARRAY: gui_varray_up(id); break;
        case GUI_HSTACK: gui_hstack_up(id); break;
        case GUI_VSTACK: gui_vstack_up(id); break;
        case GUI_FILLER:                    break;
        default:         gui_button_up(id); break;
        }
}

/*---------------------------------------------------------------------------*/
/*
 * The  top-down layout  pass distributes  available area  as computed
 * during the bottom-up pass.  Widgets  use their area and position to
 * initialize rendering state.
 */

static void gui_widget_dn(int id, int x, int y, int w, int h);

static void gui_harray_dn(int id, int x, int y, int w, int h)
{
    int jd, i = 0, c = 0;

    widget[id].x = x;
    widget[id].y = y;
    widget[id].w = w;
    widget[id].h = h;

    /* Count children. */

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
        c += 1;

    /* Distribute horizontal space evenly to all children. */

    for (jd = widget[id].car; jd; jd = widget[jd].cdr, i++)
    {
        int x0 = x +  i      * w / c;
        int x1 = x + (i + 1) * w / c;

        gui_widget_dn(jd, x0, y, x1 - x0, h);
    }
}

static void gui_varray_dn(int id, int x, int y, int w, int h)
{
    int jd, i = 0, c = 0;

    widget[id].x = x;
    widget[id].y = y;
    widget[id].w = w;
    widget[id].h = h;

    /* Count children. */

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
        c += 1;

    /* Distribute vertical space evenly to all children. */

    for (jd = widget[id].car; jd; jd = widget[jd].cdr, i++)
    {
        int y0 = y +  i      * h / c;
        int y1 = y + (i + 1) * h / c;

        gui_widget_dn(jd, x, y0, w, y1 - y0);
    }
}

static void gui_hstack_dn(int id, int x, int y, int w, int h)
{
    int jd, jx = x, jw = 0, c = 0;

    widget[id].x = x;
    widget[id].y = y;
    widget[id].w = w;
    widget[id].h = h;

    /* Measure the total width requested by non-filler children. */

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
        if (widget[jd].type == GUI_FILLER)
            c += 1;
        else if (widget[jd].flags & GUI_FILL)
        {
            c  += 1;
            jw += widget[jd].w;
        }
        else
            jw += widget[jd].w;

    /* Give non-filler children their requested space.   */
    /* Distribute the rest evenly among filler children. */

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        if (widget[jd].type == GUI_FILLER)
            gui_widget_dn(jd, jx, y, (w - jw) / c, h);
        else if (widget[jd].flags & GUI_FILL)
            gui_widget_dn(jd, jx, y, widget[jd].w + (w - jw) / c, h);
        else
            gui_widget_dn(jd, jx, y, widget[jd].w, h);

        jx += widget[jd].w;
    }
}

static void gui_vstack_dn(int id, int x, int y, int w, int h)
{
    int jd, jy = y, jh = 0, c = 0;

    widget[id].x = x;
    widget[id].y = y;
    widget[id].w = w;
    widget[id].h = h;

    /* Measure the total height requested by non-filler children. */

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
        if (widget[jd].type == GUI_FILLER)
            c += 1;
        else if (widget[jd].flags & GUI_FILL)
        {
            c  += 1;
            jh += widget[jd].h;
        }
        else
            jh += widget[jd].h;

    /* Give non-filler children their requested space.   */
    /* Distribute the rest evenly among filler children. */

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        if (widget[jd].type == GUI_FILLER)
            gui_widget_dn(jd, x, jy, w, (h - jh) / c);
        else if (widget[jd].flags & GUI_FILL)
            gui_widget_dn(jd, x, jy, w, widget[jd].h + (h - jh) / c);
        else
            gui_widget_dn(jd, x, jy, w, widget[jd].h);

        jy += widget[jd].h;
    }
}

static void gui_filler_dn(int id, int x, int y, int w, int h)
{
    /* Filler expands to whatever size it is given. */

    widget[id].x = x;
    widget[id].y = y;
    widget[id].w = w;
    widget[id].h = h;
}

static void gui_button_dn(int id, int x, int y, int w, int h)
{
    widget[id].x = x;
    widget[id].y = y;
    widget[id].w = w;
    widget[id].h = h;
}

static void gui_widget_dn(int id, int x, int y, int w, int h)
{
    if (id)
        switch (widget[id].type)
        {
        case GUI_HARRAY: gui_harray_dn(id, x, y, w, h); break;
        case GUI_VARRAY: gui_varray_dn(id, x, y, w, h); break;
        case GUI_HSTACK: gui_hstack_dn(id, x, y, w, h); break;
        case GUI_VSTACK: gui_vstack_dn(id, x, y, w, h); break;
        case GUI_FILLER: gui_filler_dn(id, x, y, w, h); break;
        case GUI_SPACE:  gui_filler_dn(id, x, y, w, h); break;
        default:         gui_button_dn(id, x, y, w, h); break;
        }
}

/*---------------------------------------------------------------------------*/
/*
 * During GUI layout, we make a bottom-up pass to determine total area
 * requirements for  the widget  tree.  We position  this area  to the
 * sides or center of the screen.  Finally, we make a top-down pass to
 * distribute this area to each widget.
 */

void gui_layout(int id, int xd, int yd)
{
    int x, y;

    int w, W = video.device_w;
    int h, H = video.device_h;

    gui_widget_up(id);

    w = widget[id].w;
    h = widget[id].h;

    if      (xd < 0) x = 0;
    else if (xd > 0) x = (W - w);
    else             x = (W - w) / 2;

    if      (yd < 0) y = 0;
    else if (yd > 0) y = (H - h);
    else             y = (H - h) / 2;

    gui_widget_dn(id, x, y, w, h);

    /* Set up GUI rendering state. */

    gui_geom_widget(id, 0);

    /* Hilite the widget under the cursor, if any. */

    gui_point(id, -1, -1);
}

int gui_search(int id, int x, int y)
{
    int jd, kd;

    /* Search the hierarchy for the widget containing the given point. */

    if (id && (widget[id].x <= x && x < widget[id].x + widget[id].w &&
               widget[id].y <= y && y < widget[id].y + widget[id].h))
    {
        if (gui_hot(id))
            return id;

        for (jd = widget[id].car; jd; jd = widget[jd].cdr)
            if ((kd = gui_search(jd, x, y)))
                return kd;
    }
    return 0;
}

int gui_delete(int id)
{
    if (id)
    {
        /* Recursively delete all subwidgets. */

        gui_delete(widget[id].cdr);
        gui_delete(widget[id].car);

        /* Release any GL resources held by this widget. */

        if (widget[id].image)
            glDeleteTextures(1, &widget[id].image);

        /* Mark this widget unused. */

        widget[id].type  = GUI_FREE;
        widget[id].flags = 0;
        widget[id].image = 0;
        widget[id].cdr   = 0;
        widget[id].car   = 0;

        /* Clear focus from this widget. */

        if (active == id)
            active = 0;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static void gui_paint_rect(int id, int st, int flags)
{
    int jd, i = 0;

    /* Use the widget status to determine the background color. */

    i = st | (((widget[id].flags & GUI_HILITE) ? 2 : 0) |
              ((id == active)                  ? 1 : 0));

    if ((widget[id].flags & GUI_RECT) && !(flags & GUI_RECT))
    {
        /* Draw a leaf's background, colored by widget state. */

        glPushMatrix();
        {
            glTranslatef((GLfloat) (widget[id].x + widget[id].w / 2),
                         (GLfloat) (widget[id].y + widget[id].h / 2), 0.f);

            glBindTexture(GL_TEXTURE_2D, curr_theme.tex[i]);
            draw_rect(id);
        }
        glPopMatrix();

        flags |= GUI_RECT;
    }

    switch (widget[id].type)
    {
    case GUI_HARRAY:
    case GUI_VARRAY:
    case GUI_HSTACK:
    case GUI_VSTACK:

        /* Recursively paint all subwidgets. */

        for (jd = widget[id].car; jd; jd = widget[jd].cdr)
            gui_paint_rect(jd, i, flags);

        break;
    }
}

/*---------------------------------------------------------------------------*/

static void gui_paint_text(int id);

static void gui_paint_array(int id)
{
    int jd;

    glPushMatrix();
    {
        GLfloat cx = widget[id].x + widget[id].w / 2.0f;
        GLfloat cy = widget[id].y + widget[id].h / 2.0f;
        GLfloat ck = widget[id].scale;

        if (1.0f < ck || ck < 1.0f)
        {
            glTranslatef(+cx, +cy, 0.0f);
            glScalef(ck, ck, ck);
            glTranslatef(-cx, -cy, 0.0f);
        }

        /* Recursively paint all subwidgets. */

        for (jd = widget[id].car; jd; jd = widget[jd].cdr)
            gui_paint_text(jd);
    }
    glPopMatrix();
}

static void gui_paint_image(int id)
{
    /* Draw the widget rect, textured using the image. */

    glPushMatrix();
    {
        glTranslatef((GLfloat) (widget[id].x + widget[id].w / 2),
                     (GLfloat) (widget[id].y + widget[id].h / 2), 0.f);

        glScalef(widget[id].scale,
                 widget[id].scale,
                 widget[id].scale);

        glBindTexture(GL_TEXTURE_2D, widget[id].image);
        glColor4ub(gui_wht[0], gui_wht[1], gui_wht[2], gui_wht[3]);
        draw_image(id);
    }
    glPopMatrix();
}

static void gui_paint_count(int id)
{
    int j, i = widget[id].size;

    glPushMatrix();
    {
        /* Translate to the widget center, and apply the pulse scale. */

        glTranslatef((GLfloat) (widget[id].x + widget[id].w / 2),
                     (GLfloat) (widget[id].y + widget[id].h / 2), 0.f);

        glScalef(widget[id].scale,
                 widget[id].scale,
                 widget[id].scale);

        if (widget[id].value > 0)
        {
            /* Translate right by half the total width of the rendered value. */

            GLfloat w = -widget[digit_id[i][0]].text_w * 0.5f;

            for (j = widget[id].value; j; j /= 10)
                w += widget[digit_id[i][j % 10]].text_w * 0.5f;

            glTranslatef(w, 0.0f, 0.0f);

            /* Render each digit, moving left after each. */

            for (j = widget[id].value; j; j /= 10)
            {
                int jd = digit_id[i][j % 10];

                glBindTexture(GL_TEXTURE_2D, widget[jd].image);
                draw_text(jd);
                glTranslatef((GLfloat) -widget[jd].text_w, 0.0f, 0.0f);
            }
        }
        else if (widget[id].value == 0)
        {
            /* If the value is zero, just display a zero in place. */

            glBindTexture(GL_TEXTURE_2D, widget[digit_id[i][0]].image);
            draw_text(digit_id[i][0]);
        }
    }
    glPopMatrix();
}

static void gui_paint_clock(int id)
{
    int i  =   widget[id].size;
    int mt =  (widget[id].value / 6000) / 10;
    int mo =  (widget[id].value / 6000) % 10;
    int st = ((widget[id].value % 6000) / 100) / 10;
    int so = ((widget[id].value % 6000) / 100) % 10;
    int ht = ((widget[id].value % 6000) % 100) / 10;
    int ho = ((widget[id].value % 6000) % 100) % 10;

    GLfloat dx_large = (GLfloat) widget[digit_id[i][0]].text_w;
    GLfloat dx_small = (GLfloat) widget[digit_id[i][0]].text_w * 0.75f;

    if (widget[id].value < 0)
        return;

    glPushMatrix();
    {
        /* Translate to the widget center, and apply the pulse scale. */

        glTranslatef((GLfloat) (widget[id].x + widget[id].w / 2),
                     (GLfloat) (widget[id].y + widget[id].h / 2), 0.f);

        glScalef(widget[id].scale,
                 widget[id].scale,
                 widget[id].scale);

        /* Translate left by half the total width of the rendered value. */

        if (mt > 0)
            glTranslatef(-2.25f * dx_large, 0.0f, 0.0f);
        else
            glTranslatef(-1.75f * dx_large, 0.0f, 0.0f);

        /* Render the minutes counter. */

        if (mt > 0)
        {
            glBindTexture(GL_TEXTURE_2D, widget[digit_id[i][mt]].image);
            draw_text(digit_id[i][mt]);
            glTranslatef(dx_large, 0.0f, 0.0f);
        }

        glBindTexture(GL_TEXTURE_2D, widget[digit_id[i][mo]].image);
        draw_text(digit_id[i][mo]);
        glTranslatef(dx_small, 0.0f, 0.0f);

        /* Render the colon. */

        glBindTexture(GL_TEXTURE_2D, widget[digit_id[i][10]].image);
        draw_text(digit_id[i][10]);
        glTranslatef(dx_small, 0.0f, 0.0f);

        /* Render the seconds counter. */

        glBindTexture(GL_TEXTURE_2D, widget[digit_id[i][st]].image);
        draw_text(digit_id[i][st]);
        glTranslatef(dx_large, 0.0f, 0.0f);

        glBindTexture(GL_TEXTURE_2D, widget[digit_id[i][so]].image);
        draw_text(digit_id[i][so]);
        glTranslatef(dx_small, 0.0f, 0.0f);

        /* Render hundredths counter half size. */

        glScalef(0.5f, 0.5f, 1.0f);

        glBindTexture(GL_TEXTURE_2D, widget[digit_id[i][ht]].image);
        draw_text(digit_id[i][ht]);
        glTranslatef(dx_large, 0.0f, 0.0f);

        glBindTexture(GL_TEXTURE_2D, widget[digit_id[i][ho]].image);
        draw_text(digit_id[i][ho]);
    }
    glPopMatrix();
}

static void gui_paint_label(int id)
{
    /* Short-circuit empty labels. */

    if (widget[id].image == 0)
        return;

    /* Draw the widget text box, textured using the glyph. */

    glPushMatrix();
    {
        glTranslatef((GLfloat) (widget[id].x + widget[id].w / 2),
                     (GLfloat) (widget[id].y + widget[id].h / 2), 0.f);

        glScalef(widget[id].scale,
                 widget[id].scale,
                 widget[id].scale);

        glBindTexture(GL_TEXTURE_2D, widget[id].image);
        draw_text(id);
    }
    glPopMatrix();
}

static void gui_paint_text(int id)
{
    switch (widget[id].type)
    {
    case GUI_SPACE:  break;
    case GUI_FILLER: break;
    case GUI_HARRAY: gui_paint_array(id); break;
    case GUI_VARRAY: gui_paint_array(id); break;
    case GUI_HSTACK: gui_paint_array(id); break;
    case GUI_VSTACK: gui_paint_array(id); break;
    case GUI_IMAGE:  gui_paint_image(id); break;
    case GUI_COUNT:  gui_paint_count(id); break;
    case GUI_CLOCK:  gui_paint_clock(id); break;
    default:         gui_paint_label(id); break;
    }
}

void gui_paint(int id)
{
    if (id)
    {
        video_push_ortho();
        {
            glDisable(GL_DEPTH_TEST);
            {
                draw_enable(GL_FALSE, GL_TRUE, GL_TRUE);
                gui_paint_rect(id, 0, 0);

                draw_enable(GL_TRUE, GL_TRUE, GL_TRUE);
                gui_paint_text(id);

                if (cursor_st && cursor_id)
                    gui_paint_image(cursor_id);

                draw_disable();
                glColor4ub(gui_wht[0], gui_wht[1], gui_wht[2], gui_wht[3]);
            }
            glEnable(GL_DEPTH_TEST);
        }
        video_pop_matrix();
    }
}

/*---------------------------------------------------------------------------*/

void gui_dump(int id, int d)
{
    int jd, i;

    if (id)
    {
        char *type = "?";

        switch (widget[id].type)
        {
        case GUI_HARRAY: type = "harray"; break;
        case GUI_VARRAY: type = "varray"; break;
        case GUI_HSTACK: type = "hstack"; break;
        case GUI_VSTACK: type = "vstack"; break;
        case GUI_FILLER: type = "filler"; break;
        case GUI_IMAGE:  type = "image";  break;
        case GUI_LABEL:  type = "label";  break;
        case GUI_COUNT:  type = "count";  break;
        case GUI_CLOCK:  type = "clock";  break;
        case GUI_BUTTON: type = "button"; break;
        }

        for (i = 0; i < d; i++)
            printf("    ");

        printf("%04d %s\n", id, type);

        for (jd = widget[id].car; jd; jd = widget[jd].cdr)
            gui_dump(jd, d + 1);
    }
}

void gui_pulse(int id, float k)
{
    if (id) widget[id].scale = k;
}

void gui_timer(int id, float dt)
{
    int jd;

    if (id)
    {
        for (jd = widget[id].car; jd; jd = widget[jd].cdr)
            gui_timer(jd, dt);

        if (widget[id].scale - 1.0f < dt)
            widget[id].scale = 1.0f;
        else
            widget[id].scale -= dt;
    }
}

int gui_point(int id, int x, int y)
{
    static int x_cache = 0;
    static int y_cache = 0;

    int jd;

    /* Reuse the last coordinates if (x,y) == (-1,-1) */

    if (x < 0 && y < 0)
        return gui_point(id, x_cache, y_cache);

    x_cache = x;
    y_cache = y;

    /* Move the cursor, if any. */

    if (cursor_id)
    {
        widget[cursor_id].x = x - widget[cursor_id].w / 2;
        widget[cursor_id].y = y - widget[cursor_id].h / 2;
    }

    /* Short-circuit check the current active widget. */

    jd = gui_search(active, x, y);

    /* If not still active, search the hierarchy for a new active widget. */

    if (jd == 0)
        jd = gui_search(id, x, y);

    /* Note hovered widget. */

    hovered = jd;

    /* If the active widget has changed, return the new active id. */

    if (jd == 0 || jd == active)
        return 0;
    else
        return active = jd;
}

void gui_focus(int i)
{
    active = i;
}

int gui_active(void)
{
    return active;
}

int gui_token(int id)
{
    return id ? widget[id].token : 0;
}

int gui_value(int id)
{
    return id ? widget[id].value : 0;
}

void gui_toggle(int id)
{
    widget[id].flags ^= GUI_HILITE;
}

/*---------------------------------------------------------------------------*/

static int gui_vert_offset(int id, int jd)
{
    /* Vertical offset between bottom of id and top of jd */

    return  widget[id].y - (widget[jd].y + widget[jd].h);
}

static int gui_horz_offset(int id, int jd)
{
    /* Horizontal offset between left of id and right of jd */

    return  widget[id].x - (widget[jd].x + widget[jd].w);
}

static int gui_vert_overlap(int id, int jd)
{
    /* Extent of vertical intersection of id and jd. */

    const int a0 = widget[id].y;
    const int a1 = widget[id].y + widget[id].h;
    const int aw = widget[id].h;
    const int b0 = widget[jd].y;
    const int b1 = widget[jd].y + widget[jd].h;
    /* const int bw = widget[jd].h; */

    return aw + MIN(b1 - a1, 0) - MAX(b0 - a0, 0);
}

static int gui_horz_overlap(int id, int jd)
{
    /* Extent of horizontal intersection of id and jd. */

    const int a0 = widget[id].x;
    const int a1 = widget[id].x + widget[id].w;
    const int aw = widget[id].w;
    const int b0 = widget[jd].x;
    const int b1 = widget[jd].x + widget[jd].w;
    /* const int bw = widget[jd].w; */

    return aw + MIN(b1 - a1, 0) - MAX(b0 - a0, 0);
}

/*---------------------------------------------------------------------------*/

/*
 * Widget navigation heuristics.
 *
 * People generally read left-to-right and top-to-bottom, and have
 * expectations on how navigation should behave. Thus, we hand-craft a
 * bunch of rules rather than devise a generic algorithm.
 *
 * Horizontal navigation only picks overlapping widgets. Closer is
 * better. Out of multiple closest widgets pick the topmost widget.
 *
 * Vertical navigation picks both overlapping and non-overlapping
 * widgets. Closer is better. Out of multiple closest widgets: if
 * overlapping, pick the leftmost widget; if not overlapping, pick the
 * one with the least negative overlap.
 */

/*
 * Leftmost/topmost is decided by the operator used to test
 * distance. A less-than will pick the first of a group of
 * equal-distance widgets, while a less-or-equal will pick the last
 * one.
 */

/* FIXME This isn't how you factor out reusable code. */

#define CHECK_HORIZONTAL \
    (o > 0 && (omin > 0 ? d <= dmin : 1))

#define CHECK_VERTICAL                                                  \
    (omin > 0 ?                                                         \
     d < dmin :                                                         \
     (o > 0 ? d <= dmin : (d < dmin || (d == dmin && o > omin))))

static int gui_stick_L(int id, int dd)
{
    int jd, kd, hd;
    int d, dmin, o, omin;

    /* Find the closest "hot" widget to the left of dd (and inside id) */

    if (id && gui_hot(id))
        return id;

    hd = 0;
    dmin = widget[dd].x - widget[id].x + 1;
    omin = INT_MIN;

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        kd = gui_stick_L(jd, dd);

        if (kd && kd != dd)
        {
            d = gui_horz_offset(dd, kd);
            o = gui_vert_overlap(dd, kd);

            if (d >= 0 && CHECK_HORIZONTAL)
            {
                hd = kd;
                dmin = d;
                omin = o;
            }
        }
    }

    return hd;
}

static int gui_stick_R(int id, int dd)
{
    int jd, kd, hd;
    int d, dmin, o, omin;

    /* Find the closest "hot" widget to the right of dd (and inside id) */

    if (id && gui_hot(id))
        return id;

    hd = 0;
    dmin = (widget[id].x + widget[id].w) - (widget[dd].x + widget[dd].w) + 1;
    omin = INT_MIN;

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        kd = gui_stick_R(jd, dd);

        if (kd && kd != dd)
        {
            d = gui_horz_offset(kd, dd);
            o = gui_vert_overlap(dd, kd);

            if (d >= 0 && CHECK_HORIZONTAL)
            {
                hd = kd;
                dmin = d;
                omin = o;
            }
        }
    }

    return hd;
}

static int gui_stick_D(int id, int dd)
{
    int jd, kd, hd;
    int d, dmin, o, omin;

    /* Find the closest "hot" widget below dd (and inside id) */

    if (id && gui_hot(id))
        return id;

    hd = 0;
    dmin = widget[dd].y - widget[id].y + 1;
    omin = INT_MIN;

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        kd = gui_stick_D(jd, dd);

        if (kd && kd != dd)
        {
            d = gui_vert_offset(dd, kd);
            o = gui_horz_overlap(dd, kd);

            if (d >= 0 && CHECK_VERTICAL)
            {
                hd = kd;
                dmin = d;
                omin = o;
            }
        }
    }

    return hd;
}

static int gui_stick_U(int id, int dd)
{
    int jd, kd, hd;
    int d, dmin, o, omin;

    /* Find the closest "hot" widget above dd (and inside id) */

    if (id && gui_hot(id))
        return id;

    hd = 0;
    dmin = (widget[id].y + widget[id].h) - (widget[dd].y + widget[dd].h) + 1;
    omin = INT_MIN;

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        kd = gui_stick_U(jd, dd);

        if (kd && kd != dd)
        {
            d = gui_vert_offset(kd, dd);
            o = gui_horz_overlap(dd, kd);

            if (d >= 0 && CHECK_VERTICAL)
            {
                hd = kd;
                dmin = d;
                omin = o;
            }
        }
    }

    return hd;
}

/*---------------------------------------------------------------------------*/

static int gui_wrap_L(int id, int dd)
{
    int jd, kd;

    if ((jd = gui_stick_L(id, dd)) == 0)
        for (jd = dd; (kd = gui_stick_R(id, jd)); jd = kd)
            ;

    return jd;
}

static int gui_wrap_R(int id, int dd)
{
    int jd, kd;

    if ((jd = gui_stick_R(id, dd)) == 0)
        for (jd = dd; (kd = gui_stick_L(id, jd)); jd = kd)
            ;

    return jd;
}

static int gui_wrap_U(int id, int dd)
{
    int jd, kd;

    if ((jd = gui_stick_U(id, dd)) == 0)
        for (jd = dd; (kd = gui_stick_D(id, jd)); jd = kd)
            ;

    return jd;
}

static int gui_wrap_D(int id, int dd)
{
    int jd, kd;

    if ((jd = gui_stick_D(id, dd)) == 0)
        for (jd = dd; (kd = gui_stick_U(id, jd)); jd = kd)
            ;

    return jd;
}

/*---------------------------------------------------------------------------*/

int gui_stick(int id, int a, float v, int bump)
{
    int jd = 0;

    if (!bump)
        return 0;

    /* Find a new active widget in the direction of joystick motion. */

    if      (config_tst_d(CONFIG_JOYSTICK_AXIS_X0, a))
    {
        if (v < 0) jd = gui_wrap_L(id, active);
        if (v > 0) jd = gui_wrap_R(id, active);
    }
    else if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y0, a))
    {
        if (v < 0) jd = gui_wrap_U(id, active);
        if (v > 0) jd = gui_wrap_D(id, active);
    }

    /* If the active widget has changed, return the new active id. */

    if (jd == 0 || jd == active)
        return 0;
    else
        return active = jd;
}

int gui_click(int b, int d)
{
    if (b == SDL_BUTTON_LEFT)
    {
        if (d)
        {
            clicked = hovered;
            return 0;
        }
        else
        {
            int c = (clicked && clicked == hovered);
            clicked = 0;
            return c;
        }
    }
    return 0;
}

int gui_click_right(int b, int d)
{
    if (b == SDL_BUTTON_RIGHT && d)
    {
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

int gui_navig(int id, int total, int first, int step)
{
    int pages = (int) ceil((double) total / step);
    int page = first / step + 1;

    int prev = (page > 1);
    int next = (page < pages);

    int jd, kd;

    if ((jd = gui_hstack(id)))
    {
        if (next || prev)
        {
            gui_maybe(jd, " > ", GUI_NEXT, GUI_NONE, next);

            if ((kd = gui_label(jd, "999/999", GUI_SML, gui_wht, gui_wht)))
            {
                char str[16];
                sprintf(str, "%d/%d", page, pages);
                gui_set_label(kd, str);
            }

            gui_maybe(jd, " < ", GUI_PREV, GUI_NONE, prev);
        }

        gui_space(jd);

        gui_start(jd, _("Back"), GUI_SML, GUI_BACK, 0);
    }
    return jd;
}

int gui_maybe(int id, const char *label, int etoken, int dtoken, int enabled)
{
    int bd;

    if (!enabled)
    {
        bd = gui_state(id, label, GUI_SML, dtoken, 0);
        gui_set_color(bd, gui_gry, gui_gry);
    }
    else
        bd = gui_state(id, label, GUI_SML, etoken, 0);

    return bd;
}

/*---------------------------------------------------------------------------*/
