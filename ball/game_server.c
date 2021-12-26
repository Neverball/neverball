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
#include <assert.h>

#include "vec3.h"
#include "geom.h"
#include "config.h"
#include "binary.h"
#include "common.h"

#include "solid_sim.h"
#include "solid_all.h"

#include "game_common.h"
#include "game_server.h"
#include "game_proxy.h"

#include "cmd.h"

/* Do this differently ASAP. */
void incr_gained(int);

/*---------------------------------------------------------------------------*/

static int server_state = 0;

static struct s_vary vary;

static float timer      = 0.f;          /* Clock time                        */
static int   timer_down = 1;            /* Timer go up or down?              */

static int status = GAME_NONE;          /* Outcome of the game               */

static struct game_tilt tilt;           /* Floor rotation                    */
static struct game_view view;           /* Current view                      */

static float view_k;

static float view_time;                 /* Manual rotation time              */
static float view_fade;

#define VIEW_FADE_MIN 0.2f
#define VIEW_FADE_MAX 1.0f

static int   coins  = 0;                /* Collected coins                   */
static int   goal_e = 0;                /* Goal enabled flag                 */
static int   jump_e = 1;                /* Jumping enabled flag              */
static int   jump_b = 0;                /* Jump-in-progress flag             */
static float jump_dt;                   /* Jump duration                     */
static float jump_p[3];                 /* Jump destination                  */

/*---------------------------------------------------------------------------*/

/*
 * This is an abstraction of the game's input state.  All input is
 * encapsulated here, and all references to the input by the game are
 * made here.
 */

struct input
{
    float s;
    float x;
    float z;
    float r;
    int   c;
};

static struct input input_current;

static void input_init(void)
{
    input_current.s = RESPONSE;
    input_current.x = 0;
    input_current.z = 0;
    input_current.r = 0;
    input_current.c = 0;
}

static void input_set_s(float s)
{
    input_current.s = s;
}

static void input_set_x(float x)
{
    if (x < -ANGLE_BOUND) x = -ANGLE_BOUND;
    if (x >  ANGLE_BOUND) x =  ANGLE_BOUND;

    input_current.x = x;
}

static void input_set_z(float z)
{
    if (z < -ANGLE_BOUND) z = -ANGLE_BOUND;
    if (z >  ANGLE_BOUND) z =  ANGLE_BOUND;

    input_current.z = z;
}

static void input_set_r(float r)
{
    if (r < -VIEWR_BOUND) r = -VIEWR_BOUND;
    if (r >  VIEWR_BOUND) r =  VIEWR_BOUND;

    input_current.r = r;
}

static void input_set_c(int c)
{
    input_current.c = c;
}

static float input_get_s(void)
{
    return input_current.s;
}

static float input_get_x(void)
{
    return input_current.x;
}

static float input_get_z(void)
{
    return input_current.z;
}

static float input_get_r(void)
{
    return input_current.r;
}

static int input_get_c(void)
{
    return input_current.c;
}

/*---------------------------------------------------------------------------*/

/*
 * Utility functions for preparing the "server" state and events for
 * consumption by the "client".
 */

static union cmd cmd;

static void game_cmd_map(const char *name, int ver_x, int ver_y)
{
    cmd.type          = CMD_MAP;
    cmd.map.name      = strdup(name);
    cmd.map.version.x = ver_x;
    cmd.map.version.y = ver_y;
    game_proxy_enq(&cmd);
}

static void game_cmd_eou(void)
{
    cmd.type = CMD_END_OF_UPDATE;
    game_proxy_enq(&cmd);
}

static void game_cmd_ups(void)
{
    cmd.type  = CMD_UPDATES_PER_SECOND;
    cmd.ups.n = UPS;
    game_proxy_enq(&cmd);
}

static void game_cmd_sound(const char *filename, float a)
{
    cmd.type = CMD_SOUND;

    cmd.sound.n = strdup(filename);
    cmd.sound.a = a;

    game_proxy_enq(&cmd);
}

