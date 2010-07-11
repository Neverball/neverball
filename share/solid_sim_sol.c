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

#include "vec3.h"
#include "common.h"

#include "solid.h"
#include "solid_sim.h"
#include "solid_all.h"
#include "solid_cmd.h"

#define LARGE 1.0e+5f
#define SMALL 1.0e-3f

/*---------------------------------------------------------------------------*/
/* Solves (p + v * t) . (p + v * t) == r * r for smallest t.                 */

/*
 * Given vectors A = P, B = V * t, C = A + B, |C| = r, solve for
 * smallest t.
 *
 * Some useful dot product properties:
 *
 * 1) A . A = |A| * |A|
 * 2) A . (B + C) = A . B + A . C
 * 3) (r * A) . B = r * (A . B)
 *
 * Deriving a quadratic equation:
 *
 * C . C = r * r                                     (1)
 * (A + B) . (A + B) = r * r
 * A . (A + B) + B . (A + B) = r * r                 (2)
 * A . A + A . B + B . A + B . B = r * r             (2)
 * A . A + 2 * (A . B) + B . B = r * r
 * P . P + 2 * (P . V * t) + (V * t . V * t) = r * r
 * P . P + 2 * (P . V) * t + (V . V) * t * t = r * r (3)
 * (V . V) * t * t + 2 * (P . V) * t + P . P - r * r = 0
 *
 * This equation is solved using the quadratic formula.
 */

static float v_sol(const float p[3], const float v[3], float r)
{
    float a = v_dot(v, v);
    float b = v_dot(v, p) * 2.0f;
    float c = v_dot(p, p) - r * r;
    float d = b * b - 4.0f * a * c;

/* HACK: This seems to cause failures to detect low-velocity collision
         Yet, the potential division by zero below seems fine.
    if (fabsf(a) < SMALL) return LARGE;
*/

    if      (d < 0.0f) return LARGE;
    else if (d > 0.0f)
    {
        float t0 = 0.5f * (-b - fsqrtf(d)) / a;
        float t1 = 0.5f * (-b + fsqrtf(d)) / a;
        float t  = (t0 < t1) ? t0 : t1;

        return (t < 0.0f) ? LARGE : t;
    }
    else return -b * 0.5f / a;
}

/*---------------------------------------------------------------------------*/

/*
 * Compute the  earliest time  and position of  the intersection  of a
 * sphere and a vertex.
 *
 * The sphere has radius R and moves along vector V from point P.  The
 * vertex moves  along vector  W from point  Q in a  coordinate system
 * based at O.
 */
static float v_vert(float Q[3],
                    const float o[3],
                    const float q[3],
                    const float w[3],
                    const float p[3],
                    const float v[3], float r)
{
    float O[3], P[3], V[3];
    float t = LARGE;

    v_add(O, o, q);
    v_sub(P, p, O);
    v_sub(V, v, w);

    if (v_dot(P, V) < 0.0f)
    {
        t = v_sol(P, V, r);

        if (t < LARGE)
            v_mad(Q, O, w, t);
    }
    return t;
}

/*
 * Compute the  earliest time  and position of  the intersection  of a
 * sphere and an edge.
 *
 * The sphere has radius R and moves along vector V from point P.  The
 * edge moves along vector W from point Q in a coordinate system based
 * at O.  The edge extends along the length of vector U.
 */
static float v_edge(float Q[3],
                    const float o[3],
                    const float q[3],
                    const float u[3],
                    const float w[3],
                    const float p[3],
                    const float v[3], float r)
{
    float d[3], e[3];
    float P[3], V[3];
    float du, eu, uu, s, t;

    v_sub(d, p, o);
    v_sub(d, d, q);
    v_sub(e, v, w);

    /*
     * Think projections.  Vectors D, extending from the edge vertex Q
     * to the sphere,  and E, the relative velocity  of sphere wrt the
     * edge, are  made orthogonal to  the edge vector U.   Division of
     * the  dot products  is required  to obtain  the  true projection
     * ratios since U does not have unit length.
     */

    du = v_dot(d, u);
    eu = v_dot(e, u);
    uu = v_dot(u, u);

    v_mad(P, d, u, -du / uu);

    /* First, test for intersection. */

    if (v_dot(P, P) < r * r)
    {
        /* The sphere already intersects the line of the edge. */

        if (du < 0 || du > uu)
        {
            /*
             * The sphere is behind the endpoints of the edge, and
             * can't hit the edge without hitting the vertices first.
             */
            return LARGE;
        }

        /* The sphere already intersects the edge. */

        if (v_dot(P, e) >= 0)
        {
            /* Moving apart. */
            return LARGE;
        }

        v_nrm(P, P);
        v_mad(Q, p, P, -r);

        return 0;
    }

    v_mad(V, e, u, -eu / uu);

    t = v_sol(P, V, r);
    s = (du + eu * t) / uu; /* Projection of D + E * t on U. */

    if (0.0f <= t && t < LARGE && 0.0f < s && s < 1.0f)
    {
        v_mad(d, o, w, t);
        v_mad(e, q, u, s);
        v_add(Q, e, d);
    }
    else
        t = LARGE;

    return t;
}

