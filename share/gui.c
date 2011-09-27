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

#include "config.h"
#include "video.h"
#include "glext.h"
#include "image.h"
#include "vec3.h"
#include "gui.h"
#include "common.h"

#include "fs.h"
#include "fs_rwops.h"

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

#define GUI_STATE 1
#define GUI_FILL  2

#define GUI_LINES 8

/*---------------------------------------------------------------------------*/

struct widget
{
    int     type;
    int     flags;
    int     token;
    int     value;
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
static int           sticky;
static int           radius;
static TTF_Font     *font[3] = { NULL, NULL, NULL };

/* Digit widgets for the HUD. */

static int digit_id[3][11];

/* Font data access. */

static void      *fontdata;
static int        fontdatalen;
static SDL_RWops *fontrwops;

/*---------------------------------------------------------------------------*/

static int gui_hot(int id)
{
    return (widget[id].flags & GUI_STATE);
}

/*---------------------------------------------------------------------------*/

/* Vertex buffer definitions for widget rendering. */

#define RECT_VERT 36
#define TEXT_VERT 8
#define WIDGET_VERT (RECT_VERT + TEXT_VERT)

struct vert
{
    GLubyte c[4];
    GLfloat u[2];
    GLshort p[2];
};

static struct vert vert_buf[WIDGET_MAX * WIDGET_VERT];
static GLuint      vert_obj = 0;

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
    glBindBuffer_(GL_ARRAY_BUFFER, vert_obj);

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
    glDrawArrays(GL_TRIANGLE_STRIP, id * WIDGET_VERT,  RECT_VERT);
}

static void draw_text(int id)
{
    glDrawArrays(GL_TRIANGLE_STRIP, id * WIDGET_VERT + RECT_VERT, TEXT_VERT);
}

