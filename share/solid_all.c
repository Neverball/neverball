/*
 * Copyright (C) 2003-2010 Neverball authors
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


/* Random code used in more than one place. */

#include "solid_all.h"
#include "solid_vary.h"

#include "common.h"
#include "vec3.h"
#include "geom.h"

/*---------------------------------------------------------------------------*/

static float erp(float t)
{
    return 3.0f * t * t - 2.0f * t * t * t;
}

#if UNUSED
static float derp(float t)
{
    return 6.0f * t     - 6.0f * t * t;
}
#endif

/*
 * This here is a scene graph, believe it or not.
 */

struct vec3
{
    float x, y, z;
};

struct vec4
{
    float w, x, y, z;
};

static const struct vec3 POS_IDENTITY = { 0.0f, 0.0f, 0.0f };
static const struct vec4 ROT_IDENTITY = { 1.0f, 0.0f, 0.0f, 0.0f };

static struct vec3 get_move_pos(const struct s_vary *vary, int mi, float dt);
static struct vec3 get_path_pos(const struct s_vary *vary, int pi, float dt);

static struct vec4 get_move_rot(const struct s_vary *vary, int mi, float dt);
static struct vec4 get_path_rot(const struct s_vary *vary, int pi, float dt);

static struct vec3 get_move_pos(const struct s_vary *vary, int mi, float dt)
{
    if (mi < 0)
        return POS_IDENTITY;

    const struct v_move *mp = vary->mv + mi;

    int curr_pi = mp->pi;
    int next_pi = vary->base->pv[curr_pi].pi;

    struct vec3 p0 = get_path_pos(vary, curr_pi, dt);
    struct vec3 p1 = next_pi != curr_pi ? get_path_pos(vary, next_pi, dt) : p0;

    const struct b_path *pp = vary->base->pv + curr_pi;

    float s;

    if (vary->pv[curr_pi].f)
        s = (mp->t + dt) / pp->t;
    else
        s = mp->t / pp->t;

    s = pp->s ? erp(s) : s;

    float v[3];

    v_sub(v, (float *) &p1, (float *) &p0);
    v_mad(v, (float *) &p0, v, s);

    struct vec3 pos;

    pos.x = v[0];
    pos.y = v[1];
    pos.z = v[2];

    return pos;
}

static struct vec4 get_move_rot(const struct s_vary *vary, int mi, float dt)
{
    if (mi < 0)
        return ROT_IDENTITY;

    const struct v_move *mp = vary->mv + mi;

    int curr_pi = mp->pi;
    int next_pi = vary->base->pv[curr_pi].pi;

    struct vec4 e0 = get_path_rot(vary, curr_pi, dt);
    struct vec4 e1 = next_pi != curr_pi ? get_path_rot(vary, next_pi, dt) : e0;

    const struct b_path *pp = vary->base->pv + curr_pi;

    float s;

    if (vary->pv[curr_pi].f)
        s = (mp->t + dt) / pp->t;
    else
        s = mp->t / pp->t;

    s = pp->s ? erp(s) : s;

    struct vec4 rot;

    q_slerp((float *) &rot, (float *) &e0, (float *) &e1, s);

    return rot;
}

static struct vec3 get_path_pos(const struct s_vary *vary, int pi, float dt)
{
    if (pi < 0)
        return POS_IDENTITY;

    const struct v_path *vp = vary->pv + pi;
    const struct b_path *pp = vary->base->pv + pi;

    struct vec3 pos = get_move_pos(vary, vp->mi, dt);
    struct vec4 rot = get_move_rot(vary, vp->mj, dt);

    if (rot.w != 1.0f)
    {
        float p[3];
        q_rot(p, (float *) &rot, pp->p);
        v_add((float *) &pos, (float *) &pos, p);
    }
    else
        v_add((float *) &pos, (float *) &pos, pp->p);

    return pos;
}

static struct vec4 get_path_rot(const struct s_vary *vary, int pi, float dt)
{
    if (pi < 0)
        return ROT_IDENTITY;

    const struct v_path *vp = vary->pv + pi;
    const struct b_path *pp = vary->base->pv + pi;

    struct vec4 rot = get_move_rot(vary, vp->mj, dt);

