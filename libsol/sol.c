#include <GL/gl.h>

#include <math.h>
#include <vec.h>

#include "sol.h"
#include "cube.h"

int sol_load(struct body *S)
{
	make_body(S);
	return 1;
}

/*--------------------------------------------------------------------*/

void sol_draw(struct body *S)
{
	int li, fi, vi, vj, vk;

	glBegin(GL_TRIANGLES);
	{
		for (li = 0; li < S->lc; li++)
			for (fi = 0; fi < S->lv[li].fc; fi++)
			{
				vi = S->lv[li].fv[fi].vi;
				vj = S->lv[li].fv[fi].vj;
				vk = S->lv[li].fv[fi].vk;

				glColor4ubv(S->lv[li].fv[fi].c);
				glNormal3dv(S->lv[li].fv[fi].n);

				glVertex3dv(S->lv[li].vv[vi].p);
				glVertex3dv(S->lv[li].vv[vj].p);
				glVertex3dv(S->lv[li].vv[vk].p);
			}
	}
	glEnd();
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

		return (t0 < t1) ? t0 : t1;
	}
}

/*--------------------------------------------------------------------*/

void sol_step(struct ball *A, const struct ball *B, double t)
{
	double M[16], w[3];

	v_mad(A->p, B->p, B->v, t);
	v_cpy(A->v, B->v);
	v_cpy(A->w, B->w);

	A->r = B->r;

	if (v_len(A->w) > 0.00005)
	{
		v_nrm(w, A->w);
		m_rot(M, w, v_len(A->w) * t);

		m_vxfm(A->e[0], M, B->e[0]);
		m_vxfm(A->e[1], M, B->e[1]);
		m_vxfm(A->e[2], M, B->e[2]);

		v_crs(A->e[2], A->e[0], A->e[1]);
		v_crs(A->e[1], A->e[2], A->e[0]);
		v_nrm(A->e[0], A->e[0]);
		v_nrm(A->e[1], A->e[1]);
		v_nrm(A->e[2], A->e[2]);
	}
	else
	{
		v_cpy(A->e[0], B->e[0]);
		v_cpy(A->e[1], B->e[1]);
		v_cpy(A->e[2], B->e[2]);
	}
}

static double sol_bump(struct ball *A, const struct ball *B,
		       const double p[3], double t)
{
	double n[3], r[3], w[3];

	sol_step(A, B, t);

	v_sub(r, p, A->p);
	v_nrm(n, r);
	v_crs(w, r, B->v);

	v_scl(A->w, w, -1.0 / (B->r * B->r));

	v_mad(A->v, A->v, n, -2.0 * v_dot(n, B->v) * 0.9);
	v_mad(A->v, A->v, A->v, -10.0 * t);

	return t;
}

/*--------------------------------------------------------------------*/

static double sol_vert(struct ball *A, const struct ball *B,
                                       const struct vert *V)
{
	double r[3];
	double t;

	if (v_dot(B->v, B->p) < v_dot(B->v, V->p))
	{
		v_sub(r, V->p, B->p);

		t = v_sol(r, B->p, B->r);

		if (0.0 < t && t < HUGE_VAL)
			return sol_bump(A, B, V->p, t);
	}
	return HUGE_VAL;
}

static double sol_edge(struct ball *A, const struct ball *B,
                                       const struct edge *E)
{
	double d[3];
	double p[3];
	double v[3];
	double ww, wd, wv;
	double t,  u;

	v_sub(d, B->p, E->p);

	ww = v_dot(E->v, E->v);
	wd = v_dot(E->v,    d) / ww;
	wv = v_dot(E->v, B->v) / ww;

	v_mad(p,    d, E->v, -wd);
	v_mad(v, B->v, E->v, -wv);

	t = v_sol(p, v, B->r);
	u = wd + wv * t;

	if (0.0 < t && 0.0 < u && u < 1.0)
	{
		v_mad(p, E->p, E->v, u);

		return sol_bump(A, B, p, t);
	}
	return HUGE_VAL;
}

static double sol_side(struct ball *A, const struct ball *B,
                                       const struct side *F,
                                       int i, int n)
{
	int j;
	double p[3];
	double t;
	double pn = v_dot(B->p, F[i].n);
	double vn = v_dot(B->v, F[i].n);

	if (vn < 0.0 /*&& 0.0 < pn*/)
	{
		t = (B->r + F[i].d - pn) / vn;

		if (0.0 < t)
		{
			v_mad(p, B->p, B->v,    t);
			v_mad(p,    p, F[i].n, -B->r);

			for (j = 0; j < n; j++)
				if (j != i && v_dot(p, F[j].n) > F[j].d)
					return HUGE_VAL;

			return sol_bump(A, B, p, t);
		}
	}
	return HUGE_VAL;
}

static double sol_lump(struct ball *A, const struct ball *B,
                                       const struct lump *L)
{
	int i;
	double s;
	double t = HUGE_VAL;
	struct ball T;

	for (i = 0; i < L->vc; i++)
		if ((s = sol_vert(&T, B, L->vv + i)) < t)
		{
			 t = s;
			*A = T;
		}
	for (i = 0; i < L->ec; i++)
		if ((s = sol_edge(&T, B, L->ev + i)) < t)
		{
			 t = s;
			*A = T;
		}
	for (i = 0; i < L->sc; i++)
		if ((s = sol_side(&T, B, L->sv, i, L->sc)) < t)
		{
			 t = s;
			*A = T;
		}

	return t;
}

double sol_ball(struct ball *A, const struct ball *B,
                                const struct body *S)
{
	int i;
	double s;
	double t = HUGE_VAL;
	struct ball T;

	for (i = 0; i < S->lc; i++)
		if ((s = sol_lump(&T, B, S->lv + i)) < t)
		{
			 t = s;
			*A = T;
		}

	return t;
}

/*--------------------------------------------------------------------*/

