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
#include <math.h>

#include "glext.h"
#include "game.h"
#include "vec3.h"
#include "geom.h"
#include "back.h"
#include "part.h"
#include "text.h"
#include "hud.h"
#include "image.h"
#include "audio.h"
#include "solid.h"
#include "level.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

static struct s_file file;

static float clock = 0.f;               /* Clock time                        */

static float game_ix;                   /* Input rotation about X axis       */
static float game_iz;                   /* Input rotation about Z axis       */
static float game_rx;                   /* Floor rotation about X axis       */
static float game_rz;                   /* Floor rotation about Z axis       */

static float view_a;                    /* Ideal view rotation about Y axis  */
static float view_ry;                   /* Angular velocity about Y axis     */
static float view_dc;                   /* Ideal view distance above ball    */
static float view_dp;                   /* Ideal view distance above ball    */
static float view_dz;                   /* Ideal view distance behind ball   */

static float view_c[3];                 /* Current view center               */
static float view_v[3];                 /* Current view vector               */
static float view_p[3];                 /* Current view position             */
static float view_e[3][3];              /* Current view orientation          */

static int   swch_e = 1;                /* Switching enabled flag            */
static int   jump_e = 1;                /* Jumping enabled flag              */
static int   jump_b = 0;                /* Jump-in-progress flag             */
static float jump_dt;                   /* Jump duration                     */
static float jump_p[3];                 /* Jump destination                  */

static GLuint shadow_text;              /* Shadow texture object             */

/*---------------------------------------------------------------------------*/

static void view_init(void)
{
    view_a  = 0.f;
    view_ry = 0.f;

    view_dp = 0.75f;
    view_dc = 0.25f;
    view_dz = 2.0f;

    view_c[0] = 0.f;
    view_c[1] = view_dc;
    view_c[2] = 0.f;

    view_p[0] =     0.f;
    view_p[1] = view_dp;
    view_p[2] = view_dz;

    view_e[0][0] = 1.f;
    view_e[0][1] = 0.f;
    view_e[0][2] = 0.f;
    view_e[1][0] = 0.f;
    view_e[1][1] = 1.f;
    view_e[1][2] = 0.f;
    view_e[2][0] = 0.f;
    view_e[2][1] = 0.f;
    view_e[2][2] = 1.f;
}

void game_init(const char *s, int t)
{
    game_ix = 0.f;
    game_iz = 0.f;
    game_rx = 0.f;
    game_rz = 0.f;

    jump_e = 1;
    jump_b = 0;

    view_init();
    part_init(GOAL_HEIGHT);

    hud_ball_pulse(0.f);
    hud_time_pulse(0.f);
    hud_coin_pulse(0.f);

    sol_load(&file, s, config_get(CONFIG_TEXTURES), config_get(CONFIG_SHADOW));
    clock = (float) t;

    shadow_text = make_image_from_file(NULL, NULL, NULL, NULL, IMG_SHADOW);

    if (config_get(CONFIG_SHADOW) == 2)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
}

void game_free(void)
{
    if (glIsTexture(shadow_text))
        glDeleteTextures(1, &shadow_text);

    sol_free(&file);
    part_free();
}

/*---------------------------------------------------------------------------*/

int curr_clock(void)
{
    return (int) (clock * 100.f);
}

char *curr_intro(void)
{
    return (file.ac > 0) ? file.av : NULL;
}

/*---------------------------------------------------------------------------*/

static void game_draw_balls(const struct s_file *fp)
{
    float M[16];

    m_basis(M, fp->uv[0].e[0], fp->uv[0].e[1], fp->uv[0].e[2]);

    glPushMatrix();
    {
        glTranslatef(fp->uv[0].p[0],
                     fp->uv[0].p[1] + BALL_FUDGE,
                     fp->uv[0].p[2]);
        glMultMatrixf(M);
        glScalef(fp->uv[0].r,
                 fp->uv[0].r,
                 fp->uv[0].r);

        glColor4fv(c_play[0]);

        ball_draw();
    }
    glPopMatrix();
}

