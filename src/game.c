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

#include <SDL/SDL_mixer.h>
#include <math.h>

#include "gl.h"
#include "main.h"
#include "vec3.h"
#include "ball.h"
#include "coin.h"
#include "image.h"
#include "solid.h"

#define MAX_DT  0.01                   /* 100Hz maximum physics update cycle */

/*---------------------------------------------------------------------------*/

#define COIN_S "data/png/coin_label.png"
#define TIME_S "data/png/time_label.png"

#define DIG0_S "data/png/digit0.png"
#define DIG1_S "data/png/digit1.png"
#define DIG2_S "data/png/digit2.png"
#define DIG3_S "data/png/digit3.png"
#define DIG4_S "data/png/digit4.png"
#define DIG5_S "data/png/digit5.png"
#define DIG6_S "data/png/digit6.png"
#define DIG7_S "data/png/digit7.png"
#define DIG8_S "data/png/digit8.png"
#define DIG9_S "data/png/digit9.png"

static struct image coin_i, *coin_p = &coin_i;
static struct image time_i, *time_p = &time_i;

static struct image dig_i[10];

/*---------------------------------------------------------------------------*/

#define TINY  0.0000000005

static double time   = 0.0;
static int    score  = 0;
static int    balls  = 0;
static int    ticks  = 0;

static Mix_Chunk *coin_wav;
static Mix_Chunk *tick_wav;
static Mix_Chunk *bump_wav;

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

void game_init(void)
{
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

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

    bump_wav = Mix_LoadWAV("data/wav/bump.wav");
    coin_wav = Mix_LoadWAV("data/wav/coin.wav");
    tick_wav = Mix_LoadWAV("data/wav/tick.wav");

    image_load(coin_p, COIN_S);
    image_load(time_p, TIME_S);

    image_load(dig_i + 0, DIG0_S);
    image_load(dig_i + 1, DIG1_S);
    image_load(dig_i + 2, DIG2_S);
    image_load(dig_i + 3, DIG3_S);
    image_load(dig_i + 4, DIG4_S);
    image_load(dig_i + 5, DIG5_S);
    image_load(dig_i + 6, DIG6_S);
    image_load(dig_i + 7, DIG7_S);
    image_load(dig_i + 8, DIG8_S);
    image_load(dig_i + 9, DIG9_S);

    score = 0;
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

        image_rect(dig_i + (balls / 10), 0.00, 0.00, 0.09, 0.09, 1.0);
        image_rect(dig_i + (balls % 10), 0.06, 0.00, 0.15, 0.09, 1.0);

        image_rect(dig_i + (score / 10), 0.85, 0.00, 0.94, 0.09, 1.0);
        image_rect(dig_i + (score % 10), 0.91, 0.00, 1.00, 0.09, 1.0);

        image_rect(dig_i + (ticks / 10), 0.45, 0.00, 0.57, 0.15, 1.0);
        image_rect(dig_i + (ticks % 10), 0.53, 0.00, 0.65, 0.15, 1.0);

        image_rect(coin_p, 0.75, 0.03, 0.85, 0.06, 1.0);
        image_rect(time_p, 0.35, 0.03, 0.45, 0.06, 1.0);
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static void game_render_all(const struct s_file *fp,
                            const struct s_ball *up)
{
    const float light_p[4]  = { 8.0, 32.0, 8.0, 1.0 };

    glLightfv(GL_LIGHT0, GL_POSITION, light_p);

    sol_render(fp);

    glPushAttrib(GL_ENABLE_BIT);
    {
        coin_draw(list_coin, fp->cv, fp->cc);
        ball_draw(list_ball, up->r, up->p, up->e);
    }
    glPopAttrib();
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

        /* Rotate the world about the position of the ball. */

        glTranslated(+up->p[0], +up->p[1], +up->p[2]);
        glRotated(-game_rz, view_e[2][0], view_e[2][1], view_e[2][2]);
        glRotated(-game_rx, view_e[0][0], view_e[0][1], view_e[0][2]);
        glTranslated(-up->p[0], -up->p[1], -up->p[2]);

        /* Draw the scene. */

        game_render_all(fp, up);
    }
    glPopMatrix();
}

int game_update_env(const double g[3], double real_dt)
{
    struct s_file *fp = &file;

    double X[16], Z[16], M[16];
    double x[3], y[3] = { 0.0, 1.0, 0.0 }, z[3];
    double a[3], h[3];
    double ay, az, dt;
    double bump = 0.0;
    int i, n;

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

    /*
     * Run the sim.  On most  hardware, rendering requires much more computing
     * power than  physics.  Since physics  takes less time than  graphics, it
     * make sense  to detach  the physics update  time step from  the graphics
     * frame rate.  By  performing multiple physics updates  for each graphics
     * update, we get  away with higher quality physics  with little impact on
     * overall performance.
     *
     * Toward this end, we establish a baseline maximum physics time step.  If
     * the measured frame  time exceeds this maximum, we cut  the time step in
     * half, and do two updates.  If THIS time step exceeds the maximum, we do
     * four updates.  And  so on.  In this way, the  physics system is allowed
     * to seek  an optimal  update rate independant  of, yet in  integral sync
     * with, the graphics frame rate.
     *
     * If both your CPU and your  graphics card suck in perfect balance, it's
     * not my problem.
     */

    n  = 1;
    dt = real_dt;

    while (dt > MAX_DT)
    {
        dt /= 2;
        n  *= 2;
    }

    for (i = 0; i < n; i++)
        sol_update(fp, dt, h, &bump);

    /* Create the sound of the ball. */

    if (bump > 0.25)
    {
        Mix_Volume(0, (int) (bump * MIX_MAX_VOLUME));
        Mix_PlayChannel(0, bump_wav, 0);
    }

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

    time -= real_dt;

    if ((int) floor(time) < ticks)
    {
        Mix_PlayChannel(1, tick_wav, 0);
        ticks = (int) floor(time);
    }

    /* Test for a coin grab and a possible 1UP. */

    if ((n = coin_test(fp->uv, fp->cv, fp->cc)) > 0)
    {
        Mix_PlayChannel(2, coin_wav, 0);
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

