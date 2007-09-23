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
#include "image.h"
#include "audio.h"
#include "solid_gl.h"
#include "config.h"
#include "binary.h"
#include "level.h"

/*---------------------------------------------------------------------------*/

static int game_state = 0;

static struct s_file file;
static struct s_file back;

static float clock      = 0.f;          /* Clock time                        */
static int   clock_down = 1;            /* Clock go up or down?              */

static float game_ix;                   /* Input rotation about X axis       */
static float game_iz;                   /* Input rotation about Z axis       */
static float game_rx;                   /* Floor rotation about X axis       */
static float game_rz;                   /* Floor rotation about Z axis       */

static float view_a;                    /* Ideal view rotation about Y axis  */
static float view_ry;                   /* Angular velocity about Y axis     */
static float view_dc;                   /* Ideal view distance above ball    */
static float view_dp;                   /* Ideal view distance above ball    */
static float view_dz;                   /* Ideal view distance behind ball   */
static float view_fov;                  /* Field of view                     */

static float view_c[3];                 /* Current view center               */
static float view_v[3];                 /* Current view vector               */
static float view_p[3];                 /* Current view position             */
static float view_e[3][3];              /* Current view orientation          */
static float view_k;

static int   coins  = 0;                /* Collected coins                   */
static int   goal_c = 0;                /* Goal coins remaining (0 = open)   */
static float goal_k = 0;                /* Goal animation                    */
static int   jump_e = 1;                /* Jumping enabled flag              */
static int   jump_b = 0;                /* Jump-in-progress flag             */
static float jump_dt;                   /* Jump duration                     */
static float jump_p[3];                 /* Jump destination                  */
static float fade_k = 0.0;              /* Fade in/out level                 */
static float fade_d = 0.0;              /* Fade in/out direction             */
static int   ball_b = 0;                /* Is the ball a bonus ball?         */

static int   grow = 0;                  /* Should the ball be changing size? */
static float grow_orig = 0;             /* the original ball size            */
static float grow_goal = 0;             /* how big or small to get!          */
static float grow_t = 0.0;              /* timer for the ball to grow...     */
static float grow_strt = 0;             /* starting value for growth         */
static int   got_orig = 0;              /* Do we know original ball size?    */

#define GROW_TIME  0.5f                 /* sec for the ball to get to size.  */
#define GROW_BIG   1.5f                 /* large factor                      */
#define GROW_SMALL 0.5f                 /* small factor                      */

/*---------------------------------------------------------------------------*/

static void grow_set(const struct s_file *fp, int type)
{
    if (!got_orig)
    {
        grow_orig = fp->uv->r;
        grow_goal = grow_orig;
        grow_strt = grow_orig;
        got_orig  = 1;
    }

    switch (type)
    {
    case ITEM_SHRINK:
        audio_play(AUD_SHRINK, 1.f);

        if (grow_goal == grow_orig * GROW_SMALL)
            return;
        else if (grow_goal == grow_orig * GROW_BIG)
        {
            grow_goal = grow_orig;
            grow = 1;
        }
        else
        {
            grow_goal = grow_orig * GROW_SMALL;
            grow = 1;
        }

        break;

    case ITEM_GROW:
        audio_play(AUD_GROW, 1.f);

        if (grow_goal == grow_orig * GROW_BIG)
            return;
        else if (grow_goal == grow_orig * GROW_SMALL)
        {
            grow = 1;
            grow_goal = grow_orig;
        }
        else
        {
            grow_goal = grow_orig * GROW_BIG;
            grow = 1;
        }

        break;

    default:
        break;
    }

    if (grow)
    {
        grow_t = 0.0;
        grow_strt = fp->uv->r;
    }
}

static void grow_ball(const struct s_file *fp, float dt)
{
    float dr;

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
    fp->uv->r = dr;
}

