/*   
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERPUTT is  free software; you can redistribute  it and/or modify
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
#include <math.h>

#include "glext.h"
#include "game.h"
#include "vec3.h"
#include "geom.h"
#include "text.h"
#include "back.h"
#include "hud.h"
#include "image.h"
#include "audio.h"
#include "solid.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

static struct s_file file;
static int           ball;

static double view_a;                   /* Ideal view rotation about Y axis  */
static double view_m;
static double view_ry;                  /* Angular velocity about Y axis     */
static double view_dy;                  /* Ideal view distance above ball    */
static double view_dz;                  /* Ideal view distance behind ball   */

static double view_c[3];                /* Current view center               */
static double view_v[3];                /* Current view vector               */
static double view_p[3];                /* Current view position             */
static double view_e[3][3];             /* Current view orientation          */

static double jump_e = 1;               /* Jumping enabled flag              */
static double jump_b = 0;               /* Jump-in-progress flag             */
static double jump_dt;                  /* Jump duration                     */
static double jump_p[3];                /* Jump destination                  */

static GLuint shadow_text;              /* Shadow texture object             */

/*---------------------------------------------------------------------------*/

static void view_init(void)
{
    view_a  = 0.0;
    view_m  = 0.0;
    view_ry = 0.0;
    view_dy = 3.0;
    view_dz = 5.0;

    view_c[0] = 0.0;
    view_c[1] = 0.0;
    view_c[2] = 0.0;

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

void game_init(const char *s)
{
    jump_e = 1;
    jump_b = 0;

    view_init();
    sol_load(&file, s, config_text());

    shadow_text = make_image_from_file(NULL, NULL, IMG_SHADOW);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

void game_free(void)
{
    if (glIsTexture(shadow_text))
        glDeleteTextures(1, &shadow_text);

    sol_free(&file);
}

/*---------------------------------------------------------------------------*/

static void game_draw_vect_prim(const struct s_file *fp, GLenum mode)
{
    double p[3];
    double x[3];
    double z[3];
    double r;

    v_cpy(p, fp->uv[ball].p);
    v_cpy(x, view_e[0]);
    v_cpy(z, view_e[2]);

    r = fp->uv[ball].r;
    
    glBegin(mode);
    {
        glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
        glVertex3d(p[0] - x[0] * r,
                   p[1] - x[1] * r,
                   p[2] - x[2] * r);

        glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
        glVertex3d(p[0] + z[0] * view_m,
                   p[1] + z[1] * view_m,
                   p[2] + z[2] * view_m);

        glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
        glVertex3d(p[0] + x[0] * r,
                   p[1] + x[1] * r,
                   p[2] + x[2] * r);
    }
    glEnd();
}

static void game_draw_vect(const struct s_file *fp)
{
    if (view_m > 0.0)
    {
        glPushAttrib(GL_TEXTURE_BIT);
        glPushAttrib(GL_POLYGON_BIT);
        glPushAttrib(GL_LIGHTING_BIT);
        glPushAttrib(GL_DEPTH_BUFFER_BIT);
        {
            glEnable(GL_COLOR_MATERIAL);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glDepthMask(GL_FALSE);

            glEnable(GL_DEPTH_TEST);
            game_draw_vect_prim(fp, GL_TRIANGLES);

            glDisable(GL_DEPTH_TEST);
            game_draw_vect_prim(fp, GL_LINE_STRIP);
        }
        glPopAttrib();
        glPopAttrib();
        glPopAttrib();
        glPopAttrib();
    }
}

static void game_draw_balls(const struct s_file *fp)
{
    double M[16];
    int ui;

    for (ui = fp->uc - 1; ui > 0; ui--)
    {
        if (ui == ball)
        {
            glPushMatrix();
            {
                m_basis(M, fp->uv[ui].e[0], fp->uv[ui].e[1], fp->uv[ui].e[2]);

                glTranslated(fp->uv[ui].p[0],
                             fp->uv[ui].p[1] + BALL_FUDGE,
                             fp->uv[ui].p[2]);
                glMultMatrixd(M);
                glScaled(fp->uv[ui].r,
                         fp->uv[ui].r,
                         fp->uv[ui].r);

                glColor4fv(c_play[ui]);

                ball_draw();
            }
            glPopMatrix();
        }
        else
        {
            glPushMatrix();
            {
                glTranslated(fp->uv[ui].p[0],
                             fp->uv[ui].p[1] - fp->uv[ui].r + BALL_FUDGE,
                             fp->uv[ui].p[2]);
                glScaled(fp->uv[ui].r,
                         fp->uv[ui].r,
                         fp->uv[ui].r);

                glColor4f(c_play[ui][0],
                          c_play[ui][1],
                          c_play[ui][2], 0.5f);

                mark_draw();
            }
            glPopMatrix();
        }
    }
}

static void game_draw_goals(const struct s_file *fp, double rx, double ry)
{
    int zi;

    for (zi = 0; zi < fp->zc; zi++)
    {
        glPushMatrix();
        {
            glTranslated(fp->zv[zi].p[0],
                         fp->zv[zi].p[1],
                         fp->zv[zi].p[2]);
            flag_draw();
        }
        glPopMatrix();
    }
}

static void game_draw_jumps(const struct s_file *fp)
{
    int ji;

    for (ji = 0; ji < fp->jc; ji++)
    {
        glPushMatrix();
        {
            glTranslated(fp->jv[ji].p[0],
                         fp->jv[ji].p[1],
                         fp->jv[ji].p[2]);

            glScaled(fp->jv[ji].r, 1.0, fp->jv[ji].r);
            jump_draw();
        }
        glPopMatrix();
    }
}

static void game_draw_swchs(const struct s_file *fp)
{
    int xi;

    for (xi = 0; xi < fp->xc; xi++)
    {
        glPushMatrix();
        {
            glTranslated(fp->xv[xi].p[0],
                         fp->xv[xi].p[1],
                         fp->xv[xi].p[2]);

            glScaled(fp->xv[xi].r, 1.0, fp->xv[xi].r);
            swch_draw(fp->xv[xi].f);
        }
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/

/*
 * A note about lighting and shadow: technically speaking, it's wrong.
 * The  light  position  and   shadow  projection  behave  as  if  the
 * light-source rotates with the  floor.  However, the skybox does not
 * rotate, thus the light should also remain stationary.
 *
 * The  correct behavior  would eliminate  a significant  3D  cue: the
 * shadow of  the ball indicates  the ball's position relative  to the
 * floor even  when the ball is  in the air.  This  was the motivating
 * idea  behind the  shadow  in  the first  place,  so correct  shadow
 * projection would only magnify the problem.
 */

static void game_set_shadow(const struct s_file *fp)
{
    const double *ball_p = fp->uv[ball].p;
    const double  ball_r = fp->uv[ball].r;

    if (glActiveTexture)
    {
        glActiveTexture(GL_TEXTURE1);
        glMatrixMode(GL_TEXTURE);
        {
            double k = 0.5 / ball_r;

            glEnable(GL_TEXTURE_2D);

            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glBindTexture(GL_TEXTURE_2D, shadow_text);

            glLoadIdentity();
            glTranslated(0.5 - ball_p[0] * k,
                         0.5 - ball_p[2] * k, 0.0);
            glScaled(k, k, 1.0);
        }
        glMatrixMode(GL_MODELVIEW);
        glActiveTexture(GL_TEXTURE0);
    }
}

static void game_clr_shadow(void)
{
    if (glActiveTexture)
    {
        glActiveTexture(GL_TEXTURE1);
        {
            glDisable(GL_TEXTURE_2D);
        }
        glActiveTexture(GL_TEXTURE0);
    }
}

/*---------------------------------------------------------------------------*/

void game_draw(int pose)
{
    const float light_p[4] = { 8.0, 32.0, 8.0, 1.0 };

    const struct s_file *fp = &file;
    
    double fov = FOV;

    if (jump_b) fov *= 2.0 * fabs(jump_dt - 0.5);

    config_push_persp(fov, 0.1, 300.0);
    glPushAttrib(GL_LIGHTING_BIT);
    glPushMatrix();
    {
        double v[3], rx, ry;

        v_sub(v, view_c, view_p);

        rx = V_DEG(atan2(-v[1], sqrt(v[0] * v[0] + v[2] * v[2])));
        ry = V_DEG(atan2(+v[0], -v[2]));

        glTranslated(0.0, 0.0, -v_len(v));
        glRotated(rx, 1.0, 0.0, 0.0);
        glRotated(ry, 0.0, 1.0, 0.0);
        glTranslated(-view_c[0], -view_c[1], -view_c[2]);

        /* Center the skybox about the position of the camera. */

        glPushMatrix();
        {
            glTranslated(view_p[0], view_p[1], view_p[2]);
            back_draw(0, 0);
        }
        glPopMatrix();

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, light_p);

        /* Draw the floor. */

        if (pose == 0) game_set_shadow(fp);
        sol_draw(fp);
        if (pose == 0) game_clr_shadow();

        /* Draw the game elements. */

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (pose == 0)
        {
            game_draw_balls(fp);
            game_draw_vect(fp);
        }

        game_draw_goals(fp, -rx, -ry);
        game_draw_jumps(fp);
        game_draw_swchs(fp);
    }
    glPopMatrix();
    glPopAttrib();
    config_pop_matrix();
}

/*---------------------------------------------------------------------------*/

void game_update_view(double dt)
{
    const double y[3] = { 0.0, 1.0, 0.0 };

    double dy;
    double dz;
    double k;
    double e[3];
    double d[3];
    double s = 2.0 * dt;

    /* Center the view about the ball. */

    v_cpy(view_c, file.uv[ball].p);
    v_inv(view_v, file.uv[ball].v);

    switch (config_view())
    {
    case 2:
        /* Camera 2: View vector is given by view angle. */

        view_e[2][0] = sin(V_RAD(view_a));
        view_e[2][1] = 0.0;
        view_e[2][2] = cos(V_RAD(view_a));

        s = 1.0;
        break;

    default:
        /* View vector approaches the ball velocity vector. */

        v_mad(e, view_v, y, v_dot(view_v, y));
        v_inv(e, e);

        k = v_dot(view_v, view_v);

        v_sub(view_e[2], view_p, view_c);
        v_mad(view_e[2], view_e[2], view_v, k * dt * 0.1);
    }

    /* Orthonormalize the basis of the view in its new position. */

    v_crs(view_e[0], view_e[1], view_e[2]);
    v_crs(view_e[2], view_e[0], view_e[1]);
    v_nrm(view_e[0], view_e[0]);
    v_nrm(view_e[2], view_e[2]);

    /* The current view (dy, dz) approaches the ideal (view_dy, view_dz). */

    v_sub(d, view_p, view_c);

    dy = v_dot(view_e[1], d);
    dz = v_dot(view_e[2], d);

    dy += (view_dy - dy) * s;
    dz += (view_dz - dz) * s;

    /* Compute the new view position. */

    view_p[0] = view_p[1] = view_p[2] = 0.0;

    v_mad(view_p, view_c, view_e[1], dy);
    v_mad(view_p, view_p, view_e[2], dz);

    view_a = V_DEG(atan2(view_e[2][0], view_e[2][2]));
}

static int game_update_state(double dt)
{
    static double t = 0.0;

    struct s_file *fp = &file;
    double p[3];

    if (dt > 0.0)
        t += dt;
    else
        t = 0.0;

    /* Test for a jump. */

    if (jump_e == 1 && jump_b == 0 && sol_jump_test(fp, jump_p, ball) == 1)
    {
        jump_b  = 1;
        jump_e  = 0;
        jump_dt = 0.0;
        
        audio_play(AUD_JUMP, 1.f);
    }
    if (jump_e == 0 && jump_b == 0 &&  sol_jump_test(fp, jump_p, ball) == 0)
        jump_e = 1;

    /* Test for fall-out. */

    if (fp->uv[ball].p[1] < -10.0)
        return GAME_FALL;

    /* Test for a goal or stop. */

    if (t > 1.0)
    {
        t = 0.0;

        if (sol_goal_test(fp, p, ball))
            return GAME_GOAL;
        else
            return GAME_STOP;
    }

    return GAME_NONE;
}

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

int game_step(const double g[3], double dt)
{
    struct s_file *fp = &file;

    static double s = 0.0;
    static double t = 0.0;

    double d = 0.0;
    double b = 0.0;
    double st = 0.0;
    int i, n = 1, m = 0;

    s = (7.0 * s + dt) / 8.0;
    t = s;

    if (jump_b)
    {
        jump_dt += dt;

        /* Handle a jump. */

        if (0.5 < jump_dt)
        {
            fp->uv[ball].p[0] = jump_p[0];
            fp->uv[ball].p[1] = jump_p[1];
            fp->uv[ball].p[2] = jump_p[2];
        }
        if (1.0 < jump_dt)
            jump_b = 0;
    }
    else
    {
        /* Run the sim. */

        while (t > MAX_DT && n < MAX_DN)
        {
            t /= 2;
            n *= 2;
        }

        for (i = 0; i < n; i++)
        {
            d = sol_step(fp, g, t, ball, &m);
            
            if (b < d)
                b = d;
            if (m)
                st += t;
        }

        /* Mix the sound of a ball bounce. */

        if (b > 0.5)
            audio_play(AUD_BUMP, (float) (b - 0.5) * 2.0f);
    }

    game_update_view(dt);
    return game_update_state(st);
}

void game_putt(void)
{
    /*
     * HACK: The BALL_FUDGE here  guarantees that a putt doesn't drive
     * the ball  too directly down  toward a lump,  triggering rolling
     * friction too early and stopping the ball prematurely.
     */

    file.uv[ball].v[0] = -4.0 * view_e[2][0] * view_m;
    file.uv[ball].v[1] = -4.0 * view_e[2][1] * view_m + BALL_FUDGE;
    file.uv[ball].v[2] = -4.0 * view_e[2][2] * view_m;

    view_m = 0.0;
}

/*---------------------------------------------------------------------------*/

void game_set_rot(int d)
{
    view_a += (double) (30.0 * d) / config_sens();
}

void game_clr_mag(void)
{
    view_a = 0.0;
    view_m = 1.0;
}

void game_set_mag(int d)
{
    view_m -= (double) (1.0 * d) / config_sens();

    if (view_m < 0.25)
        view_m = 0.25;
}

void game_set_fly(double k)
{
    struct s_file *fp = &file;

    double  x[3] = { 1.0, 0.0, 0.0 };
    double  y[3] = { 0.0, 1.0, 0.0 };
    double  z[3] = { 0.0, 0.0, 1.0 };
    double c0[3] = { 0.0, 0.0, 0.0 };
    double p0[3] = { 0.0, 0.0, 0.0 };
    double c1[3] = { 0.0, 0.0, 0.0 };
    double p1[3] = { 0.0, 0.0, 0.0 };
    double  v[3];

    v_cpy(view_e[0], x);
    v_cpy(view_e[1], y);
    v_cpy(view_e[2], z);

    /* k = 0.0 view is at the ball. */

    if (fp->uc > 0)
    {
        v_cpy(c0, fp->uv[ball].p);
        v_cpy(p0, fp->uv[ball].p);
    }

    v_mad(p0, p0, y, view_dy);
    v_mad(p0, p0, z, view_dz);

    /* k = +1.0 view is s_view 0 */

    if (k >= 0 && fp->wc > 0)
    {
        v_cpy(p1, fp->wv[0].p);
        v_cpy(c1, fp->wv[0].q);
    }

    /* k = -1.0 view is s_view 1 */

    if (k <= 0 && fp->wc > 1)
    {
        v_cpy(p1, fp->wv[1].p);
        v_cpy(c1, fp->wv[1].q);
    }

    /* Interpolate the views. */

    v_sub(v, p1, p0);
    v_mad(view_p, p0, v, k * k);

    v_sub(v, c1, c0);
    v_mad(view_c, c0, v, k * k);

    /* Orthonormalize the view basis. */

    v_sub(view_e[2], view_p, view_c);
    v_crs(view_e[0], view_e[1], view_e[2]);
    v_crs(view_e[2], view_e[0], view_e[1]);
    v_nrm(view_e[0], view_e[0]);
    v_nrm(view_e[2], view_e[2]);
}

void game_ball(int i)
{
    int ui;

    ball = i;

    jump_e = 1;
    jump_b = 0;

    for (ui = 0; ui < file.uc; ui++)
    {
        file.uv[ui].v[0] = 0.0;
        file.uv[ui].v[1] = 0.0;
        file.uv[ui].v[2] = 0.0;

        file.uv[ui].w[0] = 0.0;
        file.uv[ui].w[1] = 0.0;
        file.uv[ui].w[2] = 0.0;
    }
}

void game_get_pos(double p[3], double e[3][3])
{
    v_cpy(p,    file.uv[ball].p);
    v_cpy(e[0], file.uv[ball].e[0]);
    v_cpy(e[1], file.uv[ball].e[1]);
    v_cpy(e[2], file.uv[ball].e[2]);
}

void game_set_pos(double p[3], double e[3][3])
{
    v_cpy(file.uv[ball].p,    p);
    v_cpy(file.uv[ball].e[0], e[0]);
    v_cpy(file.uv[ball].e[1], e[1]);
    v_cpy(file.uv[ball].e[2], e[2]);
}

/*---------------------------------------------------------------------------*/

