/*   Copyright (C) 2003  Robert Kooima                                       */
/*                                                                           */
/*   SUPER EMPTY BALL  is  free software; you  can redistribute  it and/or   */
/*   modify  it under  the  terms  of  the  GNU General Public License  as   */
/*   published by  the Free Software Foundation;  either version 2  of the   */
/*   License, or (at your option) any later version.                         */
/*                                                                           */
/*   This program is  distributed in the hope that it  will be useful, but   */
/*   WITHOUT  ANY   WARRANTY;  without   even  the  implied   warranty  of   */
/*   MERCHANTABILITY  or FITNESS FOR  A PARTICULAR  PURPOSE.  See  the GNU   */
/*   General Public License for more details.                                */

#include <stdlib.h>
#include <GL/glx.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include "glw.h"

/*---------------------------------------------------------------------------*/

static Display      *dpy;
static Window        win;
static XVisualInfo  *xvi;
static Atom          del;
static XSizeHints   *siz;
static GLXContext    ctx;

static Cursor crs0;
static Cursor crs1;

static int center_x = 0;
static int center_y = 0;

static int last_x = 0;
static int last_y = 0;
static int warp   = 0;

/*---------------------------------------------------------------------------*/

static int glx_init_dpy(void)
{
    if ((dpy = XOpenDisplay(NULL)) == NULL)
        return 0;

    if (glXQueryExtension(dpy, NULL, NULL) == False)
        return 0;
	
    return 1;
}

static int glx_init_xvi(int e)
{
    int attributes[] = {
        GLX_STEREO,
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_DEPTH_SIZE, 16,
        GLX_STENCIL_SIZE, 1,
        None
    };

    int *p = (e == 2) ? attributes : attributes + 1;

    if ((xvi = glXChooseVisual(dpy, DefaultScreen(dpy), p)) == NULL)
        return 0;

    if (xvi->class != TrueColor)
        return 0;

    return 1;
}


static int glx_init_dec(void)
{
    unsigned long hint[5] = { 2, 0, 0, 0, 0 };
    Atom prop;

    if (!(prop = XInternAtom(dpy, "_MOTIF_WM_HINTS", True)))
        return 0;

    XChangeProperty(dpy, win, prop, prop, 32, PropModeReplace,
                    (unsigned char *) hint, 5);

    return 1;
}

static int glx_init_win(int w, int h)
{
    int W = w ? w : DisplayWidth (dpy, xvi->screen);
    int H = h ? h : DisplayHeight(dpy, xvi->screen);

    Window root = RootWindow(dpy, xvi->screen);
    Visual *vis = xvi->visual;
    XSetWindowAttributes wa;

    unsigned long valuemask = CWBorderPixel
                            | CWColormap
                            | CWEventMask;

    wa.border_pixel = 0;
    wa.colormap     = XCreateColormap(dpy, root, vis, AllocNone);
    wa.event_mask   = StructureNotifyMask
                    | ExposureMask
                    | KeyPressMask
                    | KeyReleaseMask
                    | PointerMotionMask
                    | ButtonPressMask
                    | ButtonReleaseMask;

    del = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    win = XCreateWindow(dpy, root, 0, 0, W, H, 0, xvi->depth,
                        InputOutput, vis, valuemask, &wa);

    XSetWMProtocols(dpy, win, &del, 1);

    return (w || h) ? 1 : glx_init_dec();
}

static int glx_init_hnt(const char *s)
{
    int w = DisplayWidth (dpy, DefaultScreen(dpy));
    int h = DisplayHeight(dpy, DefaultScreen(dpy));

    siz = XAllocSizeHints();

    if (siz)
    {
        siz->min_width  = 160;
        siz->min_height = 120;
        siz->max_width  = w;
        siz->max_height = h;
        
        siz->flags = PMinSize | PMaxSize;

        XmbSetWMProperties(dpy, win, s, s, NULL, 0, siz, NULL, NULL);

        return 1;
    }
    return 0;
}