/*
 * Compute  the earliest  time and  position of  the intersection  of a
 * sphere and a plane.
 *
 * The sphere has radius R and moves along vector V from point P.  The
 * plane  moves  along  vector  W.   The  plane has  normal  N  and  is
 * positioned at distance D from the origin O along that normal.
 */
static float v_side(float Q[3],
                    const float o[3],
                    const float w[3],
                    const float n[3], float d,
                    const float p[3],
                    const float v[3], float r)
{
    float vn = v_dot(v, n);
    float wn = v_dot(w, n);
    float t  = LARGE;

    if (vn - wn <= 0.0f)
    {
        float on = v_dot(o, n);
        float pn = v_dot(p, n);

        float u = (r + d + on - pn) / (vn - wn);
        float a = (    d + on - pn) / (vn - wn);

        if (0.0f <= u)
        {
            t = u;

            v_mad(Q, p, v, +t);
            v_mad(Q, Q, n, -r);
        }
        else if (0.0f <= a)
        {
            t = 0;

            v_mad(Q, p, v, +t);
            v_mad(Q, Q, n, -r);
        }
    }
    return t;
}

/*---------------------------------------------------------------------------*/

/*
 * Compute the new  linear and angular velocities of  a bouncing ball.
 * Q  gives the  position  of the  point  of impact  and  W gives  the
 * velocity of the object being impacted.
 */
static float sol_bounce(struct s_ball *up,
                        const float q[3],
                        const float w[3], float dt)
{
    float n[3], r[3], d[3], vn, wn;
    float *p = up->p;
    float *v = up->v;

    /* Find the normal of the impact. */

    v_sub(r, p, q);
    v_sub(d, v, w);
    v_nrm(n, r);

    /* Find the new angular velocity. */

    v_crs(up->w, d, r);
    v_scl(up->w, up->w, -1.0f / (up->r * up->r));

    /* Find the new linear velocity. */

    vn = v_dot(v, n);
    wn = v_dot(w, n);

    v_mad(v, v, n, 1.7 * (wn - vn));

    v_mad(p, q, n, up->r);

    /* Return the "energy" of the impact, to determine the sound amplitude. */

    return fabsf(v_dot(n, d));
}

/*---------------------------------------------------------------------------*/

static float sol_test_vert(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_vert *vp,
                           const float o[3],
                           const float w[3])
{
    return v_vert(T, o, vp->p, w, up->p, up->v, up->r);
}

static float sol_test_edge(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_edge *ep,
                           const float o[3],
                           const float w[3])
{
    float q[3];
    float u[3];

    v_cpy(q, fp->vv[ep->vi].p);
    v_sub(u, fp->vv[ep->vj].p,
          fp->vv[ep->vi].p);

    return v_edge(T, o, q, u, w, up->p, up->v, up->r);
}

static float sol_test_side(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_lump *lp,
                           const struct s_side *sp,
                           const float o[3],
                           const float w[3])
{
    float t = v_side(T, o, w, sp->n, sp->d, up->p, up->v, up->r);
    int i;

    if (t < dt)
        for (i = 0; i < lp->sc; i++)
        {
            const struct s_side *sq = fp->sv + fp->iv[lp->s0 + i];

            if (sp != sq &&
                v_dot(T, sq->n) -
                v_dot(o, sq->n) -
                v_dot(w, sq->n) * t > sq->d)
                return LARGE;
        }
    return t;
}

/*---------------------------------------------------------------------------*/

static int sol_test_fore(float dt,
                         const struct s_ball *up,
                         const struct s_side *sp,
                         const float o[3],
                         const float w[3])
{
    float q[3], d;

    /* If the ball is not behind the plane, the test passes. */

    v_sub(q, up->p, o);
    d = sp->d;

    if (v_dot(q, sp->n) - d + up->r >= 0)
        return 1;

    /* If it's not behind the plane after DT seconds, the test passes. */

    v_mad(q, q, up->v, dt);
    d += v_dot(w, sp->n) * dt;

    if (v_dot(q, sp->n) - d + up->r >= 0)
        return 1;

    /* Else, test fails. */

    return 0;
}

