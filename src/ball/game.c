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

#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <math.h>

#include <sol.h>
#include <vec.h>
#include <aio.h>
#include <etc.h>

#include "main.h"
#include "ball.h"
#include "coin.h"

#define REFLECT 1                      /* Enable the reflective floor effect */
#define MIN_DT  0.02                   /* 50Hz minimum physics update cycle  */

/*---------------------------------------------------------------------------*/

#define TINY  0.0000000005

static double time   = 0.0;
static int    score  = 0;
static int    balls  = 0;
static int    ticks  = 0;

short *coin_buf = NULL;
int    coin_len =    0;
short *tick_buf = NULL;
int    tick_len =    0;
short *bump_buf = NULL;
int    bump_len =    0;

static GLuint list_ball = 0;
static GLuint list_coin = 0;

static struct s_file file;

static double game_rx = 0.0;
static double game_rz = 0.0;

static const double view_y = 4.0;
static const double view_z = 6.0;

static double view_p[3];
static double view_e[3][3];

#define EV_NONE 0
#define EV_TIME 1
#define EV_GOAL 2
#define EV_FALL 3

/*---------------------------------------------------------------------------*/

static void  *coin_label_p = NULL;
static GLuint coin_label_o = 0;
static void  *time_label_p = NULL;
static GLuint time_label_o = 0;

static void init_labels(void)
{
    int w, h, b;

    coin_label_p = etc_load_png("data/png/coin_label.png", &w, &h, &b);
    coin_label_o = etc_make_tex(coin_label_p, w, h, b);

    time_label_p = etc_load_png("data/png/time_label.png", &w, &h, &b);
    time_label_o = etc_make_tex(time_label_p, w, h, b);
}

/*---------------------------------------------------------------------------*/

static void  *dig_p[10];
static int    dig_w[10];
static int    dig_h[10];
static int    dig_b[10];
static GLuint dig_o[10];

static void init_digits(void)
{
    dig_p[0] = etc_load_png("data/png/digit0.png", dig_w+0, dig_h+0, dig_b+0);
    dig_p[1] = etc_load_png("data/png/digit1.png", dig_w+1, dig_h+1, dig_b+1);
    dig_p[2] = etc_load_png("data/png/digit2.png", dig_w+2, dig_h+2, dig_b+2);
    dig_p[3] = etc_load_png("data/png/digit3.png", dig_w+3, dig_h+3, dig_b+3);
    dig_p[4] = etc_load_png("data/png/digit4.png", dig_w+4, dig_h+4, dig_b+4);
    dig_p[5] = etc_load_png("data/png/digit5.png", dig_w+5, dig_h+5, dig_b+5);
    dig_p[6] = etc_load_png("data/png/digit6.png", dig_w+6, dig_h+6, dig_b+6);
    dig_p[7] = etc_load_png("data/png/digit7.png", dig_w+7, dig_h+7, dig_b+7);
    dig_p[8] = etc_load_png("data/png/digit8.png", dig_w+8, dig_h+8, dig_b+8);
    dig_p[9] = etc_load_png("data/png/digit9.png", dig_w+9, dig_h+9, dig_b+9);

    dig_o[0] = etc_make_tex(dig_p[0], dig_w[0], dig_h[0], dig_b[0]);
    dig_o[1] = etc_make_tex(dig_p[1], dig_w[1], dig_h[1], dig_b[1]);
    dig_o[2] = etc_make_tex(dig_p[2], dig_w[2], dig_h[2], dig_b[2]);
    dig_o[3] = etc_make_tex(dig_p[3], dig_w[3], dig_h[3], dig_b[3]);
    dig_o[4] = etc_make_tex(dig_p[4], dig_w[4], dig_h[4], dig_b[4]);
    dig_o[5] = etc_make_tex(dig_p[5], dig_w[5], dig_h[5], dig_b[5]);
    dig_o[6] = etc_make_tex(dig_p[6], dig_w[6], dig_h[6], dig_b[6]);
    dig_o[7] = etc_make_tex(dig_p[7], dig_w[7], dig_h[7], dig_b[7]);
    dig_o[8] = etc_make_tex(dig_p[8], dig_w[8], dig_h[8], dig_b[8]);
    dig_o[9] = etc_make_tex(dig_p[9], dig_w[9], dig_h[9], dig_b[9]);
}

/*---------------------------------------------------------------------------*/