static void draw_disable(void)
{
    glBindBuffer_(GL_ARRAY_BUFFER, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

/*---------------------------------------------------------------------------*/

static void gui_rect(int id, int x, int y, int w, int h, int f, int r)
{
    struct vert *v = vert_buf + id * WIDGET_VERT;
    struct vert *p = v;

    /* Generate vertex data for the widget's rounded rectangle. */

    int n = 8;
    int i;

    /* Left side... */

    for (i = 0; i <= n; i++)
    {
        float a = 0.5f * V_PI * (float) i / (float) n;
        float s = r * fsinf(a);
        float c = r * fcosf(a);

        float X  = x     + r - c;
        float Ya = y + h + ((f & GUI_NW) ? (s - r) : 0);
        float Yb = y     + ((f & GUI_SW) ? (r - s) : 0);

        set_vert(p++, X, Ya, (X - x) / w, (Ya - y) / h, gui_wht);
        set_vert(p++, X, Yb, (X - x) / w, (Yb - y) / h, gui_wht);
    }

    /* Right side... */

    for (i = 0; i <= n; i++)
    {
        float a = 0.5f * V_PI * (float) i / (float) n;
        float s = r * fsinf(a);
        float c = r * fcosf(a);

        float X  = x + w - r + s;
        float Ya = y + h + ((f & GUI_NE) ? (c - r) : 0);
        float Yb = y     + ((f & GUI_SE) ? (r - c) : 0);

        set_vert(p++, X, Ya, (X - x) / w, (Ya - y) / h, gui_wht);
        set_vert(p++, X, Yb, (X - x) / w, (Yb - y) / h, gui_wht);
    }

    /* Copy this off to the VBO. */

    glBindBuffer_   (GL_ARRAY_BUFFER, vert_obj);
    glBufferSubData_(GL_ARRAY_BUFFER,
                     id * WIDGET_VERT * sizeof (struct vert),
                            RECT_VERT * sizeof (struct vert), v);
}

static void gui_text(int id, int x, int y,
                             int w, int h, const GLubyte *c0, const GLubyte *c1)
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

    glBindBuffer_   (GL_ARRAY_BUFFER, vert_obj);
    glBufferSubData_(GL_ARRAY_BUFFER,
                     (id * WIDGET_VERT + RECT_VERT) * sizeof (struct vert),
                                         TEXT_VERT  * sizeof (struct vert), v);
}

/*---------------------------------------------------------------------------*/

static const char *pick_font_path(void)
{
    const char *path;

    path = _(GUI_FACE);

    if (!fs_exists(path))
    {
        fprintf(stderr, L_("Font '%s' doesn't exist, trying default font.\n"),
                path);

        path = GUI_FACE;
    }

    return path;
}

void gui_init(void)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);
    int s = (h < w) ? h : w;
    int i, j;

    /* Initialize font rendering. */

    if (TTF_Init() == 0)
    {
        const char *fontpath = pick_font_path();

        int s0 = s / 26;
        int s1 = s / 13;
        int s2 = s /  7;

        memset(widget, 0, sizeof (struct widget) * WIDGET_MAX);

        /* Load the font. */

        if ((fontdata = fs_load(fontpath, &fontdatalen)))
        {
            fontrwops = SDL_RWFromConstMem(fontdata, fontdatalen);

            /* Load small, medium, and large typefaces. */

            font[GUI_SML] = TTF_OpenFontRW(fontrwops, 0, s0);

            SDL_RWseek(fontrwops, 0, SEEK_SET);
            font[GUI_MED] = TTF_OpenFontRW(fontrwops, 0, s1);

            SDL_RWseek(fontrwops, 0, SEEK_SET);
            font[GUI_LRG] = TTF_OpenFontRW(fontrwops, 0, s2);

            /* fontrwops remains open. */
        }
        else
        {
            fontrwops = NULL;

            font[GUI_SML] = NULL;
            font[GUI_MED] = NULL;
            font[GUI_LRG] = NULL;

            fprintf(stderr, L_("Could not load font '%s'.\n"), fontpath);
        }

        radius = s / 60;
    }

    /* Initialize the VBOs. */

    memset(vert_buf, 0, sizeof (vert_buf));

    glGenBuffers_(1,              &vert_obj);
    glBindBuffer_(GL_ARRAY_BUFFER, vert_obj);
    glBufferData_(GL_ARRAY_BUFFER, sizeof (vert_buf), vert_buf, GL_STATIC_DRAW);
    glBindBuffer_(GL_ARRAY_BUFFER, 0);

    /* Cache digit glyphs for HUD rendering. */

    for (i = 0; i < 3; i++)
    {
        digit_id[i][ 0] = gui_label(0, "0", i, 0, 0, 0);
        digit_id[i][ 1] = gui_label(0, "1", i, 0, 0, 0);
        digit_id[i][ 2] = gui_label(0, "2", i, 0, 0, 0);
        digit_id[i][ 3] = gui_label(0, "3", i, 0, 0, 0);
        digit_id[i][ 4] = gui_label(0, "4", i, 0, 0, 0);
        digit_id[i][ 5] = gui_label(0, "5", i, 0, 0, 0);
        digit_id[i][ 6] = gui_label(0, "6", i, 0, 0, 0);
        digit_id[i][ 7] = gui_label(0, "7", i, 0, 0, 0);
        digit_id[i][ 8] = gui_label(0, "8", i, 0, 0, 0);
        digit_id[i][ 9] = gui_label(0, "9", i, 0, 0, 0);
        digit_id[i][10] = gui_label(0, ":", i, 0, 0, 0);
    }

    for (i = 0; i < 3; i++)
        for (j = 0; j < 11; ++j)
            gui_layout(digit_id[i][j], 0, 0);

    active = 0;
}

void gui_free(void)
{
    int id;

    /* Release the VBOs. */

    glDeleteBuffers_(1, &vert_obj);

    /* Release any remaining widget texture and display list indices. */

    for (id = 1; id < WIDGET_MAX; id++)
    {
        glDeleteTextures(1, &widget[id].image);

        widget[id].type  = GUI_FREE;
        widget[id].flags = 0;
        widget[id].image = 0;
        widget[id].cdr   = 0;
        widget[id].car   = 0;
    }

    /* Release all loaded fonts and finalize font rendering. */

    if (font[GUI_LRG]) TTF_CloseFont(font[GUI_LRG]);
    if (font[GUI_MED]) TTF_CloseFont(font[GUI_MED]);
    if (font[GUI_SML]) TTF_CloseFont(font[GUI_SML]);

    if (fontrwops) SDL_RWclose(fontrwops);
    if (fontdata)  free(fontdata);

    TTF_Quit();
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
            widget[id].size   = 0;
            widget[id].rect   = GUI_NW | GUI_SW | GUI_NE | GUI_SE;
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

    fprintf(stderr, "Out of widget IDs\n");

    return 0;
}

