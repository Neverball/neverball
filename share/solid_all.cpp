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

void sol_body_p(float p[3],
                const struct s_vary *vary,
                const struct v_body *bp,
                float dt)
{
    float v[3];

    if (bp->mi >= 0)
    {
        const struct v_move *mp = vary->mv + bp->mi;

        const struct b_path *pp = vary->base->pv + mp->pi;
        const struct b_path *pq = vary->base->pv + pp->pi;

        float s;

        if (vary->pv[mp->pi].f)
            s = (mp->t + dt) / pp->t;
        else
            s = mp->t / pp->t;

        v_sub(v, pq->p, pp->p);
        v_mad(p, pp->p, v, pp->s ? erp(s) : s);

        return;
    }

    p[0] = 0.0f;
    p[1] = 0.0f;
    p[2] = 0.0f;
}

void sol_body_v(float v[3],
                const struct s_vary *vary,
                const struct v_body *bp,
                float dt)
{
    if (bp->mi >= 0)
    {
        const struct v_move *mp = vary->mv + bp->mi;

        if (vary->pv[mp->pi].f)
        {
            float p[3], q[3];

            sol_body_p(p, vary, bp, 0.0f);
            sol_body_p(q, vary, bp, dt);

            v_sub(v, q, p);

            v[0] /= dt;
            v[1] /= dt;
            v[2] /= dt;

            return;
        }
    }

    v[0] = 0.0f;
    v[1] = 0.0f;
    v[2] = 0.0f;
}

void sol_body_e(float e[4],
                const struct s_vary *vary,
                const struct v_body *bp,
                float dt)
{
    if (bp->mj >= 0)
    {
        const struct v_move *mp = vary->mv + bp->mj;

        const struct b_path *pp = vary->base->pv + mp->pi;
        const struct b_path *pq = vary->base->pv + pp->pi;

        if (pp->fl & P_ORIENTED || pq->fl & P_ORIENTED)
        {
            float s;

            if (vary->pv[mp->pi].f)
                s = (mp->t + dt) / pp->t;
            else
                s = mp->t / pp->t;

            q_slerp(e, pp->e, pq->e, pp->s ? erp(s) : s);

            return;
        }
    }

    e[0] = 1.0f;
    e[1] = 0.0f;
    e[2] = 0.0f;
    e[3] = 0.0f;
}

/*
 * Determine if the body might be rotating.
 */
int sol_body_w(const struct s_vary *vary,
               const struct v_body *bp)
{
    if (bp->mj >= 0)
    {
        const struct v_move *mp = vary->mv + bp->mj;

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
    const float *ball_p = vary->uv->p;
    const float  ball_r = vary->uv->r;
    int hi;

    for (hi = 0; hi < vary->hc; hi++)
    {
        struct v_item *hp = vary->hv + hi;
        float r[3];

        v_sub(r, ball_p, hp->p);

        if (hp->t != ITEM_NONE && v_len(r) < ball_r + item_r)
        {
            p[0] = hp->p[0];
            p[1] = hp->p[1];
            p[2] = hp->p[2];

            return hi;
        }
    }
    return -1;
}

struct b_goal *sol_goal_test(struct s_vary *vary, float *p, int ui)
{
    const float *ball_p = vary->uv[ui].p;
    const float  ball_r = vary->uv[ui].r;
    int zi;

    for (zi = 0; zi < vary->base->zc; zi++)
    {
        struct b_goal *zp = vary->base->zv + zi;
        float r[3];

        r[0] = ball_p[0] - zp->p[0];
        r[1] = ball_p[2] - zp->p[2];
        r[2] = 0;

        if (v_len(r) + ball_r < zp->r &&
            ball_p[1] > zp->p[1] &&
            ball_p[1] < zp->p[1] + GOAL_HEIGHT / 2)
        {
            p[0] = zp->p[0];
            p[1] = zp->p[1];
            p[2] = zp->p[2];

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
    const float *ball_p = vary->uv[ui].p;
    const float  ball_r = vary->uv[ui].r;
    int ji, touch = 0;

    for (ji = 0; ji < vary->base->jc; ji++)
    {
        struct b_jump *jp = vary->base->jv + ji;
        float d, r[3];

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
    const float *ball_p = vary->uv[ui].p;
    const float  ball_r = vary->uv[ui].r;

    int xi, rc = SWCH_OUTSIDE;

    for (xi = 0; xi < vary->xc; xi++)
    {
        struct v_swch *xp = vary->xv + xi;

        /* FIXME enter/exit events don't work for timed switches */

        if (xp->base->t == 0 || xp->f == xp->base->f)
        {
            float d, r[3];

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

            if (d <= ball_r * 2 &&
                ball_p[1] > xp->base->p[1] &&
                ball_p[1] < xp->base->p[1] + SWCH_HEIGHT / 2)
            {
                if (!xp->e && d <= 0.0f)
                {
                    /* The ball enters. */

                    if (xp->base->tm == 0)
                    {
                        xp->e = 1;

                        if (cmd_func)
                        {
                            union cmd cmd = { CMD_SWCH_ENTER };
                            cmd.swchenter.xi = xi;
                            cmd_func(&cmd);
                        }
                    }

                    /* Toggle the state, update the path. */

                    xp->f = xp->f ? 0 : 1;

                    if (cmd_func)
                    {
                        union cmd cmd = { CMD_SWCH_TOGGLE };
                        cmd.swchtoggle.xi = xi;
                        cmd_func(&cmd);
                    }

                    sol_path_loop(vary, cmd_func, xp->base->pi, xp->f);

                    /* It toggled to non-default state, start the timer. */

                    if (xp->f != xp->base->f)
                    {
                        xp->t = 0.0f;
                        xp->tm = 0;
                    }

                    /* If visible, set the result. */

                    if (!xp->base->i)
                        rc = SWCH_INSIDE;
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