void game_init(void)
{
    glClearColor(0.0, 0.0, 0.2, 1.0);

    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    list_ball = ball_init(4);
    list_coin = coin_init(32);

    bump_buf = aio_load("data/sw/bump.sw", &bump_len);
    coin_buf = aio_load("data/sw/coin.sw", &coin_len);
    tick_buf = aio_load("data/sw/tick.sw", &tick_len);

    init_digits();
    init_labels();

    score   = 0;
}

/*---------------------------------------------------------------------------*/

void game_render_hud(void)
{
    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
    }
    glMatrixMode(GL_MODELVIEW);

    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_COLOR_MATERIAL);

        glColor4d(1.0, 1.0, 1.0, 1.0);

        etc_draw_tex(dig_o[balls / 10], 0.00, 0.00, 0.09, 0.09, 1.0);
        etc_draw_tex(dig_o[balls % 10], 0.06, 0.00, 0.15, 0.09, 1.0);

        etc_draw_tex(coin_label_o,      0.75, 0.03, 0.85, 0.06, 1.0);
        etc_draw_tex(dig_o[score / 10], 0.85, 0.00, 0.94, 0.09, 1.0);
        etc_draw_tex(dig_o[score % 10], 0.91, 0.00, 1.00, 0.09, 1.0);

        etc_draw_tex(time_label_o,      0.35, 0.03, 0.45, 0.06, 1.0);
        etc_draw_tex(dig_o[ticks / 10], 0.45, 0.00, 0.57, 0.15, 1.0);
        etc_draw_tex(dig_o[ticks % 10], 0.53, 0.00, 0.65, 0.15, 1.0);
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static void game_render_all(const struct s_file *fp,
                            const struct s_ball *up, int f, int x)
{
    const float light_p[4]  = { 8.0, 32.0, 8.0, 1.0 };

    glLightfv(GL_LIGHT0, GL_POSITION, light_p);

    sol_render(fp, f);

    if (f == 0)
    {
        glPushAttrib(GL_ENABLE_BIT);
        {
            glDisable(GL_TEXTURE_2D);

            coin_draw(list_coin, fp->cv, fp->cc);
            ball_draw(list_ball, up->r, up->p, up->e, x);
        }
        glPopAttrib();
    }
}

void game_render_env(void)
{
    const struct s_file *fp = &file;
    const struct s_ball *up =  file.uv;
    
    GLint vp[4];

    glMatrixMode(GL_PROJECTION);
    {
        glGetIntegerv(GL_VIEWPORT, vp);
        glLoadIdentity();
        gluPerspective(30.0, (GLdouble) vp[2] / (GLdouble) vp[3], 0.1, 1000.0);
    }
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    {
        gluLookAt(view_p[0], view_p[1], view_p[2],
                   up->p[0],  up->p[1],  up->p[2], 0, 1, 0);

        /* Rotate the world about the position of the ball */

        glTranslated(+up->p[0], +up->p[1], +up->p[2]);
        glRotated(-game_rz, view_e[2][0], view_e[2][1], view_e[2][2]);
        glRotated(-game_rx, view_e[0][0], view_e[0][1], view_e[0][2]);
        glTranslated(-up->p[0], -up->p[1], -up->p[2]);

#ifdef REFLECT
        glEnable(GL_STENCIL_TEST);
        {
            /* Fill the stencil buffer with floor geometry. */

            glDisable(GL_DEPTH_TEST);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            {
                glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
                {
                    game_render_all(fp, up, 1, 0);
                }
                glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            }
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glEnable(GL_DEPTH_TEST);

            /* Draw the reflected world in the stenciled area. */

            glPushAttrib(GL_ENABLE_BIT);
            glPushMatrix();
            {
                GLdouble eq[4] = { 0.0, 1.0, 0.0, 0.0 };
                
                glScalef(1.0, -1.0, 1.0);
                
                glEnable(GL_CLIP_PLANE0);
                glClipPlane(GL_CLIP_PLANE0, eq);

                glCullFace(GL_FRONT);
                game_render_all(fp, up, 0, 1);
                glCullFace(GL_BACK);
            }
            glPopMatrix();
            glPopAttrib();
        }
        glDisable(GL_STENCIL_TEST);
#endif

        /* Blend the real world overtop the floor reflection. */

        game_render_all(fp, up, 1, 0);
        game_render_all(fp, up, 0, 0);
    }
    glPopMatrix();
}

