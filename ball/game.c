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
#include "ball.h"
#include "image.h"
#include "audio.h"
#include "solid_gl.h"
#include "config.h"
#include "binary.h"

/*---------------------------------------------------------------------------*/

static int game_state = 0;

static struct s_file file;
static struct s_file back;

static float timer      = 0.f;          /* Clock time                        */
static int   timer_down = 1;            /* Timer go up or down?              */

static float game_rx;                   /* Floor rotation about X axis       */
static float game_rz;                   /* Floor rotation about Z axis       */

static float view_a;                    /* Ideal view rotation about Y axis  */
static float view_dc;                   /* Ideal view distance above ball    */
static float view_dp;                   /* Ideal view distance above ball    */
static float view_dz;                   /* Ideal view distance behind ball   */
static float view_fov;                  /* Field of view                     */

static float view_c[3];                 /* Current view center               */
static float view_v[3];                 /* Current view vector               */
static float view_p[3];                 /* Current view position             */
static float view_e[3][3];              /* Current view reference frame      */
static float view_k;

static int   coins  = 0;                /* Collected coins                   */
static int   goal_e = 0;                /* Goal enabled flag                 */
static float goal_k = 0;                /* Goal animation                    */
static int   jump_e = 1;                /* Jumping enabled flag              */
static int   jump_b = 0;                /* Jump-in-progress flag             */
static int   jump_u = 0;                /* Which ball is jumping?            */
static float jump_dt;                   /* Jump duration                     */
static float jump_p[3];                 /* Jump destination                  */
static float fade_k = 0.0;              /* Fade in/out level                 */
static float fade_d = 0.0;              /* Fade in/out direction             */

/*---------------------------------------------------------------------------*/

/*
 * This is an abstraction of the game's input state.  All input is
 * encapsulated here, and all references to the input by the game are made
 * here.  This has the effect of homogenizing input for use in replay
 * recording and playback.
 *
 * x and z:
 *     -32767 = -ANGLE_BOUND
 *     +32767 = +ANGLE_BOUND
 *
 * r:
 *     -32767 = -VIEWR_BOUND
 *     +32767 = +VIEWR_BOUND
 *
 */

struct input
{
    short x;
    short z;
    short r;
    short c;
};

static struct input input_current;

static void input_init(void)
{
    input_current.x = 0;
    input_current.z = 0;
    input_current.r = 0;
    input_current.c = 0;
}

static void input_set_x(float x)
{
    if (x < -ANGLE_BOUND) x = -ANGLE_BOUND;
    if (x >  ANGLE_BOUND) x =  ANGLE_BOUND;

    input_current.x = (short) (32767.0f * x / ANGLE_BOUND);
}

static void input_set_z(float z)
{
    if (z < -ANGLE_BOUND) z = -ANGLE_BOUND;
    if (z >  ANGLE_BOUND) z =  ANGLE_BOUND;

    input_current.z = (short) (32767.0f * z / ANGLE_BOUND);
}

static void input_set_r(float r)
{
    if (r < -VIEWR_BOUND) r = -VIEWR_BOUND;
    if (r >  VIEWR_BOUND) r =  VIEWR_BOUND;

    input_current.r = (short) (32767.0f * r / VIEWR_BOUND);
}

static void input_set_c(int c)
{
    input_current.c = (short) c;
}

static float input_get_x(void)
{
    return ANGLE_BOUND * (float) input_current.x / 32767.0f;
}

static float input_get_z(void)
{
    return ANGLE_BOUND * (float) input_current.z / 32767.0f;
}

static float input_get_r(void)
{
    return VIEWR_BOUND * (float) input_current.r / 32767.0f;
}

static int input_get_c(void)
{
    return (int) input_current.c;
}

int input_put(FILE *fout)
{
    if (game_state)
    {
        put_short(fout, &input_current.x);
        put_short(fout, &input_current.z);
        put_short(fout, &input_current.r);
        put_short(fout, &input_current.c);

        return 1;
    }
    return 0;
}

