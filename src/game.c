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

#define MAX_DT  0.01666666             /* Maximum physics update cycle       */
#define MAX_DN 16                      /* Maximum subdivisions of dt         */
#define FOV    45.0                    /* Field of view                      */

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

#define IMG_SHADOW "data/png/shadow.png"

static GLuint shadow_text;

/*---------------------------------------------------------------------------*/

#define STR_BALLS "Balls"
#define STR_COINS "Coins"

static int large_w, large_h;
static int small_w, small_h;
static int coins_w, coins_h;
static int balls_w, balls_h;
static int pad_w;

static GLuint large_text[10], large_list[10];
static GLuint small_text[10], small_list[10];

static GLuint balls_text, balls_list;
static GLuint coins_text, coins_list;

void hud_init(void)
{
    const GLfloat c0[3] = { 1.0f, 1.0f, 0.0f };
    const GLfloat c1[3] = { 1.0f, 0.5f, 0.0f };
    const GLfloat c2[3] = { 1.0f, 1.0f, 1.0f };

    char buf[4];
    int i;

    text_size("0",       TXT_MED, &small_w, &small_h);
    text_size("0",       TXT_LRG,  &large_w, &large_h);
    text_size(STR_BALLS, TXT_SML,  &balls_w, &balls_h);
    text_size(STR_COINS, TXT_SML,  &coins_w, &coins_h);
    pad_w = small_w / 2;

    for (i = 0; i < 10; i++)
    {
        sprintf(buf, "%d", i);

        small_text[i] = make_text(buf, TXT_MED);
        small_list[i] = make_list(buf, TXT_MED, c0, c1);

        large_text[i] = make_text(buf, TXT_LRG);
        large_list[i] = make_list(buf, TXT_LRG, c0, c1);
    }

    balls_text = make_text(STR_BALLS, TXT_SML);
    balls_list = make_list(STR_BALLS, TXT_SML, c2, c2);

    coins_text = make_text(STR_COINS, TXT_SML);
    coins_list = make_list(STR_COINS, TXT_SML, c2, c2);
}

void hud_free(void)
{
    int i;

    glDeleteTextures(1, &coins_text);
    glDeleteLists(coins_list, 1);

    glDeleteTextures(1, &balls_text);
    glDeleteLists(balls_list, 1);

    for (i = 0; i < 10; i++)
    {
        glDeleteTextures(1, large_text + i);
        glDeleteLists(large_list[i], 1);

        glDeleteTextures(1, small_text + i);
        glDeleteLists(small_list[i], 1);
    }
}

/*---------------------------------------------------------------------------*/

static void game_render_small(int d, int x, int y)
{
    glPushMatrix();
    {
        glTranslatef(x, y, 0);

        glBindTexture(GL_TEXTURE_2D, small_text[d]);
        glCallList(small_list[d]);
    }
    glPopMatrix();
}