static void game_draw_coins(const struct s_file *fp)
{
    float r = 360.f * SDL_GetTicks() / 1000.f;
    int ci;

    coin_push();
    {
        for (ci = 0; ci < fp->cc; ci++)
            if (fp->cv[ci].n > 0)
            {
                glPushMatrix();
                {
                    glTranslatef(fp->cv[ci].p[0],
                                 fp->cv[ci].p[1],
                                 fp->cv[ci].p[2]);
                    glRotatef(r, 0.0f, 1.0f, 0.0f);
                    coin_draw(fp->cv[ci].n, r);
                }
                glPopMatrix();
            }
    }
    coin_pull();
}

static void game_draw_goals(const struct s_file *fp, float rx, float ry)
{
    int zi;

    for (zi = 0; zi < fp->zc; zi++)
    {
        glPushMatrix();
        {
            glTranslatef(fp->zv[zi].p[0],
                         fp->zv[zi].p[1],
                         fp->zv[zi].p[2]);

            part_draw_goal(rx, ry, fp->zv[zi].r);

            glScalef(fp->zv[zi].r, 1.f, fp->zv[zi].r);
            goal_draw();
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
            glTranslatef(fp->jv[ji].p[0],
                         fp->jv[ji].p[1],
                         fp->jv[ji].p[2]);

            glScalef(fp->jv[ji].r, 1.f, fp->jv[ji].r);
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
            glTranslatef(fp->xv[xi].p[0],
                         fp->xv[xi].p[1],
                         fp->xv[xi].p[2]);

            glScalef(fp->xv[xi].r, 1.f, fp->xv[xi].r);
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
    const float *ball_p = fp->uv->p;
    const float  ball_r = fp->uv->r;

    if (config_get(CONFIG_SHADOW))
    {
        glActiveTexture(GL_TEXTURE1);
        glMatrixMode(GL_TEXTURE);
        {
            float k = 0.5f / ball_r;

            glEnable(GL_TEXTURE_2D);

            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glBindTexture(GL_TEXTURE_2D, shadow_text);

            glLoadIdentity();
            glTranslatef(0.5f - ball_p[0] * k,
                         0.5f - ball_p[2] * k, 0.f);
            glScalef(k, k, 1.f);
        }
        glMatrixMode(GL_MODELVIEW);
        glActiveTexture(GL_TEXTURE0);
    }
}

static void game_clr_shadow(void)
{
    if (config_get(CONFIG_SHADOW))
    {
        glActiveTexture(GL_TEXTURE1);
        {
            glDisable(GL_TEXTURE_2D);
        }
        glActiveTexture(GL_TEXTURE0);
    }
}

/*---------------------------------------------------------------------------*/

static void game_refl_all(void)
{
    const float *ball_p = file.uv->p;
    
    glPushMatrix();
    {
        /* Rotate the environment about the position of the ball. */

        glTranslatef(+ball_p[0], +ball_p[1], +ball_p[2]);
        glRotatef(-game_rz, view_e[2][0], view_e[2][1], view_e[2][2]);
        glRotatef(-game_rx, view_e[0][0], view_e[0][1], view_e[0][2]);
        glTranslatef(-ball_p[0], -ball_p[1], -ball_p[2]);

        /* Draw the floor. */

        sol_refl(&file);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

static void game_draw_light(void)
{
    const float light_p[2][4] = {
        { -8.0f, +32.0f, -8.0f, 1.0f },
        { +8.0f, +32.0f, +8.0f, 1.0f },
    };
    const float light_c[2][4] = {
        { 1.0f, 0.8f, 0.8f, 1.0f },
        { 0.8f, 1.0f, 0.8f, 1.0f },
    };

    /* Configure the lighting. */

    glEnable(GL_LIGHTING);

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_p[0]);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_c[0]);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_c[0]);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, light_p[1]);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_c[1]);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_c[1]);
}

