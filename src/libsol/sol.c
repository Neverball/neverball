#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <vec.h>
#include <aux.h>

#include "sol.h"

#define MINDT 0.02

#define TINY 1.0e-10
#define HUGE 1.0e+10

/*---------------------------------------------------------------------------*/

static void sol_body_v(double v[3],
                       const struct s_file *fp,
                       const struct s_body *bp)
{
    if (bp->pi >= 0)
    {
        const struct s_path *pp = fp->pv + bp->pi;
        const struct s_path *pq = fp->pv + pp->pi;

        v_sub(v, pq->p, pp->p);
        v_scl(v, v, 1.0 / pp->t);
    }
    else
        v[0] = v[1] = v[2] = 0.0;
}

static void sol_body_p(double p[3],
                       const struct s_file *fp,
                       const struct s_body *bp)
{
    double v[3];

    if (bp->pi >= 0)
    {
        const struct s_path *pp = fp->pv + bp->pi;

        sol_body_v(v, fp, bp);

        v_mad(p, pp->p, v, bp->t);
    }
    else
        p[0] = p[1] = p[2] = 0.0;
}

/*---------------------------------------------------------------------------*/

static int transparent(const struct s_file *fp, int i)
{
    return (fp->mv[i].d[3] < 0.999) ? 1 : 0;
}

static void sol_render_mtrl(const struct s_file *fp, int i)
{
    static int c = -1;

    if (i == c) return;

    if (i >= 0)
    {
        const struct s_mtrl *mp = fp->mv + i;

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mp->a);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mp->d);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mp->s);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  mp->e);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mp->h);

        glBindTexture(GL_TEXTURE_2D, fp->xv[i].o);
    }

    c = i;
}

static void sol_render_geom(const struct s_file *fp,
                            const struct s_geom *gp, int b, int f)
{
    if ((f != 0 && gp->si == 0) || (f == 0 && gp->si != 0))
    {
        if (b == transparent(fp, gp->mi))
        {
            sol_render_mtrl(fp, gp->mi);
            
            glBegin(GL_POLYGON);
            {
                glNormal3dv  (fp->sv[gp->si].n);
                glTexCoord2dv(fp->tv[gp->ti].u);
                glVertex3dv  (fp->vv[gp->vi].p);
                glTexCoord2dv(fp->tv[gp->tj].u);
                glVertex3dv  (fp->vv[gp->vj].p);
                glTexCoord2dv(fp->tv[gp->tk].u);
                glVertex3dv  (fp->vv[gp->vk].p);
            }
            glEnd();
        }
    }
}

static void sol_render_lump(const struct s_file *fp,
                            const struct s_lump *lp, int b, int f)
{
    int i;

    for (i = 0; i < lp->gc; i++)
    {
        int gi = fp->iv[lp->g0 + i];

        sol_render_geom(fp, fp->gv + gi, b, f);
    }
}

static void sol_render_body(const struct s_file *fp,
                            const struct s_body *bp, int b, int f)
{
    int li;

    for (li = 0; li < bp->lc; li++)
        sol_render_lump(fp, fp->lv + bp->l0 + li, b, f);
}

/*---------------------------------------------------------------------------*/

static void sol_render_list(const struct s_file *fp,
                            const struct s_list *dp,
                            const struct s_body *bp, int b, int f)
{
    double p[3];

    glPushMatrix();
    {
        sol_body_p(p, fp, bp);
        glTranslated(p[0], p[1], p[2]);

        if (f)
            glCallList(dp->f);
        else
        {
            if (b)
                glCallList(dp->t);
            else
                glCallList(dp->o);
        }
    }
    glPopMatrix();
}