    float e[4];

    q_mul(e, (float *) &rot, pp->e);

    rot.w = e[0];
    rot.x = e[1];
    rot.y = e[2];
    rot.z = e[3];

    return rot;
}

void sol_body_p(float p[3],
                const struct s_vary *vary,
                int mi,
                float dt)
{
    if (mi >= 0)
    {
        struct vec3 pos = get_move_pos(vary, mi, dt);

        p[0] = pos.x;
        p[1] = pos.y;
        p[2] = pos.z;
    }
    else
    {
        p[0] = 0.0f;
        p[1] = 0.0f;
        p[2] = 0.0f;
    }
}

void sol_body_v(float v[3],
                const struct s_vary *vary,
                int mi,
                float dt)
{
    if (dt > 0.0f)
    {
        float p[3], q[3];

        sol_body_p(p, vary, mi, 0.0f);
        sol_body_p(q, vary, mi, dt);

        v_sub(v, q, p);

        v[0] /= dt;
        v[1] /= dt;
        v[2] /= dt;
    }
    else
    {
        v[0] = 0.0f;
        v[1] = 0.0f;
        v[2] = 0.0f;
    }
}

void sol_body_e(float e[4],
                const struct s_vary *vary,
                int mi,
                float dt)
{
    if (mi >= 0)
    {
        const struct vec4 rot = get_move_rot(vary, mi, dt);

        e[0] = rot.w;
        e[1] = rot.x;
        e[2] = rot.y;
        e[3] = rot.z;
    }
    else
    {
        e[0] = 1.0f;
        e[1] = 0.0f;
        e[2] = 0.0f;
        e[3] = 0.0f;
    }
}

/*
 * Determine if the body might be rotating.
 */
int sol_body_w(const struct s_vary *vary, int mi)
{
    if (mi >= 0)
    {
        const struct v_move *mp = vary->mv + mi;

        if (vary->pv[mp->pi].f)
        {
            const struct b_path *pp = vary->base->pv + mp->pi;
            const struct b_path *pq = vary->base->pv + pp->pi;

            if (pp->fl & P_ORIENTED || pq->fl & P_ORIENTED)
                return 1;
        }
    }
    return 0;
}

void sol_entity_p(float p[3],
                  const struct s_vary *vary,
                  int mi, int mj)
{
    if (mj < 0)
        mj = mi;

    sol_body_p(p, vary, mi, 0.0f);
}

void sol_entity_e(float e[4], const struct s_vary *vary, int mi, int mj)
{
    if (mj < 0)
        mj = mi;

    sol_body_e(e, vary, mj, 0.0f);
}

/*
 * Transform a mover-space point into world space.
 */
void sol_entity_world(float w[3],
                      const struct s_vary *vary,
                      int mi, int mj,
                      const float v[3])
{
    float move_p[3], move_e[4];

    sol_entity_p(move_p, vary, mi, mj);
    sol_entity_e(move_e, vary, mi, mj);

    q_rot(w, move_e, v);
    v_add(w, move_p, w);
}

/*
 * Transform a world-space point into mover space.
 */
void sol_entity_local(float w[3],
                      const struct s_vary *vary,
                      int mi, int mj,
                      const float v[3])
{
    float move_p[3], move_e[4];

    sol_entity_p(move_p, vary, mi, mj);
    sol_entity_e(move_e, vary, mi, mj);

    v_sub(w, v, move_p);
    q_conj(move_e, move_e);
    q_rot(w, move_e, w);
}

/*---------------------------------------------------------------------------*/

/*
 * Integrate the rotation of the given basis E under angular velocity W
 * through time DT.
 */
void sol_rotate(float e[3][3], const float w[3], float dt)
{
    if (v_len(w) > 0.0f)
    {
        float a[3], M[16], f[3][3];

        /* Compute the rotation matrix. */

        v_nrm(a, w);
        m_rot(M, a, v_len(w) * dt);

        /* Apply it to the basis. */

        m_vxfm(f[0], M, e[0]);
        m_vxfm(f[1], M, e[1]);
        m_vxfm(f[2], M, e[2]);

        /* Re-orthonormalize the basis. */

        v_crs(e[2], f[0], f[1]);
        v_crs(e[1], f[2], f[0]);
        v_crs(e[0], f[1], f[2]);

        v_nrm(e[0], e[0]);
        v_nrm(e[1], e[1]);
        v_nrm(e[2], e[2]);
    }
}