static void view_init(void)
{
    view_a  = 0.f;
    view_ry = 0.f;

    view_fov = (float) config_get_d(CONFIG_VIEW_FOV);
    view_dp  = (float) config_get_d(CONFIG_VIEW_DP) / 100.0f;
    view_dc  = (float) config_get_d(CONFIG_VIEW_DC) / 100.0f;
    view_dz  = (float) config_get_d(CONFIG_VIEW_DZ) / 100.0f;
    view_k   = 1.0f;

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

int game_init(const struct level *level, int t, int g)
{
    clock      = (float) t / 100.f;
    clock_down = (t > 0);
    coins      = 0;

    if (game_state)
        game_free();

    if (!sol_load_gl(&file, config_data(level->file),
                     config_get_d(CONFIG_TEXTURES),
                     config_get_d(CONFIG_SHADOW)))
        return (game_state = 0);

    game_state = 1;

    game_ix = 0.f;
    game_iz = 0.f;
    game_rx = 0.f;
    game_rz = 0.f;

    /* Initialize jump and goal states. */

    jump_e = 1;
    jump_b = 0;

    goal_c = g;
    goal_k = (g == 0) ? 1.0f : 0.0f;

    /* Initialise the level, background, particles, fade, and view. */

    fade_k =  1.0f;
    fade_d = -2.0f;

    part_reset(GOAL_HEIGHT);
    view_init();
    back_init(level->grad, config_get_d(CONFIG_GEOMETRY));

    sol_load_gl(&back, config_data(level->back),
                config_get_d(CONFIG_TEXTURES), 0);

    /* Initialize ball size tracking... */

    got_orig = 0;
    grow = 0;

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
    return (int) (clock * 100.f);
}

int curr_coins(void)
{
    return coins;
}

int curr_goal(void)
{
    return goal_c;
}

/*---------------------------------------------------------------------------*/

static void game_draw_balls(const struct s_file *fp)
{
    float c[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
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

        glColor4fv(c);

        ball_draw(ball_b);
    }
    glPopMatrix();
}

static void game_draw_items(const struct s_file *fp)
{
    float r = 360.f * SDL_GetTicks() / 1000.f;
    int hi;

    /* FIXME:  Draw items of different types in one pass? */

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

static void game_draw_goals(const struct s_file *fp, float rx, float ry)
{
    int zi;

    if (goal_c == 0)
        for (zi = 0; zi < fp->zc; zi++)
        {
            glPushMatrix();
            {
                glTranslatef(fp->zv[zi].p[0],
                             fp->zv[zi].p[1],
                             fp->zv[zi].p[2]);

                part_draw_goal(rx, ry, fp->zv[zi].r, goal_k);

                glScalef(fp->zv[zi].r, goal_k, fp->zv[zi].r);
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

            glScalef(fp->xv[xi].r, 1.f, fp->xv[xi].r);
            swch_draw(fp->xv[xi].f, fp->xv[xi].e);
        }
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/

static void game_refl_all(int s)
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

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_p[0]);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_c[0]);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_c[0]);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, light_p[1]);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_c[1]);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_c[1]);
}