static void game_draw_all(int pose, float rx, float ry, int d)
{
    const float *ball_p = file.uv->p;
    
    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_COLOR_BUFFER_BIT);
    {
        glPushMatrix();
        {
            if (d < 0)
            {
                glRotatef(game_rz, view_e[2][0], view_e[2][1], view_e[2][2]);
                glRotatef(game_rx, view_e[0][0], view_e[0][1], view_e[0][2]);
            }

            game_clr_shadow();
            back_draw(d, 0);
        }
        glPopMatrix();

        glPushMatrix();
        {
            /* Rotate the environment about the position of the ball. */

            glTranslatef(+ball_p[0], +ball_p[1] * d, +ball_p[2]);
            glRotatef(-game_rz * d, view_e[2][0], view_e[2][1], view_e[2][2]);
            glRotatef(-game_rx * d, view_e[0][0], view_e[0][1], view_e[0][2]);
            glTranslatef(-ball_p[0], -ball_p[1] * d, -ball_p[2]);

            if (d < 0)
            {
                GLdouble e[4];

                e[0] = +0;
                e[1] = +1;
                e[2] = +0;
                e[3] = -0.001;

                glEnable(GL_CLIP_PLANE0);
                glClipPlane(GL_CLIP_PLANE0, e);
            }

            /* Draw the floor. */

            if (pose == 0) game_set_shadow(&file);
            sol_draw(&file, config_get(CONFIG_SHADOW));
            if (pose == 0) game_clr_shadow();

            /* Draw the game elements. */

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (pose == 0)
            {
                part_draw_coin(-rx * d, -ry);
                game_draw_coins(&file);
                game_draw_balls(&file);
            }
            game_draw_goals(&file, -rx * d, -ry);
            game_draw_jumps(&file);
            game_draw_swchs(&file);

            glDisable(GL_CLIP_PLANE0);
        }
        glPopMatrix();
    }
    glPopAttrib();
    glPopAttrib();
}

void game_draw(int pose, float dy)
{
    float fov = FOV;

    if (jump_b) fov *= 2.f * fabsf(jump_dt - 0.5);

    config_push_persp(fov, 0.1f, FAR_DIST);
    glPushMatrix();
    {
        float v[3], rx, ry;

        /* Compute and apply the view. */

        v_sub(v, view_c, view_p);

        rx = V_DEG(fatan2f(-v[1], fsqrtf(v[0] * v[0] + v[2] * v[2])));
        ry = V_DEG(fatan2f(+v[0], -v[2])) + dy;

        /*
        if (game_rx > 0 && rx < game_rx) rx = game_rx;
        */

        glTranslatef(0.f, 0.f, -v_len(v));
        glRotatef(rx, 1.f, 0.f, 0.f);
        glRotatef(ry, 0.f, 1.f, 0.f);
        glTranslatef(-view_c[0], -view_c[1], -view_c[2]);

        if (config_get(CONFIG_REFLECTION))
        {
            /* Draw the mirror into the stencil buffer. */

            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
            glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

            game_refl_all();

            /* Draw the scene reflected. */

            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);

            glFrontFace(GL_CW);
            glPushMatrix();
            {
                glScalef(+1.f, -1.f, +1.f);

                game_draw_light();
                game_draw_all(pose, rx, ry, -1);
            }
            glPopMatrix();
            glFrontFace(GL_CCW);

            glDisable(GL_STENCIL_TEST);
        }

        /* Draw the scene normally. */

        game_draw_light();
        game_draw_all(pose, rx, ry, +1);
    }
    glPopMatrix();
    config_pop_matrix();
}

/*---------------------------------------------------------------------------*/