/*---------------------------------------------------------------------------*/

/*
 * Compute the new angular velocity and orientation of a ball pendulum.
 * A gives the accelleration of the ball.  G gives the gravity vector.
 */
void sol_pendulum(struct v_ball *up,
                  const float a[3],
                  const float g[3], float dt)
{
    float v[3], A[3], F[3], r[3], Y[3], T[3] = { 0.0f, 0.0f, 0.0f };

    const float m  = 5.000f;
    const float ka = 0.500f;
    const float kd = 0.995f;

    /* Find the total force over DT. */

    v_scl(A, a,     ka);
    v_mad(A, A, g, -dt);

    /* Find the force. */

    v_scl(F, A, m / dt);

    /* Find the position of the pendulum. */

    v_scl(r, up->E[1], -up->r);

    /* Find the torque on the pendulum. */

    if (fabsf(v_dot(r, F)) > 0.0f)
        v_crs(T, F, r);

    /* Apply the torque and dampen the angular velocity. */

    v_mad(up->W, up->W, T, dt);
    v_scl(up->W, up->W,    kd);

    /* Apply the angular velocity to the pendulum basis. */

    sol_rotate(up->E, up->W, dt);

    /* Apply a torque turning the pendulum toward the ball velocity. */

    v_mad(v, up->v, up->E[1], v_dot(up->v, up->E[1]));
    v_crs(Y, v, up->E[2]);
    v_scl(Y, up->E[1], 2 * v_dot(Y, up->E[1]));

    sol_rotate(up->E, Y, dt);
}

/*---------------------------------------------------------------------------*/

static void sol_path_flag(struct s_vary *vary, cmd_fn cmd_func, int pi, int f)
{
    if (pi < 0 || pi >= vary->pc)
        return;

    if (vary->pv[pi].f == f)
        return;

    vary->pv[pi].f = f;

    if (cmd_func)
    {
        union cmd cmd = { CMD_PATH_FLAG };
        cmd.pathflag.pi = pi;
        cmd.pathflag.f = vary->pv[pi].f;
        cmd_func(&cmd);
    }
}

static void sol_path_loop(struct s_vary *vary, cmd_fn cmd_func, int p0, int f)
{
    int pi = p0;
    int pj = p0;
    int pk;

    if (p0 < 0 || p0 >= vary->pc)
        return;

    do  /* Tortoise and hare cycle traverser. */
    {
        sol_path_flag(vary, cmd_func, pi, f);

        pi = vary->base->pv[pi].pi;
        pj = vary->base->pv[pj].pi;
        pj = vary->base->pv[pj].pi;
    }
    while (pi != pj);

    /*
     * At this point, the indices point to a node in the loop, but we
     * still need to walk any remaining nodes in that loop. This is
     * essentially the second part of the tortoise and hare algorithm
     * which finds the start of the loop, although we only care about
     * walking the remaining nodes.
     */

    pj = p0;
    pk = pi;

    do
    {
        sol_path_flag(vary, cmd_func, pi, f);

        pi = vary->base->pv[pi].pi;
        pj = vary->base->pv[pj].pi;
    }
    while (pi != pj && pi != pk);
}

/*---------------------------------------------------------------------------*/

/*
 * Compute the states of all switches after DT seconds have passed.
 */
void sol_swch_step(struct s_vary *vary, cmd_fn cmd_func, float dt, int ms)
{
    int xi;

    for (xi = 0; xi < vary->xc; xi++)
    {
        struct v_swch *xp = vary->xv + xi;

        if (xp->tm < xp->base->tm)
        {
            xp->t += dt;
            xp->tm += ms;

            if (xp->tm >= xp->base->tm)
            {
                sol_path_loop(vary, cmd_func, xp->base->pi, xp->base->f);

                xp->f = xp->base->f;

                if (cmd_func)
                {
                    union cmd cmd = { CMD_SWCH_TOGGLE };
                    cmd.swchtoggle.xi = xi;
                    cmd_func(&cmd);
                }
            }
        }
    }
}

