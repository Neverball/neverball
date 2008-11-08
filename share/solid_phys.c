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
#include "geom.h" /* Only for height constants! */
#include "solid.h"
#include "solid_phys.h"

#define LARGE 1.0e+5f
#define SMALL 1.0e-3f

#define GAMMA 0.78f

/*---------------------------------------------------------------------------*/

static float erp(float t)
{
    return 3.0f * t * t - 2.0f * t * t * t;
}

static float derp(float t)
{
    return 6.0f * t     - 6.0f * t * t;
}

static void sol_body_v(float v[3],
                       const struct s_file *fp,
                       const struct s_body *bp)
{
    if (bp->pi >= 0 && fp->pv[bp->pi].f)
    {
        const struct s_path *pp = fp->pv + bp->pi;
        const struct s_path *pq = fp->pv + pp->pi;

        v_sub(v, pq->p, pp->p);
        v_scl(v, v, 1.0f / pp->t);

        if (pp->s)
            v_scl(v, v, derp(bp->t / pp->t));
    }
    else
    {
        v[0] = 0.0f;
        v[1] = 0.0f;
        v[2] = 0.0f;
    }
}

void sol_body_p(float p[3],
                const struct s_file *fp,
                const struct s_body *bp)
{
    float v[3];

    if (bp->pi >= 0)
    {
        const struct s_path *pp = fp->pv + bp->pi;
        const struct s_path *pq = fp->pv + pp->pi;

        if (pp->s)
        {
            v_sub(v, pq->p, pp->p);
            v_mad(p, pp->p, v, erp(bp->t / pp->t));
        }
        else
        {
            v_sub(v, pq->p, pp->p);
            v_mad(p, pp->p, v, bp->t / pp->t);
        }
    }
    else
    {
        p[0] = 0.0f;
        p[1] = 0.0f;
        p[2] = 0.0f;
    }
}

/*---------------------------------------------------------------------------*/

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

    du = v_dot(d, u);
    eu = v_dot(e, u);
    uu = v_dot(u, u);

    v_mad(P, d, u, -du / uu);
    v_mad(V, e, u, -eu / uu);

    t = v_sol(P, V, r);
    s = (du + eu * t) / uu;

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
 * Integrate the rotation of the given basis E under angular velocity W
 * through time DT.
 */
static void sol_rotate(float e[3][3], const float w[3], float dt)
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

/*
 * Compute the new  linear velocities of two colliding balls.
 * t gives the time after which they collide.
 */
static float sol_bounce_ball(struct s_ball *up,
                             struct s_ball *u2p,
                             const float t)
{
    float r_rel[3], u[3];
    float v_rel[3], v1_par[3], v1_perp[3], v2_par[3], v2_perp[3];
    float v11[3], v12[3], v21[3], v22[3];
    float *p1 = up->p, *v1 = up->v, *p2 = u2p->p, *v2 = u2p->v;
    float inertia, factor;

    /* Correct positions up to the collision */
    v_mad(p1, p1, v1, t);
    v_mad(p2, p2, v2, t);

    /* Floating point precision */
    if (!(p1[1] - p2[1] > 0.001f ) && !(p2[1] - p1[1] > 0.001f))
    {
        if (p1[1] > p2[1])
            p2[1] = p1[1];
        else
            p1[1] = p2[1];
    }

    v_sub(v_rel, v2, v1);
    if (v_len(v_rel) < 0.001f)
        return 0.0f;

    /* r_rel is the unit vector from p1 to p2 */
    v_sub(r_rel, p2, p1);
    v_nrm(r_rel, r_rel);

   /*
    * project velocities upon r_rel to get components parallel
    * to r_rel - only these will be changed in the collision
    */
    factor = v_dot(v1, r_rel);
    v_scl(v1_par, r_rel, factor);
    v_sub(v1_perp, v1, v1_par);

    factor = v_dot(v2, r_rel);
    v_scl(v2_par, r_rel, factor);
    v_sub(v2_perp, v2, v2_par);

    /* u is used to calculate the "energy" of the impact */
    v_sub(u, v2_par, v1_par);

   /*
    * New parallel velocities follow from momentum conservation,
    * coefficient of restitution GAMMA, mass ratio inertia
    */
    inertia = pow(up->r / u2p->r, 3);

    v_scl(v11, v1_par, (inertia - GAMMA) / (inertia + 1.0f));
    v_scl(v12, v1_par, (GAMMA + 1.0f) * inertia / (inertia + 1.0f));
    v_scl(v21, v2_par, (GAMMA + 1.0f) / (inertia + 1.0f));
    v_scl(v22, v2_par, (1.0f - GAMMA * inertia) / (inertia + 1.0f));
    v_add(v1_par, v11, v21);
    v_add(v2_par, v12, v22);

    v_add(v1, v1_par, v1_perp);
    v_add(v2, v2_par, v2_perp);

    /* Hack: stop stationary balls from spinning */
    if (v_len(v1) < 0.01f)
    {
        up->w[0] = 0.0f;
        up->w[1] = 0.0f;
        up->w[2] = 0.0f;
    }

    if (v_len(v2) < 0.01f)
    {
        u2p->w[0] = 0.0f;
        u2p->w[1] = 0.0f;
        u2p->w[2] = 0.0f;
    }

   /*
    * Return the length of the relative velocity parallel
    * to the line of impact
    */
    return fabsf(v_len(u));
}