static void game_draw_back(int pose, int d, const float p[3])
{
    float c[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float t = SDL_GetTicks() / 1000.f + 120.0f;

    glPushMatrix();
    {
        if (d < 0)
        {
            glRotatef(game_rz * 2, view_e[2][0], view_e[2][1], view_e[2][2]);
            glRotatef(game_rx * 2, view_e[0][0], view_e[0][1], view_e[0][2]);
        }

        glTranslatef(p[0], p[1], p[2]);
        glColor4fv(c);

        if (config_get_d(CONFIG_BACKGROUND))
        {
            /* Draw all background layers back to front. */

            sol_back(&back, BACK_DIST, FAR_DIST, t);
            back_draw(0);
            sol_back(&back, 0, BACK_DIST, t);

            /* Draw all foreground geometry in the background file. */

            sol_draw(&back);
        }
        else back_draw(0);
    }
    glPopMatrix();
}

static void game_draw_fore(int pose, float rx, float ry, int d, const float p[3])
{
    const float *ball_p = file.uv->p;
    const float  ball_r = file.uv->r;

    glPushAttrib(GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);
    {
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
                e[3] = -0.00001;

                glEnable(GL_CLIP_PLANE0);
                glClipPlane(GL_CLIP_PLANE0, e);
            }

            /* Draw the floor. */

            sol_draw(&file);

            if (config_get_d(CONFIG_SHADOW))
            {
                shad_draw_set(ball_p, ball_r);
                sol_shad(&file);
                shad_draw_clr();
            }

            /* Draw the game elements. */

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (pose == 0)
            {
                part_draw_coin(-rx * d, -ry);
                game_draw_items(&file);
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
}

void game_draw(int pose, float st)
{
    float fov = view_fov;

    if (jump_b) fov *= 2.f * fabsf(jump_dt - 0.5);

    if (game_state)
    {
        config_push_persp(fov, 0.1f, FAR_DIST);
        glPushMatrix();
        {
            float v[3], rx, ry;
            float pup[3];
            float pdn[3];

            v_cpy(pup, view_p);
            v_cpy(pdn, view_p);
            pdn[1] = -pdn[1];

            /* Compute and apply the view. */

            v_sub(v, view_c, view_p);

            rx = V_DEG(fatan2f(-v[1], fsqrtf(v[0] * v[0] + v[2] * v[2])));
            ry = V_DEG(fatan2f(+v[0], -v[2])) + st;

            glTranslatef(0.f, 0.f, -v_len(v));
            glRotatef(rx, 1.f, 0.f, 0.f);
            glRotatef(ry, 0.f, 1.f, 0.f);
            glTranslatef(-view_c[0], -view_c[1], -view_c[2]);

            if (config_get_d(CONFIG_REFLECTION))
            {
                /* Draw the mirror only into the stencil buffer. */

                glDisable(GL_DEPTH_TEST);
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
                glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

                game_refl_all(0);

                /* Draw the scene reflected into color and depth buffers. */

                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);
                glEnable(GL_DEPTH_TEST);

                glFrontFace(GL_CW);
                glPushMatrix();
                {
                    glScalef(+1.f, -1.f, +1.f);

                    game_draw_light();
                    game_draw_back(pose,         -1, pdn);
                    game_draw_fore(pose, rx, ry, -1, pdn);
                }
                glPopMatrix();
                glFrontFace(GL_CCW);

                glDisable(GL_STENCIL_TEST);
            }

            /* Draw the scene normally. */

            game_draw_light();
            game_refl_all(pose ? 0 : config_get_d(CONFIG_SHADOW));
            game_draw_back(pose,         +1, pup);
            game_draw_fore(pose, rx, ry, +1, pup);
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
    float dc = view_dc * (jump_b ? 2.0f * fabsf(jump_dt - 0.5f) : 1.0f);
    float dx = view_ry * dt * 5.0f;
    float k;

    view_a += view_ry * dt * 90.f;

    /* Center the view about the ball. */

    v_cpy(view_c, file.uv->p);
    v_inv(view_v, file.uv->v);

    switch (config_get_d(CONFIG_CAMERA))
    {
    case 1: /* Camera 1:  Viewpoint chases the ball position. */

        v_sub(view_e[2], view_p, view_c);
        break;

    case 2: /* Camera 2: View vector is given by view angle. */

        view_e[2][0] = fsinf(V_RAD(view_a));
        view_e[2][1] = 0.f;
        view_e[2][2] = fcosf(V_RAD(view_a));

        dx = 0.0f;

        break;

    default: /* Default: View vector approaches the ball velocity vector. */

        k = v_dot(view_v, view_v);

        v_sub(view_e[2], view_p, view_c);
        v_mad(view_e[2], view_e[2], view_v, k * dt / 4);

        break;
    }

    /* Orthonormalize the basis of the view in its new position. */

    v_crs(view_e[0], view_e[1], view_e[2]);
    v_crs(view_e[2], view_e[0], view_e[1]);
    v_nrm(view_e[0], view_e[0]);
    v_nrm(view_e[2], view_e[2]);

    /* Compute the new view position. */

    k = 1.0f + v_dot(view_e[2], view_v) / 10.0f;

    view_k = view_k + (k - view_k) * dt;

    if (view_k < 0.5) view_k = 0.5;

    v_cpy(view_p, file.uv->p);
    v_mad(view_p, view_p, view_e[0], dx      * view_k);
    v_mad(view_p, view_p, view_e[1], view_dp * view_k);
    v_mad(view_p, view_p, view_e[2], view_dz * view_k);

    /* Compute the new view center. */

    v_cpy(view_c, file.uv->p);
    v_mad(view_c, view_c, view_e[1], dc);

    /* Note the current view angle. */

    view_a = V_DEG(fatan2f(view_e[2][0], view_e[2][2]));
}

static void game_update_time(float dt, int b)
{
    if (goal_c == 0 && goal_k < 1.0f)
        goal_k += dt;

   /* The ticking clock. */

    if (b && clock_down)
    {
        if (clock < 600.f)
            clock -= dt;
        if (clock < 0.f)
            clock = 0.f;
    }
    else if (b)
    {
        clock += dt;
    }
}

static int game_update_state(int bt)
{
    struct s_file *fp = &file;
    struct s_goal *zp;
    struct s_item *hp;

    float p[3];
    float c[3];

    /* Test for an item. */
    if (bt && (hp = sol_item_test(fp, p, COIN_RADIUS)))
    {
        int sound = AUD_COIN;

        item_color(hp, c);
        part_burst(p, c);

        grow_set(fp, hp->t);

        if (hp->t == ITEM_COIN)
        {
            coins += hp->n;

            /* Check for goal open. */
            if (goal_c > 0)
            {
                goal_c -= hp->n;
                if (goal_c <= 0)
                {
                    sound = AUD_SWITCH;
                    goal_c = 0;
                }
            }
        }
        audio_play(sound, 1.f);

        /* Reset item type. */
        hp->t = ITEM_NONE;
    }

    /* Test for a switch. */
    if (sol_swch_test(fp, 0))
        audio_play(AUD_SWITCH, 1.f);

    /* Test for a jump. */

    if (jump_e == 1 && jump_b == 0 && sol_jump_test(fp, jump_p, 0) == 1)
    {
        jump_b  = 1;
        jump_e  = 0;
        jump_dt = 0.f;

        audio_play(AUD_JUMP, 1.f);
    }
    if (jump_e == 0 && jump_b == 0 && sol_jump_test(fp, jump_p, 0) == 0)
        jump_e = 1;

    /* Test for a goal. */

    if (bt && goal_c == 0 && (zp = sol_goal_test(fp, p, 0)))
    {
        audio_play(AUD_GOAL, 1.0f);
        return GAME_GOAL;
    }

    /* Test for time-out. */

    if (bt && clock_down && clock <= 0.f)
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
 * seek an optimal update rate independent of, yet in integral sync with, the
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

    if (game_state)
    {
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

        if (grow)
            grow_ball(fp, dt);

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

void game_no_aa(void)
{
    float max = game_ix * game_ix + game_iz * game_iz;
    if (max > ANGLE_BOUND * ANGLE_BOUND)
    {
        max = ANGLE_BOUND / sqrt(max);
        game_ix *= max;
        game_iz *= max;
    }
}

void game_set_x(int k)
{
    game_ix = -(ANGLE_BOUND) * k / JOY_MAX;
#if NO_AA
    game_no_aa();
#endif
}

void game_set_z(int k)
{
    game_iz = +ANGLE_BOUND * k / JOY_MAX;
#if NO_AA
    game_no_aa();
#endif
}

void game_set_pos(int x, int y)
{
    game_ix += 40.f * y / config_get_d(CONFIG_MOUSE_SENSE);
    game_iz += 40.f * x / config_get_d(CONFIG_MOUSE_SENSE);

#if NO_AA
    game_no_aa();
#else
    if (game_ix > +ANGLE_BOUND) game_ix = +ANGLE_BOUND;
    if (game_ix < -ANGLE_BOUND) game_ix = -ANGLE_BOUND;
    if (game_iz > +ANGLE_BOUND) game_iz = +ANGLE_BOUND;
    if (game_iz < -ANGLE_BOUND) game_iz = -ANGLE_BOUND;
#endif
}

void game_set_rot(float r)
{
    view_ry = r;
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

int put_game_state(FILE *fout)
{
    if (game_state)
    {
        /* Write the view and tilt state. */

        put_float(fout, &game_rx);
        put_float(fout, &game_rz);
        put_array(fout,  view_c, 3);
        put_array(fout,  view_p, 3);

        /* Write the game simulation state. */

        put_file_state(fout, &file);

        return 1;
    }
    return 0;
}

int get_game_state(FILE *fin)
{
    if (game_state)
    {
        /* Read the view and tilt state. */

        get_float(fin, &game_rx);
        get_float(fin, &game_rz);
        get_array(fin,  view_c, 3);
        get_array(fin,  view_p, 3);

        /* Read the game simulation state. */

        get_file_state(fin, &file);

        return (feof(fin) ? 0 : 1);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