static int sol_test_back(float dt,
                         const struct s_ball *up,
                         const struct s_side *sp,
                         const float o[3],
                         const float w[3])
{
    float q[3], d;

    /* If the ball is not in front of the plane, the test passes. */

    v_sub(q, up->p, o);
    d = sp->d;

    if (v_dot(q, sp->n) - d - up->r <= 0)
        return 1;

    /* If it's not in front of the plane after DT seconds, the test passes. */

    v_mad(q, q, up->v, dt);
    d += v_dot(w, sp->n) * dt;

    if (v_dot(q, sp->n) - d - up->r <= 0)
        return 1;

    /* Else, test fails. */

    return 0;
}

/*---------------------------------------------------------------------------*/

static float sol_test_lump(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_lump *lp,
                           const float o[3],
                           const float w[3])
{
    float U[3] = { 0.0f, 0.0f, 0.0f };
    float u, t = dt;
    int i;

    /* Short circuit a non-solid lump. */

    if (lp->fl & L_DETAIL) return t;

    /* Test all verts */

    if (up->r > 0.0f)
        for (i = 0; i < lp->vc; i++)
        {
            const struct s_vert *vp = fp->vv + fp->iv[lp->v0 + i];

            if ((u = sol_test_vert(t, U, up, vp, o, w)) < t)
            {
                v_cpy(T, U);
                t = u;
            }
        }

    /* Test all edges */

    if (up->r > 0.0f)
        for (i = 0; i < lp->ec; i++)
        {
            const struct s_edge *ep = fp->ev + fp->iv[lp->e0 + i];

            if ((u = sol_test_edge(t, U, up, fp, ep, o, w)) < t)
            {
                v_cpy(T, U);
                t = u;
            }
        }

    /* Test all sides */

    for (i = 0; i < lp->sc; i++)
    {
        const struct s_side *sp = fp->sv + fp->iv[lp->s0 + i];

        if ((u = sol_test_side(t, U, up, fp, lp, sp, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }
    return t;
}

static float sol_test_node(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_node *np,
                           const float o[3],
                           const float w[3])
{
    float U[3], u, t = dt;
    int i;

    /* Test all lumps */

    for (i = 0; i < np->lc; i++)
    {
        const struct s_lump *lp = fp->lv + np->l0 + i;

        if ((u = sol_test_lump(t, U, up, fp, lp, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }

    /* Test in front of this node */

    if (np->ni >= 0 && sol_test_fore(t, up, fp->sv + np->si, o, w))
    {
        const struct s_node *nq = fp->nv + np->ni;

        if ((u = sol_test_node(t, U, up, fp, nq, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }

    /* Test behind this node */

    if (np->nj >= 0 && sol_test_back(t, up, fp->sv + np->si, o, w))
    {
        const struct s_node *nq = fp->nv + np->nj;

        if ((u = sol_test_node(t, U, up, fp, nq, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }

    return t;
}

static float sol_test_body(float dt,
                           float T[3], float V[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_body *bp)
{
    float U[3], O[3], E[4], W[3], A[3], u, t = dt;

    const struct s_node *np = fp->nv + bp->ni;

    sol_body_p(O, fp, bp->pi, bp->t);
    sol_body_v(W, fp, bp->pi, bp->t, dt);
    sol_body_e(E, fp, bp);
    sol_body_w(A, fp, bp);

    /*
     * For rotating bodies, rather than rotate every normal and vertex
     * of the body, we temporarily pretend the ball is rotating and
     * moving about a static body.
     */

    /*
     * Linear velocity of a point rotating about the origin:
     * v = w x p
     */

    if (E[0] != 1.0f || v_dot(A, A))
    {
        /* The body has a non-identity orientation or it is rotating. */

        struct s_ball ball = *up;
        float e[4], w[3], v[3];

        e[0] =  E[0];
        e[1] = -E[1];
        e[2] = -E[2];
        e[3] = -E[3];

        w[0] = -A[0];
        w[1] = -A[1];
        w[2] = -A[2];

        /* Transform position. */

        v_sub(v, up->p, O);
        q_rot(ball.p, e, v);

        /* Transform velocity. */

        v_sub(v, up->v, W);
        q_rot(ball.v, e, v);

        /* Also add the velocity from rotation. */

        v_crs(v, w, ball.p);
        v_add(ball.v, ball.v, v);

        /* Force the solver to work in body space. */

        v[0] = 0.0f;
        v[1] = 0.0f;
        v[2] = 0.0f;

        w[0] = 0.0f;
        w[1] = 0.0f;
        w[2] = 0.0f;

        if ((u = sol_test_node(t, U, &ball, fp, np, v, w)) < t)
        {
            float d[4];

            q_by_axisangle(d, A, v_len(A) * u);
            q_mul(e, E, d);
            q_nrm(e, e);

            q_rot(T, e, U);

            v_crs(V, A, T);
            v_add(V, V, W);

            v_add(T, O, T);
            v_mad(O, T, W, u);

            t = u;
        }
    }
    else
    {
        if ((u = sol_test_node(t, U, up, fp, np, O, W)) < t)
        {
            v_cpy(T, U);
            v_cpy(V, W);
            t = u;
        }
    }
    return t;
}

static float sol_test_file(float dt,
                           float T[3], float V[3],
                           const struct s_ball *up,
                           const struct s_file *fp)
{
    float U[3], W[3], u, t = dt;
    int i;

    for (i = 0; i < fp->bc; i++)
    {
        const struct s_body *bp = fp->bv + i;

        if ((u = sol_test_body(t, U, W, up, fp, bp)) < t)
        {
            v_cpy(T, U);
            v_cpy(V, W);
            t = u;
        }
    }
    return t;
}

/*---------------------------------------------------------------------------*/

/*
 * Step the physics forward DT  seconds under the influence of gravity
 * vector G.  If the ball gets pinched between two moving solids, this
 * loop might not terminate.  It  is better to do something physically
 * impossible than  to lock up the game.   So, if we make  more than C
 * iterations, punt it.
 */

float sol_step(struct s_file *fp, const float *g, float dt, int ui, int *m)
{
    float P[3], V[3], v[3], r[3], a[3], d, e, nt, b = 0.0f, tt = dt;
    int c = 16;

    union cmd cmd;

    sol_cmd_defer = 1;

    if (ui < fp->uc)
    {
        struct s_ball *up = fp->uv + ui;

        /* If the ball is in contact with a surface, apply friction. */

        v_cpy(a, up->v);
        v_cpy(v, up->v);
        v_cpy(up->v, g);

        if (m && sol_test_file(tt, P, V, up, fp) < 0.0005f)
        {
            v_cpy(up->v, v);
            v_sub(r, P, up->p);

            if ((d = v_dot(r, g) / (v_len(r) * v_len(g))) > 0.999f)
            {
                if ((e = (v_len(up->v) - dt)) > 0.0f)
                {
                    /* Scale the linear velocity. */

                    v_nrm(up->v, up->v);
                    v_scl(up->v, up->v, e);

                    /* Scale the angular velocity. */

                    v_sub(v, V, up->v);
                    v_crs(up->w, v, r);
                    v_scl(up->w, up->w, -1.0f / (up->r * up->r));
                }
                else
                {
                    /* Friction has brought the ball to a stop. */

                    up->v[0] = 0.0f;
                    up->v[1] = 0.0f;
                    up->v[2] = 0.0f;

                    (*m)++;
                }
            }
            else v_mad(up->v, v, g, tt);
        }
        else v_mad(up->v, v, g, tt);

        /* Test for collision. */

        while (c > 0 && tt > 0 && tt > (nt = sol_test_file(tt, P, V, up, fp)))
        {
            cmd.type       = CMD_STEP_SIMULATION;
            cmd.stepsim.dt = nt;
            sol_cmd_enq(&cmd);

            sol_body_step(fp, nt);
            sol_swch_step(fp, nt);
            sol_ball_step(fp, nt);

            tt -= nt;

            if (b < (d = sol_bounce(up, P, V, nt)))
                b = d;

            c--;
        }

        cmd.type       = CMD_STEP_SIMULATION;
        cmd.stepsim.dt = tt;
        sol_cmd_enq(&cmd);

        sol_body_step(fp, tt);
        sol_swch_step(fp, tt);
        sol_ball_step(fp, tt);

        /* Apply the ball's accelleration to the pendulum. */

        v_sub(a, up->v, a);

        sol_pendulum(up, a, g, dt);
    }

    sol_cmd_enq_deferred();
    sol_cmd_defer = 0;

    return b;
}

/*---------------------------------------------------------------------------*/

void sol_init_sim(struct s_file *fp)
{
    return;
}

void sol_quit_sim(void)
{
    return;
}

/*---------------------------------------------------------------------------*/
