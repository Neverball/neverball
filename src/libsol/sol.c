#include <GL/gl.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <vec.h>

#include "sol.h"

int sol_load(struct s_file *fp, FILE *fin)
{
	int n[12];

	memset(fp, 0, sizeof (struct s_file));

	fread(n, sizeof (int), 12, fin);

	fp->mc = n[0];
	fp->vc = n[1];
	fp->ec = n[2];
	fp->sc = n[3];
	fp->qc = n[4];
	fp->lc = n[5];
	fp->nc = n[6];
	fp->pc = n[7];
	fp->bc = n[8];
	fp->cc = n[9];
	fp->uc = n[10];
	fp->ic = n[11];

	fp->mv = (struct s_mtrl *) calloc(n[0], sizeof (struct s_mtrl));
	fp->vv = (struct s_vert *) calloc(n[1], sizeof (struct s_vert));
	fp->ev = (struct s_edge *) calloc(n[2], sizeof (struct s_edge));
	fp->sv = (struct s_side *) calloc(n[3], sizeof (struct s_side));
	fp->qv = (struct s_quad *) calloc(n[4], sizeof (struct s_quad));
	fp->lv = (struct s_lump *) calloc(n[5], sizeof (struct s_lump));
	fp->nv = (struct s_node *) calloc(n[6], sizeof (struct s_node));
	fp->pv = (struct s_path *) calloc(n[7], sizeof (struct s_path));
	fp->bv = (struct s_body *) calloc(n[8], sizeof (struct s_body));
	fp->cv = (struct s_coin *) calloc(n[9], sizeof (struct s_coin));
	fp->uv = (struct s_ball *) calloc(n[10],sizeof (struct s_ball));
	fp->iv = (int           *) calloc(n[11],sizeof (int));

	fread(fp->mv, sizeof (struct s_mtrl), n[0],  fin);
	fread(fp->vv, sizeof (struct s_vert), n[1],  fin);
	fread(fp->ev, sizeof (struct s_edge), n[2],  fin);
	fread(fp->sv, sizeof (struct s_side), n[3],  fin);
	fread(fp->qv, sizeof (struct s_quad), n[4],  fin);
	fread(fp->lv, sizeof (struct s_lump), n[5],  fin);
	fread(fp->nv, sizeof (struct s_node), n[6],  fin);
	fread(fp->pv, sizeof (struct s_path), n[7],  fin);
	fread(fp->bv, sizeof (struct s_body), n[8],  fin);
	fread(fp->cv, sizeof (struct s_coin), n[9],  fin);
	fread(fp->uv, sizeof (struct s_ball), n[10], fin);
	fread(fp->iv, sizeof (int),           n[11], fin);

	return 1;
}

int sol_stor(struct s_file *fp, FILE *fout)
{
	int n[12];

	n[0]  = fp->mc;
	n[1]  = fp->vc;
	n[2]  = fp->ec;
	n[3]  = fp->sc;
	n[4]  = fp->qc;
	n[5]  = fp->lc;
	n[6]  = fp->nc;
	n[7]  = fp->pc;
	n[8]  = fp->bc;
	n[9]  = fp->cc;
	n[10] = fp->uc;
	n[11] = fp->ic;

	fwrite(n, sizeof (int), 12, fout);

	fwrite(fp->mv, sizeof (struct s_mtrl), n[0],  fout);
	fwrite(fp->vv, sizeof (struct s_vert), n[1],  fout);
	fwrite(fp->ev, sizeof (struct s_edge), n[2],  fout);
	fwrite(fp->sv, sizeof (struct s_side), n[3],  fout);
	fwrite(fp->qv, sizeof (struct s_quad), n[4],  fout);
	fwrite(fp->lv, sizeof (struct s_lump), n[5],  fout);
	fwrite(fp->nv, sizeof (struct s_node), n[6],  fout);
	fwrite(fp->pv, sizeof (struct s_path), n[7],  fout);
	fwrite(fp->bv, sizeof (struct s_body), n[8],  fout);
	fwrite(fp->cv, sizeof (struct s_coin), n[9],  fout);
	fwrite(fp->uv, sizeof (struct s_ball), n[10], fout);
	fwrite(fp->iv, sizeof (int),           n[11], fout);

	return 1;
}