int input_get(FILE *fin)
{
    if (game_state)
    {
        get_short(fin, &input_current.x);
        get_short(fin, &input_current.z);
        get_short(fin, &input_current.r);
        get_short(fin, &input_current.c);

        return (feof(fin) ? 0 : 1);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static int   grow = 0;                  /* Should the ball be changing size? */
static float grow_orig = 0;             /* the original ball size            */
static float grow_goal = 0;             /* how big or small to get!          */
static float grow_t = 0.0;              /* timer for the ball to grow...     */
static float grow_strt = 0;             /* starting value for growth         */
static int   got_orig = 0;              /* Do we know original ball size?    */

#define GROW_TIME  0.5f                 /* sec for the ball to get to size.  */
#define GROW_BIG   1.5f                 /* large factor                      */
#define GROW_SMALL 0.5f                 /* small factor                      */

static int   grow_state = 0;            /* Current state (values -1, 0, +1)  */

static void grow_init(const struct s_file *fp, int type)
{
    if (!got_orig)
    {
        grow_orig  = fp->uv->r;
        grow_goal  = grow_orig;
        grow_strt  = grow_orig;

        grow_state = 0;

        got_orig   = 1;
    }

    if (type == ITEM_SHRINK)
    {
        switch (grow_state)
        {
        case -1:
            break;

        case  0:
            audio_play(AUD_SHRINK, 1.f);
            grow_goal = grow_orig * GROW_SMALL;
            grow_state = -1;
            grow = 1;
            break;

        case +1:
            audio_play(AUD_SHRINK, 1.f);
            grow_goal = grow_orig;
            grow_state = 0;
            grow = 1;
            break;
        }
    }
    else if (type == ITEM_GROW)
    {
        switch (grow_state)
        {
        case -1:
            audio_play(AUD_GROW, 1.f);
            grow_goal = grow_orig;
            grow_state = 0;
            grow = 1;
            break;

        case  0:
            audio_play(AUD_GROW, 1.f);
            grow_goal = grow_orig * GROW_BIG;
            grow_state = +1;
            grow = 1;
            break;

        case +1:
            break;
        }
    }

    if (grow)
    {
        grow_t = 0.0;
        grow_strt = fp->uv->r;
    }
}

static void grow_step(const struct s_file *fp, float dt)
{
    float dr;

    if (!grow)
        return;

    /* Calculate new size based on how long since you touched the coin... */

    grow_t += dt;

    if (grow_t >= GROW_TIME)
    {
        grow = 0;
        grow_t = GROW_TIME;
    }

    dr = grow_strt + ((grow_goal-grow_strt) * (1.0f / (GROW_TIME / grow_t)));

    /* No sinking through the floor! Keeps ball's bottom constant. */

    fp->uv->p[1] += (dr - fp->uv->r);
    fp->uv->r     =  dr;
}

/*---------------------------------------------------------------------------*/

static void view_init(void)
{
    view_fov = (float) config_get_d(CONFIG_VIEW_FOV);
    view_dp  = (float) config_get_d(CONFIG_VIEW_DP) / 100.0f;
    view_dc  = (float) config_get_d(CONFIG_VIEW_DC) / 100.0f;
    view_dz  = (float) config_get_d(CONFIG_VIEW_DZ) / 100.0f;
    view_k   = 1.0f;
    view_a   = 0.0f;

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

int game_init(const char *file_name, int t, int e)
{
    char *back_name = NULL, *grad_name = NULL;

    int i;

    timer      = (float) t / 100.f;
    timer_down = (t > 0);
    coins      = 0;

    if (game_state)
        game_free();

    if (!sol_load_gl(&file, config_data(file_name),
                     config_get_d(CONFIG_TEXTURES),
                     config_get_d(CONFIG_SHADOW)))
        return (game_state = 0);

    game_state = 1;

    input_init();

    game_rx = 0.0f;
    game_rz = 0.0f;

    /* Initialize jump and goal states. */

    jump_e = 1;
    jump_b = 0;
    jump_u = 0;

    goal_e = e ? 1    : 0;
    goal_k = e ? 1.0f : 0.0f;

    /* Initialise the level, background, particles, fade, and view. */

    fade_k =  1.0f;
    fade_d = -2.0f;

    for (i = 0; i < file.dc; i++)
    {
        char *k = file.av + file.dv[i].ai;
        char *v = file.av + file.dv[i].aj;

        if (strcmp(k, "back") == 0) back_name = v;
        if (strcmp(k, "grad") == 0) grad_name = v;
    }

    part_reset(GOAL_HEIGHT);
    view_init();
    back_init(grad_name, config_get_d(CONFIG_GEOMETRY));

    sol_load_gl(&back, config_data(back_name),
                config_get_d(CONFIG_TEXTURES), 0);

    /* Initialize ball size tracking... */

    got_orig = 0;
    grow = 0;

    /* Initialize play states */
    game_set_play(-2, 0);
    game_set_play( 0, 1);

    return game_state;
}

void game_free(void)
{
    if (game_state)
    {
        sol_free_gl(&file);
        sol_free_gl(&back);
        back_free();
    }
    game_state = 0;
}

/*---------------------------------------------------------------------------*/

int curr_clock(void)
{
    return (int) (timer * 100.f);
}

int curr_coins(void)
{
    return coins;
}

/*---------------------------------------------------------------------------*/

static void game_draw_balls(const struct s_file *fp,
                            const float *bill_M, float t)
{
    float c[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    float ball_M[16];
    float pend_M[16];

    int ui;

    for (ui = 0; ui < fp->uc; ui++)
    {
        if (!fp->uv[ui].P)
            continue;

        m_basis(ball_M, fp->uv[ui].e[0], fp->uv[ui].e[1], fp->uv[ui].e[2]);
        m_basis(pend_M, fp->uv[ui].E[0], fp->uv[ui].E[1], fp->uv[ui].E[2]);

        glPushMatrix();
        {
            glTranslatef(fp->uv[ui].p[0],
                         fp->uv[ui].p[1] + BALL_FUDGE,
                         fp->uv[ui].p[2]);
            glScalef(fp->uv[ui].r,
                     fp->uv[ui].r,
                     fp->uv[ui].r);

            glColor4fv(c);
            ball_draw(ball_M, pend_M, bill_M, t);
        }
        glPopMatrix();
    }
}

static void game_draw_items(const struct s_file *fp, float t)
{
    float r = 360.f * t;
    int hi;

    glPushAttrib(GL_LIGHTING_BIT);
    {
        item_push(ITEM_COIN);
        {
            for (hi = 0; hi < fp->hc; hi++)

                if (fp->hv[hi].t == ITEM_COIN && fp->hv[hi].n > 0)
                {
                    glPushMatrix();
                    {
                        glTranslatef(fp->hv[hi].p[0],
                                     fp->hv[hi].p[1],
                                     fp->hv[hi].p[2]);
                        glRotatef(r, 0.0f, 1.0f, 0.0f);
                        item_draw(&fp->hv[hi], r);
                    }
                    glPopMatrix();
                }
        }
        item_pull();

        item_push(ITEM_SHRINK);
        {
            for (hi = 0; hi < fp->hc; hi++)

                if (fp->hv[hi].t == ITEM_SHRINK)
                {
                    glPushMatrix();
                    {
                        glTranslatef(fp->hv[hi].p[0],
                                     fp->hv[hi].p[1],
                                     fp->hv[hi].p[2]);
                        glRotatef(r, 0.0f, 1.0f, 0.0f);
                        item_draw(&fp->hv[hi], r);
                    }
                    glPopMatrix();
                }
        }
        item_pull();

        item_push(ITEM_GROW);
        {
            for (hi = 0; hi < fp->hc; hi++)

                if (fp->hv[hi].t == ITEM_GROW)
                {
                    glPushMatrix();
                    {
                        glTranslatef(fp->hv[hi].p[0],
                                     fp->hv[hi].p[1],
                                     fp->hv[hi].p[2]);
                        glRotatef(r, 0.0f, 1.0f, 0.0f);
                        item_draw(&fp->hv[hi], r);
                    }
                    glPopMatrix();
                }
        }
        item_pull();
    }
    glPopAttrib();
}

static void game_draw_goals(const struct s_file *fp, const float *M, float t)
{
    if (goal_e)
    {
        int zi;

        /* Draw the goal particles. */

        glEnable(GL_TEXTURE_2D);
        {
            for (zi = 0; zi < fp->zc; zi++)
            {
                glPushMatrix();
                {
                    glTranslatef(fp->zv[zi].p[0],
                                 fp->zv[zi].p[1],
                                 fp->zv[zi].p[2]);

                    part_draw_goal(M, fp->zv[zi].r, goal_k, t);
                }
                glPopMatrix();
            }
        }
        glDisable(GL_TEXTURE_2D);

        /* Draw the goal column. */

        for (zi = 0; zi < fp->zc; zi++)
        {
            glPushMatrix();
            {
                glTranslatef(fp->zv[zi].p[0],
                             fp->zv[zi].p[1],
                             fp->zv[zi].p[2]);

                glScalef(fp->zv[zi].r,
                         goal_k,
                         fp->zv[zi].r);

                goal_draw();
            }
            glPopMatrix();
        }
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
            glScalef(fp->jv[ji].r,
                     1.0f,
                     fp->jv[ji].r);

            jump_draw(!jump_e);
        }
        glPopMatrix();
    }
}

static void game_draw_swchs(const struct s_file *fp)
{
    int xi;

    for (xi = 0; xi < fp->xc; xi++)
    {
        if (fp->xv[xi].i)
            continue;

        glPushMatrix();
        {
            glTranslatef(fp->xv[xi].p[0],
                         fp->xv[xi].p[1],
                         fp->xv[xi].p[2]);
            glScalef(fp->xv[xi].r,
                     1.0f,
                     fp->xv[xi].r);

            swch_draw(fp->xv[xi].f, fp->xv[xi].e);
        }
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/

static void game_draw_tilt(int d)
{
    const float *ball_p = file.uv->p;

    /* Rotate the environment about the position of the ball. */

    glTranslatef(+ball_p[0], +ball_p[1] * d, +ball_p[2]);
    glRotatef(-game_rz * d, view_e[2][0], view_e[2][1], view_e[2][2]);
    glRotatef(-game_rx * d, view_e[0][0], view_e[0][1], view_e[0][2]);
    glTranslatef(-ball_p[0], -ball_p[1] * d, -ball_p[2]);
}

static void game_refl_all(void)
{
    glPushMatrix();
    {
        game_draw_tilt(1);

        /* Draw the floor. */

        sol_refl(&file);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

static void game_draw_light(void)
{
    const float light_p[2][4] = {
        { -8.0f, +32.0f, -8.0f, 0.0f },
        { +8.0f, +32.0f, +8.0f, 0.0f },
    };
    const float light_c[2][4] = {
        { 1.0f, 0.8f, 0.8f, 1.0f },
        { 0.8f, 1.0f, 0.8f, 1.0f },
    };

    /* Configure the lighting. */

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_p[0]);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_c[0]);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_c[0]);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, light_p[1]);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_c[1]);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_c[1]);
}

static void game_draw_back(int pose, int d, float t)
{
    glPushMatrix();
    {
        if (d < 0)
        {
            glRotatef(game_rz * 2, view_e[2][0], view_e[2][1], view_e[2][2]);
            glRotatef(game_rx * 2, view_e[0][0], view_e[0][1], view_e[0][2]);
        }

        glTranslatef(view_p[0], view_p[1] * d, view_p[2]);

        if (config_get_d(CONFIG_BACKGROUND))
        {
            /* Draw all background layers back to front. */

            sol_back(&back, BACK_DIST, FAR_DIST,  t);
            back_draw(0);
            sol_back(&back,         0, BACK_DIST, t);
        }
        else back_draw(0);
    }
    glPopMatrix();
}

static void game_clip_refl(int d)
{
    /* Fudge to eliminate the floor from reflection. */

    GLdouble e[4], k = -0.00001;

    e[0] = 0;
    e[1] = 1;
    e[2] = 0;
    e[3] = k;

    glClipPlane(GL_CLIP_PLANE0, e);
}

static void game_clip_ball(int d, const float *p)
{
    GLdouble r, c[3], pz[4], nz[4];

    /* Compute the plane giving the front of the ball, as seen from view_p. */

    c[0] = p[0];
    c[1] = p[1] * d;
    c[2] = p[2];

    pz[0] = view_p[0] - c[0];
    pz[1] = view_p[1] - c[1];
    pz[2] = view_p[2] - c[2];

    r = sqrt(pz[0] * pz[0] + pz[1] * pz[1] + pz[2] * pz[2]);

    pz[0] /= r;
    pz[1] /= r;
    pz[2] /= r;
    pz[3] = -(pz[0] * c[0] +
              pz[1] * c[1] +
              pz[2] * c[2]);

    /* Find the plane giving the back of the ball, as seen from view_p. */

    nz[0] = -pz[0];
    nz[1] = -pz[1];
    nz[2] = -pz[2];
    nz[3] = -pz[3];

    /* Reflect these planes as necessary, and store them in the GL state. */

    pz[1] *= d;
    nz[1] *= d;

    glClipPlane(GL_CLIP_PLANE1, nz);
    glClipPlane(GL_CLIP_PLANE2, pz);
}

static void game_draw_fore(int pose, const float *M, int d, float t)
{
    const float *ball_p = file.uv->p;
    const float  ball_r = file.uv->r;

    glPushMatrix();
    {
        /* Rotate the environment about the position of the ball. */

        game_draw_tilt(d);

        /* Compute clipping planes for reflection and ball facing. */

        game_clip_refl(d);
        game_clip_ball(d, ball_p);

        if (d < 0)
            glEnable(GL_CLIP_PLANE0);

        if (pose)
            sol_draw(&file, 0, 1);
        else
        {
            /* Draw the coins. */

            game_draw_items(&file, t);

            /* Draw the floor. */

            sol_draw(&file, 0, 1);

            /* Draw the ball shadow. */

            if (d > 0 && config_get_d(CONFIG_SHADOW))
            {
                shad_draw_set(ball_p, ball_r);
                sol_shad(&file);
                shad_draw_clr();
            }

            /* Draw the ball. */

            game_draw_balls(&file, M, t);
        }

        /* Draw the particles and light columns. */

        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDepthMask(GL_FALSE);
        {
            glColor3f(1.0f, 1.0f, 1.0f);

            sol_bill(&file, M, t);
            part_draw_coin(M, t);

            glDisable(GL_TEXTURE_2D);
            {
                game_draw_goals(&file, M, t);
                game_draw_jumps(&file);
                game_draw_swchs(&file);
            }
            glEnable(GL_TEXTURE_2D);

            glColor3f(1.0f, 1.0f, 1.0f);
        }
        glDepthMask(GL_TRUE);
        glEnable(GL_LIGHTING);
        glDisable(GL_COLOR_MATERIAL);

        if (d < 0)
            glDisable(GL_CLIP_PLANE0);
    }
    glPopMatrix();
}

void game_draw(int pose, float t)
{
    float fov = view_fov;

    if (jump_b)
    {
        if (jump_u == 0)
            fov *= 2.0f * fabsf(jump_dt - 0.5f);

        else
            fov /= 1.9f * fabsf(jump_dt - 0.5f);
    }

    if (game_state)
    {
        config_push_persp(fov, 0.1f, FAR_DIST);
        glPushMatrix();
        {
            float T[16], U[16], M[16], v[3];

            /* Compute direct and reflected view bases. */

            v[0] = +view_p[0];
            v[1] = -view_p[1];
            v[2] = +view_p[2];

            m_view(T, view_c, view_p, view_e[1]);
            m_view(U, view_c, v,      view_e[1]);

            m_xps(M, T);

            /* Apply current the view. */

            v_sub(v, view_c, view_p);

            glTranslatef(0.f, 0.f, -v_len(v));
            glMultMatrixf(M);
            glTranslatef(-view_c[0], -view_c[1], -view_c[2]);

            if (config_get_d(CONFIG_REFLECTION))
            {
                glEnable(GL_STENCIL_TEST);
                {
                    /* Draw the mirrors only into the stencil buffer. */

                    glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
                    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    glDepthMask(GL_FALSE);

                    game_refl_all();

                    glDepthMask(GL_TRUE);
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                    glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);

                    /* Draw the scene reflected into color and depth buffers. */

                    glFrontFace(GL_CW);
                    glPushMatrix();
                    {
                        glScalef(+1.0f, -1.0f, +1.0f);

                        game_draw_light();
                        game_draw_back(pose,    -1, t);
                        game_draw_fore(pose, U, -1, t);
                    }
                    glPopMatrix();
                    glFrontFace(GL_CCW);
                }
                glDisable(GL_STENCIL_TEST);
            }

            /* Draw the scene normally. */

            game_draw_light();
            game_refl_all();
            game_draw_back(pose,    +1, t);
            game_draw_fore(pose, T, +1, t);
        }
        glPopMatrix();
        config_pop_matrix();

        /* Draw the fade overlay. */

        fade_draw(fade_k);
    }
}

