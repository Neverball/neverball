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

#include <math.h>

#include "gl.h"
#include "main.h"
#include "vec3.h"
#include "geom.h"
#include "text.h"
#include "game.h"
#include "image.h"
#include "audio.h"
#include "solid.h"
#include "level.h"

#define MAX_DT  0.01                   /* 100Hz maximum physics update cycle */
#define FOV    60.0

/*---------------------------------------------------------------------------*/

static double time  = 0.0;
static int    ticks = 0;

static struct s_file file;

static double game_rx;
static double game_rz;

static double view_r;
static double view_dy;
static double view_dz;

static double view_p[3];
static double view_e[3][3];

/*---------------------------------------------------------------------------*/

#define CW 0.10
#define CH 0.10
#define CS 0.07

static void game_render_dig(int d, double x, double y, double s)
{
    text_digit(d, 
               x - CW * s * 0.5, y - CH * s * 0.5,
               x + CW * s * 0.5, y + CH * s * 0.5);
}

void game_render_hud(void)
{
    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
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

        text_label(TXT_BALLS, 0.0, 0.01, 0.1, 0.04);
        text_label(TXT_COINS, 0.9, 0.01, 1.0, 0.04);

        game_render_dig((balls / 10), 0.1 + CS * 0.5, CH * 0.5, 1.0);
        game_render_dig((balls % 10), 0.1 + CS * 1.5, CH * 0.5, 1.0);

        game_render_dig((score / 10), 0.9 - CS * 1.5, CH * 0.5, 1.0);
        game_render_dig((score % 10), 0.9 - CS * 0.5, CH * 0.5, 1.0);

        game_render_dig((ticks / 10), 0.5 - CS, CH, 2.0);
        game_render_dig((ticks % 10), 0.5 + CS, CH, 2.0);
    }
    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    {
        glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
}