/*--------------------------------------------------------------------*/

static void sol_body_v(double v[3], const struct s_file *fp,
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
	{
		v[0] = 0.0;
		v[1] = 0.0;
		v[2] = 0.0;
	}
}

static void sol_body_p(double p[3], const struct s_file *fp,
                                    const struct s_body *bp)
{
	if (bp->pi >= 0)
	{
		const struct s_path *pp = fp->pv + bp->pi;
		double v[3];

		sol_body_v(v, fp, bp);

		v_mad(p, pp->p, v, bp->t);
	}
	else
	{
		p[0] = 0.0;
		p[1] = 0.0;
		p[2] = 0.0;
	}
}

/*--------------------------------------------------------------------*/

static int transparent(const struct s_mtrl *mp)
{
	return (mp->d[3] < 0.999) ? 1 : 0;
}

static void sol_render_mtrl(const struct s_mtrl *mp)
{
	static const struct s_mtrl *mq = NULL;

	if (mq == mp) return;

	if (mp)
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mp->d);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mp->s);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  mp->e);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mp->h);
	}
	mq = mp;
}

static void sol_render_quad(const struct s_file *fp,
                            const struct s_quad *qp, int b)
{
	const struct s_mtrl *mp = fp->mv + qp->mi;

	if (b == transparent(mp))
	{
		sol_render_mtrl(mp);

		glNormal3dv(fp->sv[qp->si].n);
		glVertex3dv(fp->vv[qp->vi].p);
		glVertex3dv(fp->vv[qp->vj].p);
		glVertex3dv(fp->vv[qp->vk].p);
		glVertex3dv(fp->vv[qp->vl].p);
	}
}

static void sol_render_lump(const struct s_file *fp,
                            const struct s_lump *lp, int b)
{
	int i;

	glBegin(GL_QUADS);
	{
		for (i = 0; i < lp->qc; i++)
		{
			int qi = fp->iv[lp->q0 + i];

			sol_render_quad(fp, fp->qv + qi, b);
		}
	}
	glEnd();
}

static void sol_render_body(const struct s_file *fp,
                            const struct s_body *bp, int b)
{
	int li;

	glPushMatrix();
	{
		double p[3];

		sol_body_p(p, fp, bp);
		glTranslated(p[0], p[1], p[2]);

		for (li = 0; li < bp->lc; li++)
			sol_render_lump(fp, fp->lv + bp->l0 + li, b);
	}
	glPopMatrix();
}

void sol_render(const struct s_file *fp)
{
	int i;

	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	{
		for (i = 0; i < fp->bc; i++)
			sol_render_body(fp, fp->bv + i, 0);

		glDepthMask(GL_FALSE);

		for (i = 0; i < fp->bc; i++)
			sol_render_body(fp, fp->bv + i, 1);
	}
	glPopAttrib();
}

/*--------------------------------------------------------------------*/
/* Solves (p + v * t) . (p + v * t) == r * r for smallest t.          */

static double v_sol(const double p[3], const double v[3], double r)
{
	double a = v_dot(v, v);
	double b = v_dot(v, p) * 2.0;
	double c = v_dot(p, p) - r * r;

	double d = b * b - 4.0 * a * c;

	if (d < 0.0)
		return HUGE_VAL;
	else
	{
		double t0 = 0.5 * (-b - sqrt(d)) / a;
		double t1 = 0.5 * (-b + sqrt(d)) / a;
	
		double t = (t0 < t1) ? t0 : t1;

		return (t < 0.0) ? HUGE_VAL : t;
	}
}

/*--------------------------------------------------------------------*/

static double v_vert(double Q[3], const double o[3],
                                  const double q[3],
                                  const double w[3],
                                  const double p[3],
                                  const double v[3], double r)
{
	double O[3], P[3], V[3];
	double t = HUGE_VAL;

	v_add(O, o, q);
	v_sub(P, p, O);
	v_sub(V, v, w);

	if (v_dot(P, V) < 0.0)
	{
		t = v_sol(P, V, r);

		if (t < HUGE_VAL)
			v_mad(Q, O, w, t);
	}
	return t;
}