int game_update_env(const double g[3], double real_dt)
{
    struct s_file *fp = &file;

    double X[16], Z[16], M[16];
    double x[3], y[3] = { 0.0, 1.0, 0.0 }, z[3];
    double a[3], h[3];
    double bump, roll;
    double ay, az, dt;
    int n;

    dt    = (real_dt > MIN_DT) ? real_dt : MIN_DT;
    time -= dt;

    /* Compute the gravity vector from the given world rotations. */

    v_sub(z, view_p, fp->uv->p);
    v_crs(x, y, z);
    v_crs(z, x, y);
    v_nrm(x, x);
    v_nrm(z, z);

    m_rot (Z, z, V_RAD(game_rz));
    m_rot (X, x, V_RAD(game_rx));
    m_mult(M, Z, X);
    m_vxfm(h, M, g);

    /* Run the sim. */

    sol_update(fp, dt, h, &bump, &roll);

    /* Create the sound of the ball. */

    if (bump > 0.50)
        aio_play(bump_buf, bump_len, MIN(bump / 2.0, 1.0));

    /*
    aio_freq(roll_aio, roll);
    aio_ampl(roll_aio, MIN(roll / 50.0, 0.3));
    */

    /* Orthonormalize the basis of the view of the ball in its new position. */

    v_sub(view_e[2], view_p, fp->uv->p);
    v_crs(view_e[0], view_e[1], view_e[2]);
    v_crs(view_e[2], view_e[0], view_e[1]);
    v_nrm(view_e[0], view_e[0]);
    v_nrm(view_e[2], view_e[2]);

    v_sub(a, view_p, fp->uv->p);
    ay = v_dot(view_e[1], a);
    az = v_dot(view_e[2], a);

    /* Compute the new view position. */

    view_p[0] = view_p[1] = view_p[2] = 0.0;

    v_mad(view_p, fp->uv->p, view_e[1], ay < view_y ? ay : view_y);
    v_mad(view_p,    view_p, view_e[2], az < view_z ? az : view_z);

    /* The clock ticks. */

    if ((int) floor(time) < ticks)
    {
        aio_play(tick_buf, tick_len, 0.5);
        ticks = (int) floor(time);
    }

    /* Test for a coin grab and a possible 1UP. */

    if ((n = coin_test(fp->uv, fp->cv, fp->cc)) > 0)
    {
        aio_play(coin_buf, coin_len, 0.8);
        score += n;

        if (score >= 100)
        {
            score -= 100;
            balls++;
        }
    }

    /* Test for a goal. */

    if ((n = sol_inside(fp)) >= 0)
        if (fp->lv[n].gc == 0)
            return EV_GOAL;

    /* Test for time-out. */

    if (time <= 0.0)
        return EV_TIME;

    /* Test for fall-out. */

    if (fp->uv[0].p[1] < -10.0)
        return EV_FALL;

    return EV_NONE;
}

void game_update_pos(int x, int y)
{
    double bound = 20.0;

    game_rx -= 40.0 * y / 500;
    game_rz += 40.0 * x / 500;

    if (game_rx > +bound) game_rx = +bound;
    if (game_rx < -bound) game_rx = -bound;
    if (game_rz > +bound) game_rz = +bound;
    if (game_rz < -bound) game_rz = -bound;
}

void game_update_fly(double k)
{
    struct s_file *fp = &file;

    if (fp->pc > 0)
    {
        double p[3];
        double v[3];

        p[0] = p[1] = p[2] = 0.0;

        v_cpy(p, fp->uv[0].p);

        v_mad(p, p, view_e[1], view_y);
        v_mad(p, p, view_e[2], view_z);

        v_sub(v, fp->pv[0].p, p);
        v_mad(view_p, p, v, k);
    }
}

/*---------------------------------------------------------------------------*/

void game_start(void)
{
    score = 0;
    balls = 2;
}

int game_fail(void)
{
    return (--balls >= 0);
}

void game_load(const char *s, int t)
{
    sol_load(&file, s);

    time   = t;
    ticks  = t;

    game_rx = 0.0;
    game_rz = 0.0;

    view_p[0] =    0.0;
    view_p[1] = view_y;
    view_p[2] = view_z;

    view_e[0][0] = 1.0;
    view_e[0][1] = 0.0;
    view_e[0][2] = 0.0;
    view_e[1][0] = 0.0;
    view_e[1][1] = 1.0;
    view_e[1][2] = 0.0;
    view_e[2][0] = 0.0;
    view_e[2][1] = 0.0;
    view_e[2][2] = 1.0;
}

void game_free(void)
{
    sol_free(&file);
}

/*---------------------------------------------------------------------------*/