int gui_harray(int pd) { return gui_widget(pd, GUI_HARRAY); }
int gui_varray(int pd) { return gui_widget(pd, GUI_VARRAY); }
int gui_hstack(int pd) { return gui_widget(pd, GUI_HSTACK); }
int gui_vstack(int pd) { return gui_widget(pd, GUI_VSTACK); }
int gui_filler(int pd) { return gui_widget(pd, GUI_FILLER); }

/*---------------------------------------------------------------------------*/

struct size
{
    int w, h;
};

static struct size gui_measure(const char *text, TTF_Font *font)
{
    struct size size = { 0, 0 };

    if (font)
        TTF_SizeUTF8(font, text, &size.w, &size.h);

    return size;
}

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

        if (gui_measure(str, font).w <= maxwidth)
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

        if (gui_measure(str, font).w <= maxwidth)
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
    if (gui_measure(text, font).w <= maxwidth)
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

    widget[id].image = make_image_from_file(file);
}

void gui_set_label(int id, const char *text)
{
    int w = 0;
    int h = 0;

    char *str;

    glDeleteTextures(1, &widget[id].image);

    str = gui_truncate(text, widget[id].w - radius,
                       font[widget[id].size],
                       widget[id].trunc);

    widget[id].image = make_image_from_font(NULL, NULL, &w, &h,
                                            str, font[widget[id].size]);
    widget[id].text_w = w;
    widget[id].text_h = h;

    gui_text(id, -w / 2, -h / 2, w, h, widget[id].color0, widget[id].color1);

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

            gui_text(id, -w / 2, -h / 2, w, h, c0, c1);
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

/*---------------------------------------------------------------------------*/

int gui_image(int pd, const char *file, int w, int h)
{
    int id;

    if ((id = gui_widget(pd, GUI_IMAGE)))
    {
        widget[id].image = make_image_from_file(file);
        widget[id].w = w;
        widget[id].h = h;
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
        widget[id].flags |= GUI_STATE;

        widget[id].image = make_image_from_font(NULL, NULL,
                                                   &widget[id].w,
                                                   &widget[id].h,
                                                   text, font[size]);
        widget[id].size  = size;
        widget[id].token = token;
        widget[id].value = value;
    }
    return id;
}

int gui_label(int pd, const char *text, int size, int rect, const GLubyte *c0,
                                                            const GLubyte *c1)
{
    int id;

    if ((id = gui_widget(pd, GUI_LABEL)))
    {
        widget[id].image = make_image_from_font(NULL, NULL,
                                                &widget[id].w,
                                                &widget[id].h,
                                                text, font[size]);
        widget[id].size   = size;
        widget[id].color0 = c0 ? c0 : gui_yel;
        widget[id].color1 = c1 ? c1 : gui_red;
        widget[id].rect   = rect;
    }
    return id;
}

int gui_count(int pd, int value, int size, int rect)
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
        widget[id].rect   = rect;
    }
    return id;
}

int gui_clock(int pd, int value, int size, int rect)
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
        widget[id].rect   = rect;
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