static double v_edge(double Q[3], const double o[3],
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

	if (0.0 < t && t < HUGE_VAL && 0.0 < s && s < 1.0)
	{
		v_mad(d, o, w, t);
		v_mad(e, q, u, s);
		v_add(Q, e, d);
	}
	else t = HUGE_VAL;

	return t;
}

static double v_side(double Q[3], const double o[3],
                                  const double w[3],
                                  const double n[3], double d,
                                  const double p[3],
                                  const double v[3], double r)
{
	double on = v_dot(o, n);
	double pn = v_dot(p, n);
	double vn = v_dot(v, n);
	double wn = v_dot(w, n);
	double t  = HUGE_VAL;

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

/*--------------------------------------------------------------------*/

static void sol_bounce(struct s_ball *up, const double p[3],
                                          const double v[3])
{
	double n[3], r[3], w[3], V[3];

	v_sub(V, up->v, v);

	v_sub(r, p, up->p);
	v_nrm(n, r);
	v_crs(w, r, V);

	v_scl(up->w, w, -1.0 / (up->r * up->r));

	v_mad(up->v, up->v, n, -2.0 * v_dot(n, up->v) * 0.9);
	v_add(up->v, up->v, v);

	v_scl(up->v, up->v, 0.99);
}

/*--------------------------------------------------------------------*/

static void sol_body_step(struct s_file *fp, double dt)
{
	int i;

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

	for (i = 0; i < fp->uc; i++)
	{
		struct s_ball *up = fp->uv + i;

		v_mad(up->p, up->p, up->v, dt);

		if (v_len(up->w) > 0.00005)
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

/*--------------------------------------------------------------------*/

static double sol_test_vert(double T[3], const struct s_ball *up,
                                         const struct s_file *fp,
                                         const struct s_vert *vp,
                                         const double o[3],
                                         const double w[3])
{
	return v_vert(T, o, vp->p, w, up->p, up->v, up->r);
}

static double sol_test_edge(double T[3], const struct s_ball *up,
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

static double sol_test_side(double T[3], const struct s_ball *up,
                                         const struct s_file *fp,
                                         const struct s_lump *lp,
                                         const struct s_side *sp,
                                         const double o[3],
                                         const double w[3])
{
	double t = v_side(T, o, w, sp->n, sp->d, up->p, up->v, up->r);

	if (t < HUGE_VAL)
	{
		double P[3];
		double Q[3];
		int i;

		v_mad(P, o, w, t);
		v_sub(Q, T, P);

		for (i = 0; i < lp->sc; i++)
		{
			const struct s_side *sq
				= fp->sv + fp->iv[lp->s0 + i];

			if (sp != sq && v_dot(Q, sq->n) > sq->d)
				return HUGE_VAL;
		}
	}
	return t;
}

static double sol_test_lump(double T[3], const struct s_ball *up,
                                         const struct s_file *fp,
                                         const struct s_lump *lp,
                                         const double o[3],
                                         const double w[3])
{
	double U[3], u, t = HUGE_VAL;
	int i;

	for (i = 0; i < lp->vc; i++)
	{
		const struct s_vert *vp = fp->vv + fp->iv[lp->v0 + i];

		if ((u = sol_test_vert(U, up, fp, vp, o, w)) < t)
		{
			v_cpy(T, U);
			t = u;
		}
	}
	for (i = 0; i < lp->ec; i++)
	{
		const struct s_edge *ep = fp->ev + fp->iv[lp->e0 + i];

		if ((u = sol_test_edge(U, up, fp, ep, o, w)) < t)
		{
			v_cpy(T, U);
			t = u;
		}
	}
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
	double U[3], O[3], W[3], u, t = HUGE_VAL;
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
	double U[3], W[3], u, t = HUGE_VAL;
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

void sol_update(struct s_file *fp, double dt, const double g[3])
{
	double T[3], V[3], nt, tt = dt;

	struct s_ball *up = fp->uv;

	while (tt >= (nt = sol_test_file(T, V, up, fp)))
	{
		sol_body_step(fp, nt);
		sol_ball_step(fp, nt);
		sol_bounce(up, T, V);

		tt -= nt;
	}

	sol_body_step(fp, tt);
	sol_ball_step(fp, tt);

	v_mad(up->v, up->v, g, tt);
}