/*
 * Compute the new angular velocity and orientation of a ball pendulum.
 * A gives the accelleration of the ball.  G gives the gravity vector.
 */
static void sol_pendulum(struct s_ball *up,
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

/*
 * Compute the states of all switches after DT seconds have passed.
 */
static void sol_swch_step(struct s_file *fp, float dt)
{
    int xi;

    for (xi = 0; xi < fp->xc; xi++)
    {
        struct s_swch *xp = fp->xv + xi;

        if (xp->t > 0)
        {
            xp->t -= dt;

            if (xp->t <= 0)
            {
                int pi = xp->pi;
                int pj = xp->pi;

                do  /* Tortoise and hare cycle traverser. */
                {
                    fp->pv[pi].f = xp->f0;
                    fp->pv[pj].f = xp->f0;

                    pi = fp->pv[pi].pi;
                    pj = fp->pv[pj].pi;
                    pj = fp->pv[pj].pi;
                }
                while (pi != pj);

                xp->f = xp->f0;
            }
        }
    }
}

/*
 * Compute the positions of all bodies after DT seconds have passed.
 */
static void sol_body_step(struct s_file *fp, float dt)
{
    int i;

    for (i = 0; i < fp->bc; i++)
    {
        struct s_body *bp = fp->bv + i;
        struct s_path *pp = fp->pv + bp->pi;

        if (bp->pi >= 0 && pp->f)
        {
            bp->t += dt;

            if (bp->t >= pp->t)
            {
                bp->t  = 0;
                bp->pi = pp->pi;
            }
        }
    }
}

/*
 * Compute the positions of all balls after DT seconds have passed.
 */
static void sol_ball_step(struct s_file *fp, float dt)
{
    int i;

    for (i = 0; i < fp->uc; i++)
    {
        struct s_ball *up = fp->uv + i;

        v_mad(up->p, up->p, up->v, dt);

        sol_rotate(up->e, up->w, dt);
    }
}

/*---------------------------------------------------------------------------*/

static float sol_test_ball(float dt,
                           const struct s_ball *up,
                           const struct s_ball *u2p)
{
    float P[3], V[3];
    float t = LARGE;

    v_sub(P, up->p, u2p->p);
    v_sub(V, up->v, u2p->v);

    if (v_dot(P, V) < 0.0f)
    {
        t = v_sol(P, V, up->r + u2p->r);
    }

    return (t > LARGE) ? (LARGE) : (t);
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
    float q[3];

    /* If the ball is not behind the plane, the test passes. */

    v_sub(q, up->p, o);

    if (v_dot(q, sp->n) - sp->d + up->r >= 0)
        return 1;

    /* If it's not behind the plane after DT seconds, the test passes. */

    v_mad(q, q, up->v, dt);

    if (v_dot(q, sp->n) - sp->d + up->r >= 0)
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
    float q[3];

    /* If the ball is not in front of the plane, the test passes. */

    v_sub(q, up->p, o);

    if (v_dot(q, sp->n) - sp->d - up->r <= 0)
        return 1;

    /* If it's not in front of the plane after DT seconds, the test passes. */

    v_mad(q, q, up->v, dt);

    if (v_dot(q, sp->n) - sp->d - up->r <= 0)
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
    float U[3], O[3], W[3], u, t = dt;

    const struct s_node *np = fp->nv + bp->ni;

    sol_body_p(O, fp, bp);
    sol_body_v(W, fp, bp);

    if ((u = sol_test_node(t, U, up, fp, np, O, W)) < t)
    {
        v_cpy(T, U);
        v_cpy(V, W);
        t = u;
    }
    return t;
}

static float sol_test_balls(float dt,
                            const struct s_file *fp)
{
    float t = 0.0f;
    int   i, j;

    for (i = 0; i < fp->uc; i++)
    {
        struct s_ball *up = fp->uv + i;

        if (!up->a)
            continue;

        for (j = i + 1; j < fp->uc; j++)
        {
            struct s_ball *u2p = fp->uv + j;

            if (up->r <= 0.001f || u2p->r <= 0.001f)
                continue;

            if (!u2p->a)
                continue;

            if (sol_test_ball(dt, up, u2p) < dt)
            {
                t = sol_bounce_ball(up, u2p, t);

                /* Set the aggressor */
                up->g  = j + 1;
                u2p->g = i + 1;
            }
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
    float l, b = 0.0f, nt = dt, p;
    int   i, c = 16;

    if ((p = sol_test_balls(nt, fp)) > 0.0f)
        l = p;
    else
        l = 0.f;

    for (i = 0; i < fp->uc; i++)
    {
        float P[3], V[3], v[3], r[3], a[3], d, e, tt = dt;

        if (i < fp->uc)
        {
            struct s_ball *up = fp->uv + i;

            if (!up->a)
                continue;

            /* If the ball is in contact with a surface, apply friction. */

            v_cpy(a, up->v);
            v_cpy(v, up->v);
            v_cpy(up->v, g);

            if (m && sol_test_file(tt, P, V, up, fp) < 0.0005f)
            {
                v_cpy(up->v, v);
                v_sub(r, P, up->p);

                if (v_len(up->v) - dt > 0.0f)
                    (*m)++;

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
                    }
                }
                else v_mad(up->v, v, g, tt);
            }
            else v_mad(up->v, v, g, tt);

            /* Test for collision. */

            while (tt && tt > (nt = sol_test_file(tt, P, V, up, fp)) && c > 0)
            {
                sol_body_step(fp, nt);
                sol_swch_step(fp, nt);
                sol_ball_step(fp, nt);

                tt -= nt;

                if (b < ((d = sol_bounce(up, P, V, nt))))
                    b = d;

                c--;
            }

            /* Apply the ball's accelleration to the pendulum. */

            v_sub(a, up->v, a);

            sol_pendulum(up, a, g, dt);
        }
    }

    if (!c && !ui)
        nt += dt;

    sol_body_step(fp, nt);
    sol_swch_step(fp, nt);
    sol_ball_step(fp, nt);

    return (b > p) ? (b) : (p);
}

/*---------------------------------------------------------------------------*/

struct s_item *sol_item_test(struct s_file *fp, float *p, float item_r)
{
    int hi, ui;

    for (hi = 0; hi < fp->hc; hi++)
    {
        for (ui = 0; ui < fp->uc; ui++)
        {
            const float *ball_p = fp->uv[ui].p;
            const float  ball_r = fp->uv[ui].r;
            float r[3];

            r[0] = ball_p[0] - fp->hv[hi].p[0];
            r[1] = ball_p[1] - fp->hv[hi].p[1];
            r[2] = ball_p[2] - fp->hv[hi].p[2];

            if (fp->hv[hi].t != ITEM_NONE && v_len(r) < ball_r + item_r)
            {
                p[0] = fp->hv[hi].p[0];
                p[1] = fp->hv[hi].p[1];
                p[2] = fp->hv[hi].p[2];

                return &fp->hv[hi];
            }
        }
    }
    return NULL;
}

int sol_goal_test(struct s_file *fp, float *p, int ui)
{
    const float *ball_p = fp->uv[ui].p;
    const float  ball_r = fp->uv[ui].r;
    int zi;

    if (!fp->uv[ui].a)
        return 0;

    for (zi = 0; zi < fp->zc; zi++)
    {
        float r[3];

        r[0] = ball_p[0] - fp->zv[zi].p[0];
        r[1] = ball_p[2] - fp->zv[zi].p[2];
        r[2] = 0;

        if (v_len(r) < fp->zv[zi].r - ball_r &&
            ball_p[1] > fp->zv[zi].p[1] &&
            ball_p[1] < fp->zv[zi].p[1] + GOAL_HEIGHT / 2)
        {
            if (p)
            {
                p[0] = fp->zv[zi].p[0];
                p[1] = fp->zv[zi].p[1];
                p[2] = fp->zv[zi].p[2];
            }

            return 1;
        }
    }
    return 0;
}

/*
 * Test and process the  event a ball  enters a jump.
 * Return 0 if no ball  enters a jump,  else returns
 * the ball's ui + 1.  p gets set to the destination.
 */
int sol_jump_test(struct s_file *fp, float *p)
{
    float l;
    int ui, ji, res = 0;

    for (ui = 0; ui < fp->uc; ui++)
    {
        for (ji = 0; ji < fp->jc; ji++)
        {
            const float *ball_p = fp->uv[ui].p;
            const float  ball_r = fp->uv[ui].r;
            float r[3];

            if (!fp->uv[ui].a)
                continue;

            r[0] = ball_p[0] - fp->jv[ji].p[0];
            r[1] = ball_p[2] - fp->jv[ji].p[2];
            r[2] = 0;

            l = v_len(r) - fp->jv[ji].r;
            if (l < 0 &&
                ball_p[1] > fp->jv[ji].p[1] &&
                ball_p[1] < fp->jv[ji].p[1] + JUMP_HEIGHT / 2)
            {
                if (l < -ball_r )
                {
                    if (((int)(pow(2, ui)) & (int)(fp->jv[ji].b)) == 0)
                    {
                        if (res >= 0)
                            res = ui + 1;
                        fp->jv[ji].b |= (int)pow(2, ui);

                        if (p)
                        {
                            p[0] = fp->jv[ji].q[0] + (ball_p[0] - fp->jv[ji].p[0]);
                            p[1] = fp->jv[ji].q[1] + (ball_p[1] - fp->jv[ji].p[1]);
                            p[2] = fp->jv[ji].q[2] + (ball_p[2] - fp->jv[ji].p[2]);
                        }
                    }
                }
            }
            else if (((int)(pow(2, ui)) & (int)(fp->jv[ji].b)) > 0)
            {
                fp->jv[ji].b &= ~((int)(pow(2, ui)));
                res = -1;
            }
        }

        if (res > 0)
            return res;
        else
            res = 0;
    }
    return res;
}

/*
 * Test and process the event the ball UI enters a switch. Return 1 if
 * a visible  switch is  activated, return 0  otherwise (no  switch is
 * activated or only invisible switches).
 */
int sol_swch_test(struct s_file *fp)
{
    int xi, ui, res = 0;

    for (xi = 0; xi < fp->xc; xi++)
    {
        struct s_swch *xp = fp->xv + xi;

        for (ui = 0; ui < fp->uc; ui++)
        {
            float l;
            float r[3];

            const float *ball_p  = fp->uv[ui].p;
            const float  ball_r  = fp->uv[ui].r;

            if (!fp->uv[ui].a)
                continue;

            if (xp->t0 == 0 || xp->f == xp->f0)
            {
                r[0] = ball_p[0] - xp->p[0];
                r[1] = ball_p[2] - xp->p[2];
                r[2] = 0;

                l = v_len(r) - xp->r;

                if (l < ball_r &&
                    ball_p[1] > xp->p[1] &&
                    ball_p[1] < xp->p[1] + SWCH_HEIGHT / 2)
                {
                    if (!xp->e && l < -ball_r)
                    {
                        int pi = xp->pi;
                        int pj = xp->pi;

                        /* The ball enters. */
                        xp->b = ui;

                        if (xp->t0 == 0)
                            xp->e = 1;

                        /* Toggle the state, update the path. */
                        xp->f = xp->f ? 0 : 1;

                        do  /* Tortoise and hare cycle traverser. */
                        {
                            fp->pv[pi].f = xp->f;
                            fp->pv[pj].f = xp->f;

                            pi = fp->pv[pi].pi;
                            pj = fp->pv[pj].pi;
                            pj = fp->pv[pj].pi;
                        }
                        while (pi != pj);

                        /* It toggled to non-default state, start the timer. */
                        if (xp->f != xp->f0)
                            xp->t  = xp->t0;

                        /* If visible, set the result. */
                        if (!xp->i)
                            res = 1;
                    }
                }

                /* The ball exits. */
                else if (xp->e && xp->b == ui)
                    xp->e = 0;
            }
        }
    }

    return res;
}

/*---------------------------------------------------------------------------*/