int gui_multi(int pd, const char *text, int size, int rect, const GLubyte *c0,
                                                            const GLubyte *c1)
{
    int id = 0;

    if (text && (id = gui_varray(pd)))
    {
        const char *p;

        char s[GUI_LINES][MAXSTR];
        int  r[GUI_LINES];
        int  i, j;

        size_t n = 0;

        /* Copy each delimited string to a line buffer. */

        for (p = text, j = 0; *p && j < GUI_LINES; j++)
        {
            strncpy(s[j], p, (n = strcspn(p, "\\")));
            s[j][n] = 0;
            r[j]    = 0;

            if (*(p += n) == '\\') p++;
        }

        /* Set the curves for the first and last lines. */

        if (j > 0)
        {
            r[0]     |= rect & (GUI_NW | GUI_NE);
            r[j - 1] |= rect & (GUI_SW | GUI_SE);
        }

        /* Create a label widget for each line. */

        for (i = 0; i < j; i++)
            gui_label(id, s[i], size, r[i], c0, c1);
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
    /* Store width and height for later use in text rendering. */

    widget[id].text_w = widget[id].w;
    widget[id].text_h = widget[id].h;

    if (widget[id].w < widget[id].h && widget[id].w > 0)
        widget[id].w = widget[id].h;

    /* Padded text elements look a little nicer. */

    if (widget[id].w < config_get_d(CONFIG_WIDTH))
        widget[id].w += radius;
    if (widget[id].h < config_get_d(CONFIG_HEIGHT))
        widget[id].h += radius;

    /* A button should be at least wide enough to accomodate the rounding. */

    if (widget[id].w < 2 * radius)
        widget[id].w = 2 * radius;
    if (widget[id].h < 2 * radius)
        widget[id].h = 2 * radius;
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
    /* Recall stored width and height for text rendering. */

    int W = widget[id].text_w;
    int H = widget[id].text_h;
    int R = widget[id].rect;

    const GLubyte *c0 = widget[id].color0;
    const GLubyte *c1 = widget[id].color1;

    widget[id].x = x;
    widget[id].y = y;
    widget[id].w = w;
    widget[id].h = h;

    /* Create vertex array data for the text area and rounded rectangle. */

    gui_rect(id, -w / 2, -h / 2, w, h, R, radius);
    gui_text(id, -W / 2, -H / 2, W, H, c0, c1);
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

    int w, W = config_get_d(CONFIG_WIDTH);
    int h, H = config_get_d(CONFIG_HEIGHT);

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

        glDeleteTextures(1, &widget[id].image);

        /* Mark this widget unused. */

        widget[id].type  = GUI_FREE;
        widget[id].flags = 0;
        widget[id].image = 0;
        widget[id].cdr   = 0;
        widget[id].car   = 0;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static void gui_paint_rect(int id, int st)
{
    static const GLfloat back[4][4] = {
        { 0.1f, 0.1f, 0.1f, 0.5f },             /* off and inactive    */
        { 0.5f, 0.5f, 0.5f, 0.8f },             /* off and   active    */
        { 1.0f, 0.7f, 0.3f, 0.5f },             /* on  and inactive    */
        { 1.0f, 0.7f, 0.3f, 0.8f },             /* on  and   active    */
    };

    int jd, i = 0;

    /* Use the widget status to determine the background color. */

    if (gui_hot(id))
        i = st | (((widget[id].value) ? 2 : 0) |
                  ((id == active)     ? 1 : 0));

    switch (widget[id].type)
    {
    case GUI_IMAGE:
    case GUI_SPACE:
    case GUI_FILLER:
        break;

    case GUI_HARRAY:
    case GUI_VARRAY:
    case GUI_HSTACK:
    case GUI_VSTACK:

        /* Recursively paint all subwidgets. */

        for (jd = widget[id].car; jd; jd = widget[jd].cdr)
            gui_paint_rect(jd, i);

        break;

    default:

        /* Draw a leaf's background, colored by widget state. */

        glPushMatrix();
        {
            glTranslatef((GLfloat) (widget[id].x + widget[id].w / 2),
                         (GLfloat) (widget[id].y + widget[id].h / 2), 0.f);

            glColor4f(back[i][0], back[i][1], back[i][2], back[i][3]);
            draw_rect(id);
        }
        glPopMatrix();

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

        if (1.0 < ck || ck < 1.0)
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
        draw_rect(id);
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
                int id = digit_id[i][j % 10];

                glBindTexture(GL_TEXTURE_2D, widget[id].image);
                draw_text(id);
                glTranslatef((GLfloat) -widget[id].text_w, 0.0f, 0.0f);
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
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            {
                draw_enable(GL_FALSE, GL_FALSE, GL_TRUE);
                glDisable(GL_TEXTURE_2D);
                gui_paint_rect(id, 0);

                draw_enable(GL_TRUE, GL_TRUE, GL_TRUE);
                glEnable(GL_TEXTURE_2D);
                gui_paint_text(id);

                draw_disable();
                glColor4ub(gui_wht[0], gui_wht[1], gui_wht[2], gui_wht[3]);
            }
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_LIGHTING);
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

    /* Short-circuit check the current active widget. */

    jd = gui_search(active, x, y);

    /* If not still active, search the hierarchy for a new active widget. */

    if (jd == 0)
        jd = gui_search(id, x, y);

    /* If the active widget has changed, return the new active id. */

    if (jd == 0)
        return active = 0;
    else if (jd == active)
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
    widget[id].value = widget[id].value ? 0 : 1;
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

static int gui_vert_dist(int id, int jd)
{
    /* Vertical distance between the tops of id and jd */

    return abs((widget[id].y + widget[id].h) - (widget[jd].y + widget[jd].h));
}

static int gui_horz_dist(int id, int jd)
{
    /* Horizontal distance between the left sides of id and jd */

    return abs(widget[id].x - widget[jd].x);
}

/*---------------------------------------------------------------------------*/

static int gui_stick_L(int id, int dd)
{
    int jd, kd, hd;
    int o, omin, d, dmin;

    /* Find the closest "hot" widget to the left of dd (and inside id) */

    if (id && gui_hot(id))
        return id;

    hd = 0;
    omin = widget[dd].x - widget[id].x + 1;
    dmin = widget[dd].y + widget[dd].h + widget[id].y + widget[id].h;

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        kd = gui_stick_L(jd, dd);

        if (kd && kd != dd)
        {
            o = gui_horz_offset(dd, kd);
            d = gui_vert_dist(dd, kd);

            if (0 <= o && o <= omin && d <= dmin)
            {
                hd = kd;
                omin = o;
                dmin = d;
            }
        }
    }

    return hd;
}

static int gui_stick_R(int id, int dd)
{
    int jd, kd, hd;
    int o, omin, d, dmin;

    /* Find the closest "hot" widget to the right of dd (and inside id) */

    if (id && gui_hot(id))
        return id;

    hd = 0;
    omin = (widget[id].x + widget[id].w) - (widget[dd].x + widget[dd].w) + 1;
    dmin = (widget[dd].y + widget[dd].h) + (widget[id].y + widget[id].h);

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        kd = gui_stick_R(jd, dd);

        if (kd && kd != dd)
        {
            o = gui_horz_offset(kd, dd);
            d = gui_vert_dist(dd, kd);

            if (0 <= o && o <= omin && d <= dmin)
            {
                hd = kd;
                omin = o;
                dmin = d;
            }
        }
    }

    return hd;
}

static int gui_stick_D(int id, int dd)
{
    int jd, kd, hd;
    int o, omin, d, dmin;

    /* Find the closest "hot" widget below dd (and inside id) */

    if (id && gui_hot(id))
        return id;

    hd = 0;
    omin = widget[dd].y - widget[id].y + 1;
    dmin = widget[dd].x + widget[dd].w + widget[id].x + widget[id].w;

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        kd = gui_stick_D(jd, dd);

        if (kd && kd != dd)
        {
            o = gui_vert_offset(dd, kd);
            d = gui_horz_dist(dd, kd);

            if (0 <= o && o <= omin && d <= dmin)
            {
                hd = kd;
                omin = o;
                dmin = d;
            }
        }
    }

    return hd;
}

static int gui_stick_U(int id, int dd)
{
    int jd, kd, hd;
    int o, omin, d, dmin;

    /* Find the closest "hot" widget above dd (and inside id) */

    if (id && gui_hot(id))
        return id;

    hd = 0;
    omin = (widget[id].y + widget[id].h) - (widget[dd].y + widget[dd].h) + 1;
    dmin = (widget[dd].x + widget[dd].w) + (widget[id].x + widget[id].w);

    for (jd = widget[id].car; jd; jd = widget[jd].cdr)
    {
        kd = gui_stick_U(jd, dd);

        if (kd && kd != dd)
        {
            o = gui_vert_offset(kd, dd);
            d = gui_horz_dist(dd, kd);

            if (0 <= o && o <= omin && d <= dmin)
            {
                hd = kd;
                omin = o;
                dmin = d;
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

    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
    {
        if (v < 0) jd = gui_wrap_L(id, active);
        if (v > 0) jd = gui_wrap_R(id, active);
    }
    else if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
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
            sticky = active;
            return 0;
        }
        else
        {
            int c = (sticky && sticky == active);
            sticky = 0;
            return c;
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