#define audio_play(s, f) game_cmd_sound((s), (f))

static void game_cmd_goalopen(void)
{
    cmd.type = CMD_GOAL_OPEN;
    game_proxy_enq(&cmd);
}

static void game_cmd_updball(void)
{
    cmd.type = CMD_BALL_POSITION;
    v_cpy(cmd.ballpos.p, vary.uv[0].p);
    game_proxy_enq(&cmd);

    cmd.type = CMD_BALL_BASIS;
    v_cpy(cmd.ballbasis.e[0], vary.uv[0].e[0]);
    v_cpy(cmd.ballbasis.e[1], vary.uv[0].e[1]);
    game_proxy_enq(&cmd);

    cmd.type = CMD_BALL_PEND_BASIS;
    v_cpy(cmd.ballpendbasis.E[0], vary.uv[0].E[0]);
    v_cpy(cmd.ballpendbasis.E[1], vary.uv[0].E[1]);
    game_proxy_enq(&cmd);
}

static void game_cmd_updview(void)
{
    cmd.type = CMD_VIEW_POSITION;
    v_cpy(cmd.viewpos.p, view.p);
    game_proxy_enq(&cmd);

    cmd.type = CMD_VIEW_CENTER;
    v_cpy(cmd.viewcenter.c, view.c);
    game_proxy_enq(&cmd);

    cmd.type = CMD_VIEW_BASIS;
    v_cpy(cmd.viewbasis.e[0], view.e[0]);
    v_cpy(cmd.viewbasis.e[1], view.e[1]);
    game_proxy_enq(&cmd);
}

static void game_cmd_ballradius(void)
{
    cmd.type         = CMD_BALL_RADIUS;
    cmd.ballradius.r = vary.uv[0].r;
    game_proxy_enq(&cmd);
}

static void game_cmd_init_balls(void)
{
    cmd.type = CMD_CLEAR_BALLS;
    game_proxy_enq(&cmd);

    cmd.type = CMD_MAKE_BALL;
    game_proxy_enq(&cmd);

    game_cmd_updball();
    game_cmd_ballradius();
}

static void game_cmd_init_items(void)
{
    int i;

    cmd.type = CMD_CLEAR_ITEMS;
    game_proxy_enq(&cmd);

    for (i = 0; i < vary.hc; i++)
    {
        cmd.type = CMD_MAKE_ITEM;

        v_cpy(cmd.mkitem.p, vary.hv[i].p);

        cmd.mkitem.t = vary.hv[i].t;
        cmd.mkitem.n = vary.hv[i].n;

        game_proxy_enq(&cmd);
    }
}

static void game_cmd_pkitem(int hi)
{
    cmd.type      = CMD_PICK_ITEM;
    cmd.pkitem.hi = hi;
    game_proxy_enq(&cmd);
}

static void game_cmd_jump(int e)
{
    cmd.type = e ? CMD_JUMP_ENTER : CMD_JUMP_EXIT;
    game_proxy_enq(&cmd);
}

static void game_cmd_tiltangles(void)
{
    cmd.type = CMD_TILT_ANGLES;

    cmd.tiltangles.x = tilt.rx;
    cmd.tiltangles.z = tilt.rz;

    game_proxy_enq(&cmd);
}

static void game_cmd_tiltaxes(void)
{
    cmd.type = CMD_TILT_AXES;

    v_cpy(cmd.tiltaxes.x, tilt.x);
    v_cpy(cmd.tiltaxes.z, tilt.z);

    game_proxy_enq(&cmd);
}

static void game_cmd_timer(void)
{
    cmd.type    = CMD_TIMER;
    cmd.timer.t = timer;
    game_proxy_enq(&cmd);
}

static void game_cmd_coins(void)
{
    cmd.type    = CMD_COINS;
    cmd.coins.n = coins;
    game_proxy_enq(&cmd);
}