/*---------------------------------------------------------------------------*/

static void game_update_grav(float h[3], const float g[3])
{
    float x[3];
    float y[3] = { 0.0f, 1.0f, 0.0f };
    float z[3];
    float X[16];
    float Z[16];
    float M[16];

    /* Compute the gravity vector from the given world rotations. */

    z[0] = fsinf(V_RAD(view_a));
    z[1] = 0.0f;
    z[2] = fcosf(V_RAD(view_a));

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
    float dc = view_dc * (jump_b ? 2.0f * fabsf(jump_dt - 0.5f) : 1.0f);
    float da = input_get_r() * dt * 90.0f;
    float k;

    float M[16], v[3], Y[3] = { 0.0f, 1.0f, 0.0f };

    view_a += da;

    /* Center the view about the ball. */

    v_cpy(view_c, file.uv->p);
    v_inv(view_v, file.uv->v);

    view_e[2][0] = fsinf(V_RAD(view_a));
    view_e[2][1] = 0.0;
    view_e[2][2] = fcosf(V_RAD(view_a));

    switch (input_get_c())
    {
    case 1: /* Camera 1: Viewpoint chases the ball position. */

        v_mad(view_e[2], view_e[2], view_v, v_dot(view_v, view_v) * dt / 16);

        break;

    case 2: /* Camera 2: View vector is given by view angle. */

        break;

    default: /* Default: View vector approaches the ball velocity vector. */

        v_mad(view_e[2], view_e[2], view_v, v_dot(view_v, view_v) * dt / 4);

        break;
    }

    /* Orthonormalize the new view reference frame. */

    v_crs(view_e[0], view_e[1], view_e[2]);
    v_crs(view_e[2], view_e[0], view_e[1]);
    v_nrm(view_e[0], view_e[0]);
    v_nrm(view_e[2], view_e[2]);

    /* Compute the new view position. */

    k = 1.0f + v_dot(view_e[2], view_v) / 10.0f;

    view_k = view_k + (k - view_k) * dt;

    if (view_k < 0.5) view_k = 0.5;

    v_scl(v,    view_e[1], view_dp * view_k);
    v_mad(v, v, view_e[2], view_dz * view_k);
    m_rot(M, Y, V_RAD(da));
    m_vxfm(view_p, M, v);
    v_add(view_p, view_p, file.uv->p);

    /* Compute the new view center. */

    v_cpy(view_c, file.uv->p);
    v_mad(view_c, view_c, view_e[1], dc);

    /* Note the current view angle. */

    view_a = V_DEG(fatan2f(view_e[2][0], view_e[2][2]));
}