void game_render_num(int num)
{
    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
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

        game_render_dig((num / 10), 0.5 - CS, 0.5, 2.0);
        game_render_dig((num % 10), 0.5 + CS, 0.5, 2.0);
    }
    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    {
        glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------------*/

void game_render_env(void)
{
    const float light_p[4] = { 8.0, 32.0, 8.0, 1.0 };

    const struct s_file *fp = &file;
    const struct s_ball *up =  file.uv;

    const double *ball_p = file.uv->p;
    
    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        gluPerspective(FOV, (GLdouble) main_width / main_height, 0.1, 200.0);
    }
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    {
        double v[3], rx, ry;

        v_sub(v, ball_p, view_p);

        rx = V_DEG(atan2(-v[1], sqrt(v[0] * v[0] + v[2] * v[2])));
        ry = V_DEG(atan2(+v[0], -v[2]));

        glTranslatef(0.0f, 0.0f, -v_len(v));
        glRotatef(rx, 1.0f, 0.0f, 0.0f);
        glRotatef(ry, 0.0f, 1.0f, 0.0f);
        glTranslatef(-ball_p[0], -ball_p[1], -ball_p[2]);

        /* Center the skybox about the position of the camera. */

        glPushMatrix();
        {
            glTranslated(view_p[0], view_p[1], view_p[2]);
            back_draw();
        }
        glPopMatrix();

        /* Rotate the world about the position of the ball. */

        glTranslated(+ball_p[0], +ball_p[1], +ball_p[2]);
        glRotated(-game_rz, view_e[2][0], view_e[2][1], view_e[2][2]);
        glRotated(-game_rx, view_e[0][0], view_e[0][1], view_e[0][2]);
        glTranslated(-ball_p[0], -ball_p[1], -ball_p[2]);

        /* Draw the scene. */

        glLightfv(GL_LIGHT0, GL_POSITION, light_p);

        sol_render(fp);

        coin_draw(fp->cv, fp->cc);
        ball_draw(up->r, up->p, up->e);
        goal_draw(-rx, -ry, fp->zv, fp->zc);
        part_draw(-rx, -ry);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

static void game_update_grav(double h[3], const double g[3])
{
    struct s_file *fp = &file;

    double x[3];
    double y[3] = { 0.0, 1.0, 0.0 };
    double z[3];
    double X[16];
    double Z[16];
    double M[16];

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
}

static void game_update_view(double dt)
{
    double *ball_p = file.uv->p;

    double d[3];
    double dy;
    double dz;

    /* Orthonormalize the basis of the view of the ball in its new position. */

    v_sub(view_e[2], view_p, ball_p);

    v_crs(view_e[0], view_e[1], view_e[2]);
    v_crs(view_e[2], view_e[0], view_e[1]);
    v_nrm(view_e[0], view_e[0]);
    v_nrm(view_e[2], view_e[2]);

    /* The current view (dy, dz) approaches the ideal (view_dy, view_dz). */

    v_sub(d, view_p, ball_p);

    dy = v_dot(view_e[1], d);
    dz = v_dot(view_e[2], d);

    dy += (view_dy - dy) * dt * 2.0;
    dz += (view_dz - dz) * dt * 2.0;

    /* Compute the new view position. */

    view_p[0] = view_p[1] = view_p[2] = 0.0;

    v_mad(view_p, ball_p, view_e[0], view_r * dt);
    v_mad(view_p, view_p, view_e[1], dy);
    v_mad(view_p, view_p, view_e[2], dz);
}

static void game_update_time(double dt)
{
    int seconds;

   /* The ticking clock. */

    time -= dt;
    seconds = (int) ceil(time);

    if (0 <= seconds && seconds < ticks && ticks < 99)
    {
        audio_play(AUD_TICK, 1.f);
        ticks = seconds;
    }
}

static int game_update_state(void)
{
    struct s_file *fp = &file;
    int n;

    /* Test for a coin grab and a possible 1UP. */

    if ((n = coin_test(fp->uv, fp->cv, fp->cc)) > 0)
    {
        score += n;

        if (score >= 100)
        {
            score -= 100;
            balls++;
            audio_play(AUD_BALL, 1.f);
        }
        else
            audio_play(AUD_COIN, 1.f);
    }

    /* Test for a goal. */

    if (goal_test(fp->uv, fp->zv, fp->zc))
        return GAME_GOAL;

    /* Test for time-out. */

    if (ticks <= 0)
        return GAME_TIME;

    /* Test for fall-out. */

    if (fp->uv[0].p[1] < -10.0)
        return GAME_FALL;

    return GAME_NONE;
}

/*---------------------------------------------------------------------------*/

/*
 * On  most  hardware, rendering  requires  much  more  computing power  than
 * physics.  Since  physics takes less time  than graphics, it  make sense to
 * detach  the physics update  time step  from the  graphics frame  rate.  By
 * performing multiple physics updates for  each graphics update, we get away
 * with higher quality physics with little impact on overall performance.
 *
 * Toward this  end, we establish a  baseline maximum physics  time step.  If
 * the measured  frame time  exceeds this  maximum, we cut  the time  step in
 * half, and  do two updates.  If THIS  time step exceeds the  maximum, we do
 * four updates.  And  so on.  In this way, the physics  system is allowed to
 * seek an optimal update rate independant of, yet in integral sync with, the
 * graphics frame rate.
 */
int game_update_env(const double g[3], double dt)
{
    struct s_file *fp = &file;

    double h[3];
    double d = 0.0;
    double b = 0.0;
    double t = dt;
    int i, n = 1;

    game_update_grav(h, g);
    part_update_grav(h, dt);

    /* Run the sim. */

    while (t > MAX_DT)
    {
        t /= 2;
        n *= 2;
    }

    for (i = 0; i < n; i++)
        if (b < (d = sol_update(fp, h, t)))
            b = d;

    /* Mix the sound of a ball bounce. */

    if (b > 0.5)
        audio_play(AUD_BUMP, (float) (b - 0.5) * 2.0f);

    game_update_view(dt);
    game_update_time(dt);

    return game_update_state();
}

/*
 * Update the tilt of the environment given the current input.
 */
void game_update_pos(int x, int y)
{
    double bound = 20.0;

    game_rx += 40.0 * y / 500;
    game_rz += 40.0 * x / 500;

    if (game_rx > +bound) game_rx = +bound;
    if (game_rx < -bound) game_rx = -bound;
    if (game_rz > +bound) game_rz = +bound;
    if (game_rz < -bound) game_rz = -bound;
}

/*
 * Update the rotation of the view about the ball.
 */
void game_update_rot(int r)
{
    view_r = r * 5.0;
}

/*
 * Update the position of the camera during a level-intro fly-by.
 */
void game_update_fly(double k)
{
    struct s_file *fp = &file;

    if (fp->pc > 0)
    {
        double p[3];
        double v[3];

        p[0] = p[1] = p[2] = 0.0;

        v_cpy(p, fp->uv[0].p);

        v_mad(p, p, view_e[1], view_dy);
        v_mad(p, p, view_e[2], view_dz);

        v_sub(v, fp->pv[0].p, p);
        v_mad(view_p, p, v, k);
    }
}

/*---------------------------------------------------------------------------*/

static void view_init(void)
{
    game_rx = 0.0;
    game_rz = 0.0;

    view_r  = 0.0;
    view_dy = 4.0;
    view_dz = 6.0;

    view_p[0] =     0.0;
    view_p[1] = view_dy;
    view_p[2] = view_dz;

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

void game_init(const char *s, int t)
{
    view_init();
    ball_init();
    goal_init();
    coin_init();
    part_init();
    text_init();

    sol_load(&file, s);

    ticks = t;
    time  = t;
}

void game_free(void)
{
    sol_free(&file);

    text_free();
    part_free();
    coin_free();
    goal_free();
    ball_free();
}

/*---------------------------------------------------------------------------*/