static void game_cmd_status(void)
{
    cmd.type     = CMD_STATUS;
    cmd.status.t = status;
    game_proxy_enq(&cmd);
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

static void grow_init(int type)
{
    if (!got_orig)
    {
        grow_orig  = vary.uv->r;
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
        grow_strt = vary.uv->r;
    }
}

static void grow_step(float dt)
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

    vary.uv->p[1] += (dr - vary.uv->r);
    vary.uv->r     =  dr;

    game_cmd_ballradius();
}

/*---------------------------------------------------------------------------*/

static struct lockstep server_step;

int game_server_init(const char *file_name, int t, int e)
{
    struct { int x, y; } version;
    int i;

    timer      = (float) t / 100.f;
    timer_down = (t > 0);
    coins      = 0;
    status     = GAME_NONE;

    game_server_free(file_name);

    /* Load SOL data. */

    if (!game_base_load(file_name))
        return (server_state = 0);

    if (!sol_load_vary(&vary, &game_base))
    {
        game_base_free(NULL);
        return (server_state = 0);
    }

    server_state = 1;

    /* Get SOL version. */

    version.x = 0;
    version.y = 0;

    for (i = 0; i < vary.base->dc; i++)
    {
        char *k = vary.base->av + vary.base->dv[i].ai;
        char *v = vary.base->av + vary.base->dv[i].aj;

        if (strcmp(k, "version") == 0)
            sscanf(v, "%d.%d", &version.x, &version.y);
    }

    input_init();

    game_tilt_init(&tilt);

    /* Initialize jump and goal states. */

    jump_e = 1;
    jump_b = 0;

    goal_e = e ? 1 : 0;

    /* Initialize the view (and put it at the ball). */

    game_view_fly(&view, &vary, 0.0f);

    view_k = 1.0f;

    view_time = 0.0f;
    view_fade = 0.0f;

    /* Initialize ball size tracking. */

    got_orig = 0;
    grow = 0;

    /* Initialize simulation. */

    sol_init_sim(&vary);

    /* Send initial update. */

    game_cmd_map(file_name, version.x, version.y);
    game_cmd_ups();
    game_cmd_timer();

    if (goal_e)
        game_cmd_goalopen();

    game_cmd_init_balls();
    game_cmd_init_items();

    game_cmd_updview();
    game_cmd_eou();

    /* Reset lockstep state. */

    lockstep_clr(&server_step);

    return server_state;
}

void game_server_free(const char *next)
{
    if (server_state)
    {
        sol_quit_sim();
        sol_free_vary(&vary);

        game_base_free(next);

        server_state = 0;
    }
}

/*---------------------------------------------------------------------------*/