static void game_update_time(float dt, int b)
{
    if (goal_e && goal_k < 1.0f)
        goal_k += dt;

   /* The ticking clock. */

    if (b && timer_down)
    {
        if (timer < 600.f)
            timer -= dt;
        if (timer < 0.f)
            timer = 0.f;
    }
    else if (b)
    {
        timer += dt;
    }
}

static int game_update_state(int bt)
{
    struct s_file *fp = &file;
    struct s_item *hp;

    float p[3];
    float c[3];

    int i;

    /* Test for an item. */

    if (bt && (hp = sol_item_test(fp, p, COIN_RADIUS)))
    {
        item_color(hp, c);
        part_burst(p, c);

        grow_init(fp, hp->t);

        if (hp->t == ITEM_COIN)
            coins += hp->n;

        audio_play(AUD_COIN, 1.f);

        /* Discard item. */

        hp->t = ITEM_NONE;
    }

    /* Test for a switch. */

    if (sol_swch_test(fp))
        audio_play(AUD_SWITCH, 1.f);

    /* Test for a jump. */

    for (i = 0; i < fp->uc; i++)
    {
        if (jump_e == 1 &&
            jump_b == 0 &&
            sol_jump_test(fp, jump_p, i) == 1)
        {
            jump_b  = 1;
            jump_e  = 0;
            jump_dt = 0.f;
            jump_u  = i;

            audio_play(AUD_JUMP, 1.f);
        }

        if (jump_e == 0 &&
            jump_b == 0 &&
            jump_u == i &&
            sol_jump_test(fp, jump_p, i) == 0)
        {
            /* Enable jumping after jump finished */
            jump_e = 1;
        }
    }

    /* Test for a goal. */

    for (i = 0; i < fp->uc; i++)
        if (bt && goal_e && (sol_goal_test(fp, NULL, i) == 2))
        {
            audio_play(AUD_GOAL, 1.0f);
            return GAME_GOAL;
        }

    /* Test for time-out. */

    if (bt && timer_down && timer <= 0.f)
    {
        audio_play(AUD_TIME, 1.0f);
        return GAME_TIME;
    }

    /* Test for fall-out. */

    if (bt && fp->uv[0].p[1] < fp->vv[0].p[1])
    {
        audio_play(AUD_FALL, 1.0f);
        return GAME_FALL;
    }

    return GAME_NONE;
}