/*
 * Compute the positions of all movers after DT seconds have passed.
 */
void sol_move_step(struct s_vary *vary, cmd_fn cmd_func, float dt, int ms)
{
    int i;

    for (i = 0; i < vary->mc; i++)
    {
        struct v_move *mp = vary->mv + i;

        if (vary->pv[mp->pi].f)
        {
            struct v_path *pp = vary->pv + mp->pi;

            mp->t  += dt;
            mp->tm += ms;

            if (mp->tm >= pp->base->tm)
            {
                mp->t  = 0;
                mp->tm = 0;
                mp->pi = pp->base->pi;

                if (cmd_func)
                {
                    union cmd cmd;

                    cmd.type        = CMD_MOVE_TIME;
                    cmd.movetime.mi = i;
                    cmd.movetime.t  = mp->t;
                    cmd_func(&cmd);

                    cmd.type        = CMD_MOVE_PATH;
                    cmd.movepath.mi = i;
                    cmd.movepath.pi = mp->pi;
                    cmd_func(&cmd);
                }
            }
        }
    }
}

/*
 * Compute the positions of all balls after DT seconds have passed.
 */
void sol_ball_step(struct s_vary *vary, cmd_fn cmd_func, float dt)
{
    int i;

    for (i = 0; i < vary->uc; i++)
    {
        struct v_ball *up = vary->uv + i;

        v_mad(up->p, up->p, up->v, dt);

        sol_rotate(up->e, up->w, dt);
    }
}

/*---------------------------------------------------------------------------*/

int sol_item_test(struct s_vary *vary, float *p, float item_r)
{
    float ball_r = vary->uv[0].r;
    float ball_p[3];

    int hi;

    for (hi = 0; hi < vary->hc; hi++)
    {
        struct v_item *hp = vary->hv + hi;
        float r[3];

        /* Transform ball position into item space. */

        sol_entity_local(ball_p, vary, hp->mi, hp->mj, vary->uv[0].p);

        v_sub(r, ball_p, hp->p);

        if (hp->t != ITEM_NONE && v_len(r) < ball_r + item_r)
            return hi;
    }
    return -1;
}

struct b_goal *sol_goal_test(struct s_vary *vary, float *p, int ui)
{
    float ball_r = vary->uv[ui].r;
    float ball_p[3];

    int zi;

    for (zi = 0; zi < vary->base->zc; zi++)
    {
        struct b_goal *zp = vary->base->zv + zi;
        float r[3];

        /* Transform ball position into goal space. */

        sol_entity_local(ball_p, vary, vary->zv[zi].mi, vary->zv[zi].mj, vary->uv[ui].p);

        r[0] = ball_p[0] - zp->p[0];
        r[1] = ball_p[2] - zp->p[2];
        r[2] = 0;

        if (v_len(r) + ball_r < zp->r &&
            ball_p[1] > zp->p[1] &&
            ball_p[1] < zp->p[1] + GOAL_HEIGHT / 2)
        {
            return zp;
        }
    }
    return NULL;
}

/*
 * Test for a ball entering a teleporter.
 */
int sol_jump_test(struct s_vary *vary, float *p, int ui)
{
    float ball_r = vary->uv[ui].r;
    float ball_p[3];

    int ji, touch = 0;

    for (ji = 0; ji < vary->base->jc; ji++)
    {
        struct b_jump *jp = vary->base->jv + ji;
        float d, r[3];

        /* Transform ball position into jump space. */

        sol_entity_local(ball_p, vary, vary->jv[ji].mi, vary->jv[ji].mj, vary->uv[ui].p);

        r[0] = ball_p[0] - jp->p[0];
        r[1] = ball_p[2] - jp->p[2];
        r[2] = 0;

        /* Distance of the far side from the edge of the halo. */

        d = v_len(r) + ball_r - jp->r;

        /*
         * The "touch"  distance, which  must be cleared  before being
         * able to trigger a  teleporter, is the ball's radius.  (This
         * is different from switches.)
         */

        if (d <= ball_r &&
            ball_p[1] > jp->p[1] &&
            ball_p[1] < jp->p[1] + JUMP_HEIGHT / 2)
        {
            touch = 1;

            if (d <= 0.0f)
            {
                p[0] = jp->q[0] + (ball_p[0] - jp->p[0]);
                p[1] = jp->q[1] + (ball_p[1] - jp->p[1]);
                p[2] = jp->q[2] + (ball_p[2] - jp->p[2]);

                return JUMP_INSIDE;
            }
        }
    }
    return touch ? JUMP_TOUCH : JUMP_OUTSIDE;
}