static void game_update_grav(float h[3], const float g[3])
{
    struct s_file *fp = &file;

    float x[3];
    float y[3] = { 0.f, 1.f, 0.f };
    float z[3];
    float X[16];
    float Z[16];
    float M[16];

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

static void game_update_view(float dt)
{
    const float y[3] = { 0.f, 1.f, 0.f };

    float dx = view_ry * dt * 5.f;
    float k;
    float e[3];
    float s = 4.f * dt;

    view_a += view_ry * dt * 90.f;

    /* Center the view about the ball. */

    v_cpy(view_c, file.uv->p);
    v_inv(view_v, file.uv->v);

    switch (config_get(CONFIG_CAMERA))
    {
    case 1:
        /* Camera 1:  Viewpoint chases the ball position. */

        v_sub(view_e[2], view_p, view_c);
        break;

    case 2:
        /* Camera 2: View vector is given by view angle. */

        view_e[2][0] = fsinf(V_RAD(view_a));
        view_e[2][1] = 0.f;
        view_e[2][2] = fcosf(V_RAD(view_a));

        dx = 0.0f;
        s  = 8.0f * dt;
        break;

    default:
        /* Default: View vector approaches the ball velocity vector. */

        v_mad(e, view_v, y, v_dot(view_v, y));
        v_inv(e, e);

        k = v_dot(view_v, view_v);

        v_sub(view_e[2], view_p, view_c);
        v_mad(view_e[2], view_e[2], view_v, k * dt * 0.25f);

        break;
    }

    /* Orthonormalize the basis of the view in its new position. */

    v_crs(view_e[0], view_e[1], view_e[2]);
    v_crs(view_e[2], view_e[0], view_e[1]);
    v_nrm(view_e[0], view_e[0]);
    v_nrm(view_e[2], view_e[2]);

    /* Compute the new view position. */

    v_cpy(view_p, file.uv->p);
    v_mad(view_p, view_p, view_e[0], dx);
    v_mad(view_p, view_p, view_e[1], view_dp);
    v_mad(view_p, view_p, view_e[2], view_dz);

    v_cpy(view_c, file.uv->p);
    v_mad(view_c, view_c, view_e[1], view_dc);

    view_a = V_DEG(fatan2f(view_e[2][0], view_e[2][2]));
}

static void game_update_time(float dt, int b)
{
    int tick = (int) floor(clock);
    int tock = (int) floor(clock * 2);

   /* The ticking clock. */

    if (b)
    {
        if (clock < 600.f)
            clock -= dt;
        if (clock < 0.f)
            clock = 0.f;

        if (0 < tick && tick <= 10 && tick == (int) ceil(clock))
        {
            audio_play(AUD_TICK, 1.f);
            hud_time_pulse(1.50);
        }
        else if (0 < tock && tock <= 10 && tock == (int) ceil(clock * 2))
        {
            audio_play(AUD_TOCK, 1.f);
            hud_time_pulse(1.25);
        }
    }
}

static int game_update_state(void)
{
    struct s_file *fp = &file;
    float p[3];
    float c[3];
    int n, e = swch_e;

    /* Test for a coin grab and a possible 1UP. */

    if ((n = sol_coin_test(fp, p, COIN_RADIUS)) > 0)
    {
        coin_color(c, n);
        part_burst(p, c);
        level_score(n);
    }

    /* Test for a switch. */

    if ((swch_e = sol_swch_test(fp, swch_e, 0)) != e && e)
        audio_play(AUD_SWITCH, 1.f);

    /* Test for a jump. */

    if (jump_e == 1 && jump_b == 0 && sol_jump_test(fp, jump_p, 0) == 1)
    {
        jump_b  = 1;
        jump_e  = 0;
        jump_dt = 0.f;
        
        audio_play(AUD_JUMP, 1.f);
    }
    if (jump_e == 0 && jump_b == 0 &&  sol_jump_test(fp, jump_p, 0) == 0)
        jump_e = 1;

    /* Test for a goal. */

    if (sol_goal_test(fp, p, 0))
        return GAME_GOAL;

    /* Test for time-out. */

    if (clock <= 0.f)
        return GAME_TIME;

    /* Test for fall-out. */

    if (fp->uv[0].p[1] < -20.f)
        return GAME_FALL;

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

int game_step(const float g[3], float dt, int bt)
{
    struct s_file *fp = &file;

    float h[3];
    float d = 0.f;
    float b = 0.f;
    float t;
    int i, n = 1;

    t = dt;

    /* Smooth jittery or discontinuous input. */

    if (t < RESPONSE)
    {
        game_rx += (game_ix - game_rx) * t / RESPONSE;
        game_rz += (game_iz - game_rz) * t / RESPONSE;
    }
    else
    {
        game_rx = game_ix;
        game_rz = game_iz;
    }

    game_update_grav(h, g);
    part_step(h, t);

    if (jump_b)
    {
        jump_dt += t;

        /* Handle a jump. */

        if (0.5 < jump_dt)
        {
            fp->uv[0].p[0] = jump_p[0];
            fp->uv[0].p[1] = jump_p[1];
            fp->uv[0].p[2] = jump_p[2];
        }
        if (1.f < jump_dt)
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
            if (b < (d = sol_step(fp, h, t, 0, NULL)))
                b = d;

        /* Mix the sound of a ball bounce. */

        if (b > 0.5)
            audio_play(AUD_BUMP, (b - 0.5f) * 2.0f);
    }

    game_update_view(dt);
    game_update_time(dt, bt);

    return game_update_state();
}

/*---------------------------------------------------------------------------*/

void game_set_x(int k)
{
    game_ix = -20.f * k / JOY_MAX;
}

void game_set_z(int k)
{
    game_iz = +20.f * k / JOY_MAX;
}

void game_set_pos(int x, int y)
{
    float bound = 20.f;

    game_ix += 40.f * y / config_get(CONFIG_MOUSE_SENSE);
    game_iz += 40.f * x / config_get(CONFIG_MOUSE_SENSE);

    if (game_ix > +bound) game_ix = +bound;
    if (game_ix < -bound) game_ix = -bound;
    if (game_iz > +bound) game_iz = +bound;
    if (game_iz < -bound) game_iz = -bound;
}

void game_set_rot(int r)
{
    view_ry = (float) r;
}

/*---------------------------------------------------------------------------*/

void game_set_fly(float k)
{
    struct s_file *fp = &file;

    float  x[3] = { 1.f, 0.f, 0.f };
    float  y[3] = { 0.f, 1.f, 0.f };
    float  z[3] = { 0.f, 0.f, 1.f };
    float c0[3] = { 0.f, 0.f, 0.f };
    float p0[3] = { 0.f, 0.f, 0.f };
    float c1[3] = { 0.f, 0.f, 0.f };
    float p1[3] = { 0.f, 0.f, 0.f };
    float  v[3];

    v_cpy(view_e[0], x);
    v_cpy(view_e[1], y);
    v_cpy(view_e[2], z);

    /* k = 0.0 view is at the ball. */

    if (fp->uc > 0)
    {
        v_cpy(c0, fp->uv[0].p);
        v_cpy(p0, fp->uv[0].p);
    }

    v_mad(p0, p0, y, view_dp);
    v_mad(p0, p0, z, view_dz);
    v_mad(c0, c0, y, view_dc);

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

/*---------------------------------------------------------------------------*/

int game_put(FILE *fout)
{
    return (float_put(fout, &game_rx)  &&
            float_put(fout, &game_rz)  &&
            vector_put(fout, view_c)    &&
            vector_put(fout, view_p)    &&
            vector_put(fout, view_e[0]) &&
            vector_put(fout, view_e[1]) &&
            vector_put(fout, view_e[2]) &&
            sol_put(fout, &file));
}

int game_get(FILE *fin)
{
    return (float_get(fin, &game_rx)  &&
            float_get(fin, &game_rz)  &&
            vector_get(fin, view_c)    &&
            vector_get(fin, view_p)    &&
            vector_get(fin, view_e[0]) &&
            vector_get(fin, view_e[1]) &&
            vector_get(fin, view_e[2]) &&
            sol_get(fin, &file));
}

/*---------------------------------------------------------------------------*/