int game_step(const float g[3], float dt, int bt)
{
    if (game_state)
    {
        struct s_file *fp = &file;

        float h[3];

        /* Smooth jittery or discontinuous input. */

        game_rx += (input_get_x() - game_rx) * dt / RESPONSE;
        game_rz += (input_get_z() - game_rz) * dt / RESPONSE;

        grow_step(fp, dt);

        game_update_grav(h, g);
        part_step(h, dt);

        if (jump_b)
        {
            jump_dt += dt;

            /* Handle a jump. */

            if (0.5f < jump_dt)
            {
                if (jump_u)
                {
                    fp->uv[jump_u - 1].p[0] = jump_p[0];
                    fp->uv[jump_u - 1].p[1] = jump_p[1];
                    fp->uv[jump_u - 1].p[2] = jump_p[2];
                }

                else
                {
                    fp->uv[0].p[0] = jump_p[0];
                    fp->uv[0].p[1] = jump_p[1];
                    fp->uv[0].p[2] = jump_p[2];
                }
            }
            if (1.0f < jump_dt)
                jump_b = 0;
        }
        else
        {
            /* Run the sim. */

            float b = sol_step(fp, h, dt, 0, NULL);

            /* Mix the sound of a ball bounce. */

            if (b > 0.5f)
            {
                float k = (b - 0.5f) * 2.0f;

                if (got_orig)
                {
                    if      (fp->uv->r > grow_orig) audio_play(AUD_BUMPL, k);
                    else if (fp->uv->r < grow_orig) audio_play(AUD_BUMPS, k);
                    else                            audio_play(AUD_BUMPM, k);
                }
                else audio_play(AUD_BUMPM, k);
            }
        }

        game_step_fade(dt);
        game_update_view(dt);
        game_update_time(dt, bt);

        return game_update_state(bt);
    }
    return GAME_NONE;
}