void sol_render(const struct s_file *fp, int f)
{
    int i;

    sol_render_mtrl(fp, -1);

    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    {
        for (i = 0; i < fp->bc; i++)
            sol_render_list(fp, fp->dv + i, fp->bv + i, 0, f);

        if (f == 0)
            glDepthMask(GL_FALSE);

        for (i = 0; i < fp->bc; i++)
            sol_render_list(fp, fp->dv + i, fp->bv + i, 1, f);
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static void sol_load_objects(struct s_file *fp)
{
    GLuint n = glGenLists(fp->bc * 3);
    int i;

    for (i = 0; i < fp->bc; i++)
    {
        fp->dv[i].o = n + i * 3 + 0;
        fp->dv[i].f = n + i * 3 + 1;
        fp->dv[i].t = n + i * 3 + 2;

        glNewList(fp->dv[i].o, GL_COMPILE);
        {
            sol_render_body(fp, fp->bv + i, 0, 0);
        }
        glEndList();

        glNewList(fp->dv[i].f, GL_COMPILE);
        {
            sol_render_body(fp, fp->bv + i, 0, 1);
            sol_render_body(fp, fp->bv + i, 1, 1);
        }
        glEndList();

        glNewList(fp->dv[i].t, GL_COMPILE);
        {
            sol_render_body(fp, fp->bv + i, 1, 0);
        }
        glEndList();
    }
}

static void sol_load_textures(struct s_file *fp)
{
    int i;

    for (i = 0; i < fp->mc; i++)
    {
        struct s_mtrl *mp = fp->mv + i;
        struct s_imag *xp = fp->xv + i;

        if ((xp->p = aux_load_jpg(mp->f, &xp->w, &xp->h, &xp->b)))
             xp->o = aux_make_tex(xp->p, xp->w, xp->h, xp->b);
    }
}

int sol_load(struct s_file *fp, const char *filename)
{
    FILE *fin;

    if ((fin = fopen(filename, "r")))
    {
        int n[13];

        memset(fp, 0, sizeof (struct s_file));

        fread(n, sizeof (int), 13, fin);

        fp->mc = n[0];
        fp->vc = n[1];
        fp->ec = n[2];
        fp->sc = n[3];
        fp->tc = n[4];
        fp->gc = n[5];
        fp->lc = n[6];
        fp->nc = n[7];
        fp->pc = n[8];
        fp->bc = n[9];
        fp->cc = n[10];
        fp->uc = n[11];
        fp->ic = n[12];

        fp->xv = (struct s_imag *) calloc(n[0],  sizeof (struct s_imag));
        fp->dv = (struct s_list *) calloc(n[9],  sizeof (struct s_list));

        fp->mv = (struct s_mtrl *) calloc(n[0],  sizeof (struct s_mtrl));
        fp->vv = (struct s_vert *) calloc(n[1],  sizeof (struct s_vert));
        fp->ev = (struct s_edge *) calloc(n[2],  sizeof (struct s_edge));
        fp->sv = (struct s_side *) calloc(n[3],  sizeof (struct s_side));
        fp->tv = (struct s_texc *) calloc(n[4],  sizeof (struct s_texc));
        fp->gv = (struct s_geom *) calloc(n[5],  sizeof (struct s_geom));
        fp->lv = (struct s_lump *) calloc(n[6],  sizeof (struct s_lump));
        fp->nv = (struct s_node *) calloc(n[7],  sizeof (struct s_node));
        fp->pv = (struct s_path *) calloc(n[8],  sizeof (struct s_path));
        fp->bv = (struct s_body *) calloc(n[9],  sizeof (struct s_body));
        fp->cv = (struct s_coin *) calloc(n[10], sizeof (struct s_coin));
        fp->uv = (struct s_ball *) calloc(n[11], sizeof (struct s_ball));
        fp->iv = (int           *) calloc(n[12], sizeof (int));

        fread(fp->mv, sizeof (struct s_mtrl), n[0],  fin);
        fread(fp->vv, sizeof (struct s_vert), n[1],  fin);
        fread(fp->ev, sizeof (struct s_edge), n[2],  fin);
        fread(fp->sv, sizeof (struct s_side), n[3],  fin);
        fread(fp->tv, sizeof (struct s_texc), n[4],  fin);
        fread(fp->gv, sizeof (struct s_geom), n[5],  fin);
        fread(fp->lv, sizeof (struct s_lump), n[6],  fin);
        fread(fp->nv, sizeof (struct s_node), n[7],  fin);
        fread(fp->pv, sizeof (struct s_path), n[8],  fin);
        fread(fp->bv, sizeof (struct s_body), n[9],  fin);
        fread(fp->cv, sizeof (struct s_coin), n[10], fin);
        fread(fp->uv, sizeof (struct s_ball), n[11], fin);
        fread(fp->iv, sizeof (int),           n[12], fin);

        fclose(fin);

        sol_load_textures(fp);
        sol_load_objects(fp);

        return 1;
    }
    return 0;
}

int sol_stor(struct s_file *fp, const char *filename)
{
    FILE *fout;

    if ((fout = fopen(filename, "w")))
    {
        int n[13];

        n[0]  = fp->mc;
        n[1]  = fp->vc;
        n[2]  = fp->ec;
        n[3]  = fp->sc;
        n[4]  = fp->tc;
        n[5]  = fp->gc;
        n[6]  = fp->lc;
        n[7]  = fp->nc;
        n[8]  = fp->pc;
        n[9]  = fp->bc;
        n[10] = fp->cc;
        n[11] = fp->uc;
        n[12] = fp->ic;

        fwrite(n, sizeof (int), 13, fout);

        fwrite(fp->mv, sizeof (struct s_mtrl), n[0],  fout);
        fwrite(fp->vv, sizeof (struct s_vert), n[1],  fout);
        fwrite(fp->ev, sizeof (struct s_edge), n[2],  fout);
        fwrite(fp->sv, sizeof (struct s_side), n[3],  fout);
        fwrite(fp->tv, sizeof (struct s_texc), n[4],  fout);
        fwrite(fp->gv, sizeof (struct s_geom), n[5],  fout);
        fwrite(fp->lv, sizeof (struct s_lump), n[6],  fout);
        fwrite(fp->nv, sizeof (struct s_node), n[7],  fout);
        fwrite(fp->pv, sizeof (struct s_path), n[8],  fout);
        fwrite(fp->bv, sizeof (struct s_body), n[9],  fout);
        fwrite(fp->cv, sizeof (struct s_coin), n[10], fout);
        fwrite(fp->uv, sizeof (struct s_ball), n[11], fout);
        fwrite(fp->iv, sizeof (int),           n[12], fout);

        fclose(fout);

        return 1;
    }
    return 0;
}

void sol_free(struct s_file *fp)
{
    int i;

    for (i = 0; i < fp->mc; i++)
        if (fp->xv[i].p) free(fp->xv[i].p);

    if (fp->xv) free(fp->xv);
    if (fp->dv) free(fp->dv);

    if (fp->mv) free(fp->mv);
    if (fp->vv) free(fp->vv);
    if (fp->ev) free(fp->ev);
    if (fp->sv) free(fp->sv);
    if (fp->tv) free(fp->tv);
    if (fp->gv) free(fp->gv);
    if (fp->lv) free(fp->lv);
    if (fp->nv) free(fp->nv);
    if (fp->pv) free(fp->pv);
    if (fp->bv) free(fp->bv);
    if (fp->cv) free(fp->cv);
    if (fp->uv) free(fp->uv);
    if (fp->iv) free(fp->iv);

    memset(fp, 0, sizeof (struct s_file));
}

/*---------------------------------------------------------------------------*/
/* Solves (p + v * t) . (p + v * t) == r * r for smallest t.                 */

static double v_sol(const double p[3], const double v[3], double r)
{
    double a = v_dot(v, v);
    double b = v_dot(v, p) * 2.0;
    double c = v_dot(p, p) - r * r;
    double d = b * b - 4.0 * a * c;

    if (d < 0.0)
        return HUGE;
    else
    {
        double t0 = 0.5 * (-b - sqrt(d)) / a;
        double t1 = 0.5 * (-b + sqrt(d)) / a;
        double t  = (t0 < t1) ? t0 : t1;

        return (t < 0.0) ? HUGE : t;
    }
}

/*---------------------------------------------------------------------------*/

static double v_vert(double Q[3],
                     const double o[3],
                     const double q[3],
                     const double w[3],
                     const double p[3],
                     const double v[3], double r)
{
    double O[3], P[3], V[3];
    double t = HUGE;

    v_add(O, o, q);
    v_sub(P, p, O);
    v_sub(V, v, w);

    if (v_dot(P, V) < 0.0)
    {
        t = v_sol(P, V, r);

        if (t < HUGE)
            v_mad(Q, O, w, t);
    }
    return t;
}

static double v_edge(double Q[3],
                     const double o[3],
                     const double q[3],
                     const double u[3],
                     const double w[3],
                     const double p[3],
                     const double v[3], double r)
{
    double d[3], e[3];
    double P[3], V[3];
    double du, eu, uu, s, t;

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

    if (0.0 < t && t < HUGE && 0.0 < s && s < 1.0)
    {
        v_mad(d, o, w, t);
        v_mad(e, q, u, s);
        v_add(Q, e, d);
    }
    else
        t = HUGE;

    return t;
}

static double v_side(double Q[3],
                     const double o[3],
                     const double w[3],
                     const double n[3], double d,
                     const double p[3],
                     const double v[3], double r)
{
    double on = v_dot(o, n);
    double pn = v_dot(p, n);
    double vn = v_dot(v, n);
    double wn = v_dot(w, n);
    double t  = HUGE;

    if (vn - wn < 0.0)
    {
        double u = (r + d + on - pn) / (vn - wn);

        if (0.0 < u)
        {
            t = u;

            v_mad(Q, p, v, +t);
            v_mad(Q, Q, n, -r);
        }
    }
    return t;
}

/*---------------------------------------------------------------------------*/

static double sol_bounce(struct s_ball *up,
                         const double q[3],
                         const double w[3])
{
    const double ke = 0.80;
    const double kr = 0.99;

    double n[3], r[3], d[3], e[3], vn, wn;
    double *p = up->p;
    double *v = up->v;

    v_sub(r, p, q);
    v_sub(d, v, w);
    v_nrm(n, r);
    v_nrm(e, d);

    v_crs(up->w, d, r);
    v_scl(up->w, up->w, -1.0 / (up->r * up->r));

    vn = v_dot(v, n);
    wn = v_dot(w, n);

    v_mad(v, v, n, -vn);
    v_scl(v, v, kr);
    v_mad(v, v, n, fabs(vn) * ke + wn); 

    return fabs(v_dot(n, d));
}

/*---------------------------------------------------------------------------*/

static void sol_body_step(struct s_file *fp, double dt)
{
    int i;

    if (dt > TINY)
        for (i = 0; i < fp->bc; i++)
        {
            struct s_body *bp = fp->bv + i;

            if (bp->pi >= 0)
            {
                bp->t += dt;

                if (bp->t > fp->pv[bp->pi].t)
                {
                    bp->t -= fp->pv[bp->pi].t;
                    bp->pi = fp->pv[bp->pi].pi;
                }
            }
        }
}

static void sol_ball_step(struct s_file *fp, double dt)
{
    int i;

    if (dt > TINY)
        for (i = 0; i < fp->uc; i++)
        {
            struct s_ball *up = fp->uv + i;

            v_mad(up->p, up->p, up->v, dt);

            if (v_len(up->w) > TINY)
            {
                double M[16];
                double w[3];
                double e[3][3];

                v_nrm(w, up->w);
                m_rot(M, w, v_len(up->w) * dt);

                m_vxfm(e[0], M, up->e[0]);
                m_vxfm(e[1], M, up->e[1]);
                m_vxfm(e[2], M, up->e[2]);

                v_crs(up->e[2], e[0], e[1]);
                v_crs(up->e[1], e[2], e[0]);
                v_crs(up->e[0], e[1], e[2]);

                v_nrm(up->e[0], up->e[0]);
                v_nrm(up->e[1], up->e[1]);
                v_nrm(up->e[2], up->e[2]);
            }
        }
}

/*---------------------------------------------------------------------------*/

static double sol_test_vert(double T[3],
                            const struct s_ball *up,
                            const struct s_file *fp,
                            const struct s_vert *vp,
                            const double o[3],
                            const double w[3])
{
    return v_vert(T, o, vp->p, w, up->p, up->v, up->r);
}

static double sol_test_edge(double T[3],
                            const struct s_ball *up,
                            const struct s_file *fp,
                            const struct s_edge *ep,
                            const double o[3],
                            const double w[3])
{
    double q[3];
    double u[3];

    v_cpy(q, fp->vv[ep->vi].p);
    v_sub(u, fp->vv[ep->vj].p,
             fp->vv[ep->vi].p);

    return v_edge(T, o, q, u, w, up->p, up->v, up->r);
}

static double sol_test_side(double T[3],
                            const struct s_ball *up,
                            const struct s_file *fp,
                            const struct s_lump *lp,
                            const struct s_side *sp,
                            const double o[3],
                            const double w[3])
{
    double t = v_side(T, o, w, sp->n, sp->d, up->p, up->v, up->r);
    int i;

    if (t < HUGE)
        for (i = 0; i < lp->sc; i++)
        {
            const struct s_side *sq = fp->sv + fp->iv[lp->s0 + i];

            if (sp != sq &&
                v_dot(T, sq->n) -
                v_dot(o, sq->n) -
                v_dot(w, sq->n) * t > sq->d)
                return HUGE;
        }
    return t;
}

/*---------------------------------------------------------------------------*/

static double sol_test_lump(double T[3],
                            const struct s_ball *up,
                            const struct s_file *fp,
                            const struct s_lump *lp,
                            const double o[3],
                            const double w[3])
{
    double U[3], u, t = HUGE;
    int i;

    /* Short circuit a non-solid lump. */

    if (lp->fl) return t;

    /* Test all verts */

    for (i = 0; i < lp->vc; i++)
    {
        const struct s_vert *vp = fp->vv + fp->iv[lp->v0 + i];

        if ((u = sol_test_vert(U, up, fp, vp, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }
 
   /* Test all edges */

    for (i = 0; i < lp->ec; i++)
    {
        const struct s_edge *ep = fp->ev + fp->iv[lp->e0 + i];

        if ((u = sol_test_edge(U, up, fp, ep, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }

    /* Test all sides */

    for (i = 0; i < lp->sc; i++)
    {
        const struct s_side *sp = fp->sv + fp->iv[lp->s0 + i];

        if ((u = sol_test_side(U, up, fp, lp, sp, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }
    return t;
}

static double sol_test_body(double T[3], double V[3],
                            const struct s_ball *up,
                            const struct s_file *fp,
                            const struct s_body *bp)
{
    double U[3], O[3], W[3], u, t = HUGE;
    int i;

    sol_body_p(O, fp, bp);
    sol_body_v(W, fp, bp);

    for (i = 0; i < bp->lc; i++)
    {
        const struct s_lump *lp = fp->lv + bp->l0 + i;

        if ((u = sol_test_lump(U, up, fp, lp, O, W)) < t)
        {
            v_cpy(T, U);
            v_cpy(V, W);
            t = u;
        }
    }
    return t;
}

static double sol_test_file(double T[3], double V[3],
                            const struct s_ball *up,
                            const struct s_file *fp)
{
    double U[3], W[3], u, t = HUGE;
    int i;

    for (i = 0; i < fp->bc; i++)
    {
        const struct s_body *bp = fp->bv + i;

        if ((u = sol_test_body(U, W, up, fp, bp)) < t)
        {
            v_cpy(T, U);
            v_cpy(V, W);
            t = u;
        }
    }
    return t;
}

/*---------------------------------------------------------------------------*/

/* If the ball makes several bumps over the course of a single dt, we note   */
/* the biggest one and play the bump sound effect once at that amplitude.    */

static void sol_bump(double *bump, double d)
{
    if (*bump < d)
        *bump = d;
}

/* Strictly speaking, the ball never rolls.  When it appears to be rolling   */
/* it is actually making many tiny bounces.  This complicates the process of */
/* determining when to play the roll sound effect.  As a hack, if the ball   */
/* goes N iterations without hitting anything, we assume the bounce is non-  */
/* small and we set the roll sound amplitude to zero.                        */

static void sol_roll(double *roll, int hit, struct s_ball *up)
{
    static int N = 10;
    static int c =  0;

    if (hit)
        c = N;

    if (--c > 0)
        *roll = v_len(up->w) * up->r;
    else
        *roll = 0.0;
}

/*---------------------------------------------------------------------------*/

void sol_update(struct s_file *fp, double dt, const double g[3],
                double *bump, double *roll)
{
    double T[3], V[3], nt, tt, d;

    struct s_ball *up = fp->uv;
    struct s_ball  uu = *up;

    tt = (dt < MINDT) ? dt : MINDT;

    v_mad(up->v, up->v, g, tt);

    *bump = 0.0;
    *roll = 0.0;

    while (tt > 0 && tt >= (nt = sol_test_file(T, V, up, fp)))
    {
        sol_body_step(fp, nt);
        sol_ball_step(fp, nt);

        tt -= nt;

        d = sol_bounce(up, T, V);

        sol_bump(bump, d);
        sol_roll(roll, 1, up);
    }

    sol_body_step(fp, tt);
    sol_ball_step(fp, tt);
    sol_roll(roll, 0, up);

    /* A minor hack here: if the ball moves less than a millimeter then zero */
    /* out any change to the linear and angular position.  This prevents the */
    /* ball from jittering while at rest, but can produce a discontinuity    */
    /* during subtle motion.                                                 */

    v_sub(V, uu.p, up->p);

    if (v_len(V) < 0.001)
    {
        v_cpy(up->p, uu.p);
        v_cpy(up->e[0], uu.e[0]);
        v_cpy(up->e[1], uu.e[1]);
        v_cpy(up->e[2], uu.e[2]);
    }
}

/*---------------------------------------------------------------------------*/

int  sol_inside(const struct s_file *fp)
{
    const struct s_ball *up = fp->uv;

    int i, j, k, n;
    double p[3];
    double P[3];

    for (i = 0; i < fp->bc; i++)
    {
        const struct s_body *bp = fp->bv + i;

        sol_body_p(p, fp, bp);
        v_sub(P, up->p, p);

        for (j = 0; j < bp->lc; j++)
        {
            const struct s_lump *lp = fp->lv + bp->l0 + j;

            if (lp->fl)
            {
                n = 0;

                for (k = 0; n == 0 && k < lp->sc; k++)
                {
                    const struct s_side *sp = fp->sv + fp->iv[lp->s0 + k];

                    if (v_dot(sp->n, P) - sp->d > -up->r)
                        n++;
                }

                if (n == 0) return bp->l0 + j;
            }
        }
    }

    return -1;
}