static int glx_init_crs(void)
{
    char   bit[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    Pixmap pix;
    XColor rgb;

    rgb.red   = 0;
    rgb.green = 0;
    rgb.blue  = 0;

    pix = XCreateBitmapFromData(dpy, win, bit, 8, 8);

    crs0 = XCreatePixmapCursor(dpy, pix, pix, &rgb, &rgb, 0, 0);
    crs1 = XCreateFontCursor(dpy, XC_left_ptr);

    XFreePixmap(dpy, pix);

    return 1;
}

static int glx_init_ctx(void)
{
    if ((ctx = glXCreateContext(dpy, xvi, NULL, True)) == NULL)
        return 0;

    if (glXMakeCurrent(dpy, win, ctx) == True)
        return 1;
    else
        return 0;
}

/*---------------------------------------------------------------------------*/

static int glx_client(XClientMessageEvent *e)
{
    if (e->data.l[0] == del)
        return GLW_CLOSE;
    else
        return 0;
}

static int glx_render(XExposeEvent *e)
{
    if (e->count == 0)
        return GLW_RENDER;
    else
        return 0;
}

static int glx_resize(XConfigureEvent *e)
{
    last_y = e->height;
    last_x = e->width;

    center_x = e->width  / 2;
    center_y = e->height / 2;

    return GLW_RESIZE;
}

/*---------------------------------------------------------------------------*/

static int glx_point(XMotionEvent *e)
{
    last_x = e->x - center_x;
    last_y = e->y - center_y;

    if (warp) XWarpPointer(dpy, None, win, 0, 0, 0, 0, center_x, center_y);

    return GLW_MOTION;
}

static int glx_btn_d(XButtonEvent *e)
{
    last_x = e->x;
    last_y = e->y;

    switch (e->button)
    {
    case 1: return GLW_L_BTN_D;
    case 2: return GLW_M_BTN_D;
    case 3: return GLW_R_BTN_D;
    case 4: return GLW_WHEEL_D;
    case 5: return GLW_WHEEL_U;
    }
    return 0;
}

static int glx_btn_u(XButtonEvent *e)
{
    last_x = e->x;
    last_y = e->y;

    switch (e->button)
    {
    case 1: return GLW_L_BTN_U;
    case 2: return GLW_M_BTN_U;
    case 3: return GLW_R_BTN_U;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static int glx_key_d(XKeyEvent *e)
{
    char c;

    XLookupString(e, &c, 1, NULL, NULL);
    last_x = (int) c;

    return GLW_KEY_D;
}

static int glx_key_u(XKeyEvent *e)
{
    char c;

    XLookupString(e, &c, 1, NULL, NULL);
    last_x = (int) c;

    return GLW_KEY_U;
}

/*---------------------------------------------------------------------------*/

int glw_create(const char *s, int w, int h, int e)
{
    if (dpy == NULL)
    {
        if (!glx_init_dpy())     return 0;
        if (!glx_init_xvi(e))    return 0;
        if (!glx_init_win(w, h)) return 0;
        if (!glx_init_hnt(s))    return 0;
        if (!glx_init_crs())     return 0;
        if (!glx_init_ctx())     return 0;
        
        XMapWindow(dpy, win);
    }
    return 1;
}

void glw_delete(void)
{
    if (dpy != NULL)
    {
        glXMakeCurrent(dpy, None, NULL);
        glXDestroyContext(dpy, ctx);

        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);

        dpy = NULL;
    }
}

/*---------------------------------------------------------------------------*/

void glw_acquire(void)
{
   XDefineCursor(dpy, win, crs0);
   XWarpPointer(dpy, None, win, 0, 0, 0, 0, center_x, center_y);
   warp = 1;
}

void glw_release(void)
{
   XDefineCursor(dpy, win, crs1);
   warp = 0;
}

/*---------------------------------------------------------------------------*/

int glw_update(int dirty, int wait)
{
    XEvent e;

    if (dirty) glXSwapBuffers(dpy, win);

    if (wait || XPending(dpy) > 0)
    {
        XNextEvent(dpy, &e);

        switch (e.type)
        {
        case ClientMessage:   return glx_client(&e.xclient);
        case Expose:          return glx_render(&e.xexpose);
        case ConfigureNotify: return glx_resize(&e.xconfigure);
            
        case MotionNotify:    return glx_point(&e.xmotion);
        case ButtonPress:     return glx_btn_d(&e.xbutton);
        case ButtonRelease:   return glx_btn_u(&e.xbutton);
        case KeyPress:        return glx_key_d(&e.xkey);
        case KeyRelease:      return glx_key_u(&e.xkey);
        }
    }
    return 0;
}

int glw_x(void)
{
    return last_x;
}

int glw_y(void)
{
    return last_y;
}