/*---------------------------------------------------------------------------*/

void game_set_goal(void)
{
    audio_play(AUD_SWITCH, 1.0f);
    goal_e = 1;
}

void game_clr_goal(void)
{
    goal_e = 0;
}

/*---------------------------------------------------------------------------*/

/*
 * Set ball B's play state as S.  Additional values can be used for b:
 * -1: Set current ball to s
 * -2: Set all balls to s
 */

void game_set_play(int b, int s)
{
    int i;

    if (b >=  0 && b  < file.uc)
        file.uv[b].P = s;

    if (b == -1)
        file.uv[0].P = s;

    if (b == -2)
        for (i = 0; i < file.uc; i++)
            file.uv[i].P = s;
}

/*---------------------------------------------------------------------------*/

void game_set_x(int k)
{
    input_set_x(-ANGLE_BOUND * k / JOY_MAX);
}

void game_set_z(int k)
{
    input_set_z(+ANGLE_BOUND * k / JOY_MAX);
}

void game_set_ang(int x, int z)
{
    input_set_x(x);
    input_set_z(z);
}

void game_set_pos(int x, int y)
{
    input_set_x(input_get_x() + 40.0f * y / config_get_d(CONFIG_MOUSE_SENSE));
    input_set_z(input_get_z() + 40.0f * x / config_get_d(CONFIG_MOUSE_SENSE));
}

