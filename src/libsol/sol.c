#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>

#include <math.h>
#include <vec.h>

#include "sol.h"

#define TINY 0.000000005

/*--------------------------------------------------------------------*/

static unsigned char *sol_load_jpeg(const char *s, unsigned int *w,
                                                   unsigned int *h,
                                                   unsigned int *b)
{
	unsigned char *p = NULL;
	unsigned char *q = NULL;
	FILE *fin;

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	if ((fin = fopen(s, "r")))
	{
		jpeg_stdio_src(&cinfo, fin);
		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);

		p = (unsigned char *) malloc(cinfo.output_width *
		                             cinfo.output_height *
		                             cinfo.output_components);

		while (p && cinfo.output_scanline < cinfo.output_height)
		{
			q = p + cinfo.output_scanline *
			        cinfo.output_width    *
			        cinfo.output_components;

			jpeg_read_scanlines(&cinfo, &q, 1);
		}

		*w = (unsigned int) cinfo.output_width;
		*h = (unsigned int) cinfo.output_height;
		*b = (unsigned int) cinfo.output_components;

		jpeg_finish_decompress(&cinfo);

		fclose(fin);
	}

	jpeg_destroy_decompress(&cinfo);

	return p;
}

static void sol_load_imag(struct s_imag *xp)
{
	GLenum f = (xp->b == 1) ? GL_LUMINANCE : GL_RGB;

	glGenTextures(1, &xp->o);
	glBindTexture(GL_TEXTURE_2D, xp->o);

	glTexImage2D(GL_TEXTURE_2D, 0, xp->b, xp->w, xp->h, 0, f,
	             GL_UNSIGNED_BYTE, xp->p);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}

static int sol_load_textures(struct s_file *fp)
{
	int i;

	for (i = 0; i < fp->mc; i++)
	{
		struct s_mtrl *mp = fp->mv + i;
		struct s_imag *xp = fp->xv + i;

		if ((xp->p = sol_load_jpeg(mp->f, &xp->w,
		                          &xp->h, &xp->b)))
			sol_load_imag(xp);
	}

	return 1;
}

/*--------------------------------------------------------------------*/

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

		fp->xv = (struct s_imag *)
			calloc(n[0], sizeof (struct s_imag));
		fp->mv = (struct s_mtrl *)
			calloc(n[0], sizeof (struct s_mtrl));
		fp->vv = (struct s_vert *)
			calloc(n[1], sizeof (struct s_vert));
		fp->ev = (struct s_edge *)
			calloc(n[2], sizeof (struct s_edge));
		fp->sv = (struct s_side *)
			calloc(n[3], sizeof (struct s_side));
		fp->tv = (struct s_texc *)
			calloc(n[4], sizeof (struct s_texc));
		fp->gv = (struct s_geom *)
			calloc(n[5], sizeof (struct s_geom));
		fp->lv = (struct s_lump *)
			calloc(n[6], sizeof (struct s_lump));
		fp->nv = (struct s_node *)
			calloc(n[7], sizeof (struct s_node));
		fp->pv = (struct s_path *)
			calloc(n[8], sizeof (struct s_path));
		fp->bv = (struct s_body *)
			calloc(n[9], sizeof (struct s_body));
		fp->cv = (struct s_coin *)
			calloc(n[10],sizeof (struct s_coin));
		fp->uv = (struct s_ball *)
			calloc(n[11],sizeof (struct s_ball));
		fp->iv = (int *)
			calloc(n[12],sizeof (int));

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

		return sol_load_textures(fp);
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
                            const struct s_geom *gp, int b)
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

static void sol_render_lump(const struct s_file *fp,
                            const struct s_lump *lp, int b)
{
	int i;

	for (i = 0; i < lp->gc; i++)
	{
		int gi = fp->iv[lp->g0 + i];

		sol_render_geom(fp, fp->gv + gi, b);
	}
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

	sol_render_mtrl(fp, -1);

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

static void sol_bounce(struct s_ball *up, const double q[3],
                                          const double w[3])
{
	const double ke = 0.80;
	const double kr = 0.99;

	double n[3], r[3], d[3], vn, wn;
	double *p = up->p;
	double *v = up->v;

	v_sub(r, p, q);
	v_sub(d, v, w);
	v_nrm(n, r);

	v_crs(up->w, d, r);
	v_scl(up->w, up->w, -1.0 / (up->r * up->r));

	vn = v_dot(v, n);
	wn = v_dot(w, n);

	v_mad(v, v, n, -vn);
	v_scl(v, v, kr);
	v_mad(v, v, n, fabs(vn) * ke + wn); 
}

/*--------------------------------------------------------------------*/

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
		int i;

		for (i = 0; i < lp->sc; i++)
		{
			const struct s_side *sq
				= fp->sv + fp->iv[lp->s0 + i];

			if (sp != sq && v_dot(T, sq->n)
			              - v_dot(o, sq->n)
			              - v_dot(w, sq->n) * t > sq->d)
				return HUGE_VAL;
		}
	}
	return t;
}

/*--------------------------------------------------------------------*/

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

	while (tt > 0 && tt >= (nt = sol_test_file(T, V, up, fp)))
	{
		if (nt < TINY)
			nt = tt = 0.0;

		sol_body_step(fp, nt);
		sol_ball_step(fp, nt);
		sol_bounce(up, T, V);

		tt -= nt;
	}

	sol_body_step(fp, tt);
	sol_ball_step(fp, tt);

	v_mad(up->v, up->v, g, tt);
}