/*
 * Test for a ball entering a switch.
 */
int sol_swch_test(struct s_vary *vary, cmd_fn cmd_func, int ui)
{
    float ball_r = vary->uv[ui].r;
    float ball_p[3];

    int xi, rc = SWCH_OUTSIDE;

    for (xi = 0; xi < vary->xc; xi++)
    {
        struct v_swch *xp = vary->xv + xi;

        float d, r[3];

        sol_entity_local(ball_p, vary, xp->mi, xp->mj, vary->uv[ui].p);

        r[0] = ball_p[0] - xp->base->p[0];
        r[1] = ball_p[2] - xp->base->p[2];
        r[2] = 0;

        /* Distance of the far side from the edge of the halo. */

        d = v_len(r) + ball_r - xp->base->r;

        /*
         * The  "touch"  distance, which  must  be cleared  before
         * being able to trigger a switch, is the ball's diameter.
         * (This is different from teleporters.)
         */

        if (xp->base->tm == 0) /* Behavior for toggle switches */
        {
            if (d <= ball_r * 2 &&
                ball_p[1] > xp->base->p[1] &&
                ball_p[1] < xp->base->p[1] + SWCH_HEIGHT / 2)
            {
                if (!xp->e && d <= 0.0f)
                {
                    /* The ball enters. */

                    xp->e = 1;

                    xp->f = xp->f ? 0 : 1;

                    if (cmd_func)
                    {
                        union cmd cmd;

                        cmd.type         = CMD_SWCH_ENTER;
                        cmd.swchenter.xi = xi;
                        cmd_func(&cmd);

                        cmd.type         = CMD_SWCH_TOGGLE;
                        cmd.swchtoggle.xi = xi;
                        cmd_func(&cmd);
                    }

                    sol_path_loop(vary, cmd_func, xp->base->pi, xp->f);

                    if (!xp->base->i)
                        rc = SWCH_INSIDE;

                }
            }
            else if (xp->e)
            {

                /* The ball exits. */

                xp->e = 0;

                if (cmd_func)
                {
                    union cmd cmd = { CMD_SWCH_EXIT };
                    cmd.swchexit.xi = xi;
                    cmd_func(&cmd);
                }
            }
        }
        else /* Behavior for timed switches */
        {
            if (d <= 0.0f &&
                ball_p[1] > xp->base->p[1] &&
                ball_p[1] < xp->base->p[1] + SWCH_HEIGHT / 2)
            {
                if (xp->e == 0)
                {
                    /* The ball enters. */
                    xp->e = 1;

                    if (cmd_func)
                    {
                        union cmd cmd = { CMD_SWCH_ENTER };
                        cmd.swchenter.xi = xi;
                        cmd_func(&cmd);
                    }
                }

                if (xp->base->tm == xp->tm) /* Timer is expired */
                {
                    /* Change switch state and start the timer. */

                    xp->f = xp->f ? 0 : 1;
                    if (cmd_func)
                    {
                        union cmd cmd = { CMD_SWCH_TOGGLE };
                        cmd.swchtoggle.xi = xi;
                        cmd_func(&cmd);
                    }
                    sol_path_loop(vary, cmd_func, xp->base->pi, xp->f);

                    if (!xp->base->i)
                        rc = SWCH_INSIDE;

                    xp->t = 0.0f;
                    xp->tm = 0;
                }
            }
            /* The ball exits. */

            else if (xp->e)
            {
                xp->e = 0;

                if (cmd_func)
                {
                    union cmd cmd = { CMD_SWCH_EXIT };
                    cmd.swchexit.xi = xi;
                    cmd_func(&cmd);
                }
            }
        }
    }
    return rc;
}

/*---------------------------------------------------------------------------*/