void game_set_cam(int c)
{
    input_set_c(c);
}

void game_set_rot(float r)
{
    input_set_r(r);
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

    z[0] = fsinf(V_RAD(view_a));
    z[2] = fcosf(V_RAD(view_a));

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

void game_look(float phi, float theta)
{
    view_c[0] = view_p[0] + fsinf(V_RAD(theta)) * fcosf(V_RAD(phi));
    view_c[1] = view_p[1] +                       fsinf(V_RAD(phi));
    view_c[2] = view_p[2] - fcosf(V_RAD(theta)) * fcosf(V_RAD(phi));
}

/*---------------------------------------------------------------------------*/

void game_kill_fade(void)
{
    fade_k = 0.0f;
    fade_d = 0.0f;
}

void game_step_fade(float dt)
{
    if ((fade_k < 1.0f && fade_d > 0.0f) ||
        (fade_k > 0.0f && fade_d < 0.0f))
        fade_k += fade_d * dt;

    if (fade_k < 0.0f)
    {
        fade_k = 0.0f;
        fade_d = 0.0f;
    }
    if (fade_k > 1.0f)
    {
        fade_k = 1.0f;
        fade_d = 0.0f;
    }
}

void game_fade(float d)
{
    fade_d = d;
}

/*---------------------------------------------------------------------------*/

const char *status_to_str(int s)
{
    switch (s)
    {
    case GAME_NONE:    return _("Aborted");
    case GAME_TIME:    return _("Time-out");
    case GAME_GOAL:    return _("Success");
    case GAME_FALL:    return _("Fall-out");
    default:           return _("Unknown");
    }
}

/*---------------------------------------------------------------------------*/