static void game_update_view(float dt)
{
    float dc = view.dc * (jump_b > 0 ? 2.0f * fabsf(jump_dt - 0.5f) : 1.0f);
    float da = input_get_r() * dt * 90.0f;
    float k;

    float M[16], v[3], Y[3] = { 0.0f, 1.0f, 0.0f };
    float view_v[3];

    float spd = (float) cam_speed(input_get_c()) / 1000.0f;

    /* Track manual rotation time. */

    if (da == 0.0f)
    {
        if (view_time < 0.0f)
        {
            /* Transition time is influenced by activity time. */

            view_fade = CLAMP(VIEW_FADE_MIN, -view_time, VIEW_FADE_MAX);
            view_time = 0.0f;
        }

        /* Inactivity. */

        view_time += dt;
    }
    else
    {
        if (view_time > 0.0f)
        {
            view_fade = 0.0f;
            view_time = 0.0f;
        }

        /* Activity (yes, this is negative). */

        view_time -= dt;
    }

    /* Center the view about the ball. */

    v_cpy(view.c, vary.uv->p);

    view_v[0] = -vary.uv->v[0];
    view_v[1] =  0.0f;
    view_v[2] = -vary.uv->v[2];

    /* Compute view vector. */

    if (spd >= 0.0f)
    {
        /* Viewpoint chases ball position. */

        if (da == 0.0f)
        {
            float s;

            v_sub(view.e[2], view.p, view.c);
            v_nrm(view.e[2], view.e[2]);

            /* Gradually restore view vector convergence rate. */

            s = fpowf(view_time, 3.0f) / fpowf(view_fade, 3.0f);
            s = CLAMP(0.0f, s, 1.0f);

            v_mad(view.e[2], view.e[2], view_v, v_len(view_v) * spd * s * dt);
        }
    }
    else
    {
        /* View vector is given by view angle. */

        view.e[2][0] = fsinf(V_RAD(view.a));
        view.e[2][1] = 0.0;
        view.e[2][2] = fcosf(V_RAD(view.a));
    }

    /* Apply manual rotation. */

    if (da != 0.0f)
    {
        m_rot(M, Y, V_RAD(da));
        m_vxfm(v, M, view.e[2]);
        v_cpy(view.e[2], v);
    }

    /* Orthonormalize the new view reference frame. */

    v_crs(view.e[0], view.e[1], view.e[2]);
    v_crs(view.e[2], view.e[0], view.e[1]);
    v_nrm(view.e[0], view.e[0]);
    v_nrm(view.e[2], view.e[2]);

    /* Compute the new view position. */

    k = 1.0f + v_dot(view.e[2], view_v) / 10.0f;

    view_k = view_k + (k - view_k) * dt;

    if (view_k < 0.5f) view_k = 0.5;

    v_scl(v,    view.e[1], view.dp * view_k);
    v_mad(v, v, view.e[2], view.dz * view_k);
    v_add(view.p, v, vary.uv->p);

    /* Compute the new view center. */

    v_cpy(view.c, vary.uv->p);
    v_mad(view.c, view.c, view.e[1], dc);

    /* Note the current view angle. */

    view.a = V_DEG(fatan2f(view.e[2][0], view.e[2][2]));

    game_cmd_updview();
}

static void game_update_time(float dt, int b)
{
   /* The ticking clock. */

    if (b && timer_down)
    {
        timer -= dt;
        if (timer < 0.f)
            timer = 0.f;
    }
    else if (b)
    {
        timer += dt;
    }

    if (b) game_cmd_timer();
}

static int game_update_state(int bt)
{
    struct b_goal *zp;
    int hi;

    float p[3];

    /* Test for an item. */

    if (bt && (hi = sol_item_test(&vary, p, ITEM_RADIUS)) != -1)
    {
        struct v_item *hp = vary.hv + hi;

        game_cmd_pkitem(hi);

        grow_init(hp->t);

        if (hp->t == ITEM_COIN)
        {
            coins += hp->n;
            game_cmd_coins();
        }
        else if (hp->t == ITEM_CLOCK)
        {
            audio_play(AUD_CLOCK, 1.f);

            if (timer_down)
            {
                /* Clock items are only effective on timed levels. */

                int seconds = hp->n;

                game_update_time((float) -seconds, bt);

                /* FIXME: calling client from server is a bad idea. */
                incr_gained(seconds);
            }
        }

        audio_play(AUD_COIN, 1.f);

        /* Discard item. */

        hp->t = ITEM_NONE;
    }

    /* Test for a switch. */

    if (sol_swch_test(&vary, game_proxy_enq, 0) == SWCH_INSIDE)
        audio_play(AUD_SWITCH, 1.f);

    /* Test for a jump. */

    if (jump_e == 1 && jump_b == 0 && (sol_jump_test(&vary, jump_p, 0) ==
                                       JUMP_INSIDE))
    {
        jump_b  = 1;
        jump_e  = 0;
        jump_dt = 0.f;

        audio_play(AUD_JUMP, 1.f);

        game_cmd_jump(1);
    }
    if (jump_e == 0 && jump_b == 0 && (sol_jump_test(&vary, jump_p, 0) ==
                                       JUMP_OUTSIDE))
    {
        jump_e = 1;
        game_cmd_jump(0);
    }

    /* Test for a goal. */

    if (bt && goal_e && (zp = sol_goal_test(&vary, p, 0)))
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

    if (bt && (vary.base->vc == 0 || vary.uv[0].p[1] < vary.base->vv[0].p[1]))
    {
        audio_play(AUD_FALL, 1.0f);
        return GAME_FALL;
    }

    return GAME_NONE;
}