static void game_render_large(int d, int x, int y)
{
    glPushMatrix();
    {
        glTranslatef(x, y, 0);

        glBindTexture(GL_TEXTURE_2D, large_text[d]);
        glCallList(large_list[d]);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

void game_render_fps(void)
{
    static int fps   = 0;
    static int then  = 0;
    static int count = 0;

    if (count > 10)
    {
        int now = SDL_GetTicks();

        fps   = count * 1000 / (now - then);
        then  = now;
        count = 0;
    }
    else count++;

    if (main_fps)
    {
        int a = (fps / 100);
        int b = (fps % 100) / 10;
        int c = (fps % 100) % 10;

        game_render_small(a, 0 * small_w, main_height - small_h);
        game_render_small(b, 1 * small_w, main_height - small_h);
        game_render_small(c, 2 * small_w, main_height - small_h);
    }
}

static void game_render_labels(void)
{
    const int C = main_width / 2;
    const int W = main_width;

    glDisable(GL_TEXTURE_2D);
    {
        const int a = 2 * pad_w + 2 * small_w + coins_w;
        const int b =     pad_w +     large_w;

        glColor4f(0.0f, 0.0f, 0.0f, 0.5f);

        glRecti(C - b, 0, C + b, large_h);
        glRecti(0,     0, a,     small_h);
        glRecti(W - a, 0, W,     small_h);
    }
    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    {
        glTranslatef(pad_w, (small_h - balls_h) / 2, 0);

        glBindTexture(GL_TEXTURE_2D, balls_text);
        glCallList(balls_list);
    }
    glPopMatrix();

    glPushMatrix();
    {
        glTranslatef(main_width - coins_w - pad_w, (small_h - coins_h) / 2, 0);

        glBindTexture(GL_TEXTURE_2D, coins_text);
        glCallList(coins_list);
    }
    glPopMatrix();
}

void game_render_hud(void)
{
    const int w = main_width;
    const int h = main_height;

    push_ortho(w, h);
    {
        glPushAttrib(GL_ENABLE_BIT);
        {
            int a = coins_w + pad_w;

            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_COLOR_MATERIAL);

            game_render_labels();

            game_render_small((balls / 10), a,           0);
            game_render_small((balls % 10), a + small_w, 0);

            game_render_small((coins / 10), w - a - small_w * 2, 0);
            game_render_small((coins % 10), w - a - small_w * 1, 0);

            game_render_large((ticks / 10), w / 2 - large_w, 0);
            game_render_large((ticks % 10), w / 2,           0);

            game_render_fps();
        }
        glPopAttrib();
    }
    pop_ortho();
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

void game_render_env(void)
{
    const float light_p[4] = { 8.0, 32.0, 8.0, 1.0 };

    const struct s_file *fp = &file;
    const struct s_ball *up =  file.uv;

    const double *ball_p = file.uv->p;
    
    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        gluPerspective(FOV, (GLdouble) main_width / main_height, 0.1, 1000.0);
    }
    glMatrixMode(GL_MODELVIEW);

    glPushAttrib(GL_LIGHTING_BIT);
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

        /* Rotate the environment about the position of the ball. */

        glTranslated(+ball_p[0], +ball_p[1], +ball_p[2]);
        glRotated(-game_rz, view_e[2][0], view_e[2][1], view_e[2][2]);
        glRotated(-game_rx, view_e[0][0], view_e[0][1], view_e[0][2]);
        glTranslated(-ball_p[0], -ball_p[1], -ball_p[2]);

        /* Configure the lighting. */

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, light_p);

        /* Configure the ball shadow. */

        glActiveTextureARB(GL_TEXTURE1_ARB);
        glMatrixMode(GL_TEXTURE);
        {
            double k = 0.5 / up->r;

            glEnable(GL_TEXTURE_2D);

            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glBindTexture(GL_TEXTURE_2D, shadow_text);

            glLoadIdentity();
            glTranslated(0.5 - ball_p[0] * k, 0.5 - ball_p[2] * k, 0.0);
            glScaled(k, k, 1.0);
        }
        glMatrixMode(GL_MODELVIEW);
        glActiveTextureARB(GL_TEXTURE0_ARB);

        /* Draw the floor. */

        sol_render(fp);

        /* Lose the shadow. */

        glActiveTextureARB(GL_TEXTURE1_ARB);
        {
            glDisable(GL_TEXTURE_2D);
        }
        glActiveTextureARB(GL_TEXTURE0_ARB);

        /* Draw the game elements. */

        coin_draw(fp->cv, fp->cc);
        ball_draw(up->r, up->p, up->e);
        goal_draw(-rx, -ry, fp->zv, fp->zc);
        part_draw(-rx, -ry);
    }
    glPopMatrix();
    glPopAttrib();
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
        coins += n;

        if (coins >= 100)
        {
            coins -= 100;
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

    while (t > MAX_DT && n < MAX_DN)
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

void game_update_x(int k)
{
    game_rx = -20.0 * k / 32768;
}

void game_update_z(int k)
{
    game_rz = +20.0 * k / 32768;
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
        double q[3];
        double v[3];

        p[0] = p[1] = p[2] = 0.0;

        if (fp->uc > 0) v_cpy(p, fp->uv[0].p);

        v_mad(p, p, view_e[1], view_dy);
        v_mad(p, p, view_e[2], view_dz);

        if (k >= 0.0)
        {
            q[0] = +fp->pv[0].p[0];
            q[1] = +fp->pv[0].p[1];
            q[2] = +fp->pv[0].p[2];
        }
        else
        {
            q[0] = -fp->pv[0].p[0];
            q[1] = +fp->pv[0].p[1];
            q[2] = +fp->pv[0].p[2];
        }

        v_sub(v, q, p);
        v_mad(view_p, p, v, +(k * k));
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
    hud_init();

    shadow_text = make_image_from_file(NULL, NULL, IMG_SHADOW);

    sol_load(&file, s, get_image_scale());

    ticks = t;
    time  = t;
}

void game_free(void)
{
    sol_free(&file);

    glDeleteTextures(1, &shadow_text);

    hud_free();
    part_free();
    coin_free();
    goal_free();
    ball_free();
}

char *game_note(void)
{
    return file.av;
}

/*---------------------------------------------------------------------------*/