static int game_step(const float g[3], float dt, int bt)
{
    if (server_state)
    {
        float h[3];

        /* Smooth jittery or discontinuous input. */

        tilt.rx += (input_get_x() - tilt.rx) * dt / MAX(dt, input_get_s());
        tilt.rz += (input_get_z() - tilt.rz) * dt / MAX(dt, input_get_s());

        game_tilt_axes(&tilt, view.e);

        game_cmd_tiltaxes();
        game_cmd_tiltangles();

        grow_step(dt);

        game_tilt_grav(h, g, &tilt);

        if (jump_b > 0)
        {
            jump_dt += dt;

            /* Handle a jump. */

            if (jump_dt >= 0.5f)
            {
                /* Translate view at the exact instant of the jump. */

                if (jump_b == 1)
                {
                    float dp[3];

                    v_sub(dp,     jump_p, vary.uv->p);
                    v_add(view.p, view.p, dp);

                    jump_b = 2;
                }

                /* Translate ball and hold it at the destination. */

                v_cpy(vary.uv->p, jump_p);
            }

            if (jump_dt >= 1.0f)
                jump_b = 0;
        }
        else
        {
            /* Run the sim. */

            float b = sol_step(&vary, game_proxy_enq, h, dt, 0, NULL);

            /* Mix the sound of a ball bounce. */

            if (b > 0.5f)
            {
                float k = (b - 0.5f) * 2.0f;

                if (got_orig)
                {
                    if      (vary.uv->r > grow_orig) audio_play(AUD_BUMPL, k);
                    else if (vary.uv->r < grow_orig) audio_play(AUD_BUMPS, k);
                    else                             audio_play(AUD_BUMPM, k);
                }
                else audio_play(AUD_BUMPM, k);
            }
        }

        game_cmd_updball();

        game_update_view(dt);
        game_update_time(dt, bt);

        return game_update_state(bt);
    }
    return GAME_NONE;
}

static void game_server_iter(float dt)
{
    switch (status)
    {
    case GAME_GOAL: game_step(GRAVITY_UP, dt, 0); break;
    case GAME_FALL: game_step(GRAVITY_DN, dt, 0); break;

    case GAME_NONE:
        if ((status = game_step(GRAVITY_DN, dt, 1)) != GAME_NONE)
            game_cmd_status();
        break;
    }

    game_cmd_eou();
}

static struct lockstep server_step = { game_server_iter, DT };

void game_server_step(float dt)
{
    lockstep_run(&server_step, dt);
}

float game_server_blend(void)
{
    return lockstep_blend(&server_step);
}

/*---------------------------------------------------------------------------*/

void game_set_goal(void)
{
    audio_play(AUD_SWITCH, 1.0f);
    goal_e = 1;

    game_cmd_goalopen();
}

/*---------------------------------------------------------------------------*/

void game_set_x(float k)
{
    input_set_x(-ANGLE_BOUND * k);

    input_set_s(config_get_d(CONFIG_JOYSTICK_RESPONSE) * 0.001f);
}

void game_set_z(float k)
{
    input_set_z(+ANGLE_BOUND * k);

    input_set_s(config_get_d(CONFIG_JOYSTICK_RESPONSE) * 0.001f);
}

void game_set_ang(float x, float z)
{
    input_set_x(x);
    input_set_z(z);
}

void game_set_pos(int x, int y)
{
    const float range = ANGLE_BOUND * 2;

    input_set_x(input_get_x() + range * y / config_get_d(CONFIG_MOUSE_SENSE));
    input_set_z(input_get_z() + range * x / config_get_d(CONFIG_MOUSE_SENSE));

    input_set_s(config_get_d(CONFIG_MOUSE_RESPONSE) * 0.001f);
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
