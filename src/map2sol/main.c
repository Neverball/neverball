#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sol.h>
#include <vec.h>

#define MAXSTR 64
#define MAXKEY 16
#define SCALE  64.0
#define SMALL  0.000005

/* The overall design of this map converter is very stupid, but very  */
/* simple.  It begins by assuming that every mtrl, vert, edge, quad,  */
/* and side in the map is unique.  It then makes an optimizing pass   */
/* which discards all redundant information.  The result is optimal,  */
/* though the process is terribly inefficient.                        */


/*--------------------------------------------------------------------*/

/* Arbitrary limits woo! */

#define MAXM	1024
#define MAXV	4096
#define MAXE	4096
#define MAXS	2048
#define MAXQ	4096
#define MAXL	1024
#define MAXN	1024
#define MAXP	512
#define MAXB	512
#define MAXC	1024
#define MAXU	16
#define MAXI	16384

static void init_sol(struct s_file *fp)
{
	fp->mc = 0;
	fp->vc = 0;
	fp->ec = 0;
	fp->sc = 0;
	fp->qc = 0;
	fp->lc = 0;
	fp->nc = 0;
	fp->pc = 0;
	fp->bc = 0;
	fp->cc = 0;
	fp->uc = 0;
	fp->ic = 0;

	/* This is Linux.  Alloc never fails, a process just dies.  */

	fp->mv = (struct s_mtrl *) calloc(MAXM, sizeof (struct s_mtrl));
	fp->vv = (struct s_vert *) calloc(MAXV, sizeof (struct s_vert));
	fp->ev = (struct s_edge *) calloc(MAXE, sizeof (struct s_edge));
	fp->sv = (struct s_side *) calloc(MAXS, sizeof (struct s_side));
	fp->qv = (struct s_quad *) calloc(MAXQ, sizeof (struct s_quad));
	fp->lv = (struct s_lump *) calloc(MAXL, sizeof (struct s_lump));
	fp->nv = (struct s_node *) calloc(MAXN, sizeof (struct s_node));
	fp->pv = (struct s_path *) calloc(MAXP, sizeof (struct s_path));
	fp->bv = (struct s_body *) calloc(MAXB, sizeof (struct s_body));
	fp->cv = (struct s_coin *) calloc(MAXC, sizeof (struct s_coin));
	fp->uv = (struct s_ball *) calloc(MAXU, sizeof (struct s_ball));
	fp->iv = (int           *) calloc(MAXI, sizeof (int));
}

/*--------------------------------------------------------------------*/

/* The following is a small symbol table data structure.  Symbols and */
/* their integer values are collected in symv and valv.  References   */
/* and pointers to their unsatisfied integer values are collected in  */
/* refv and pntv.  Resolve matches references to symbols and fills    */
/* waiting locations with the proper values.                          */

#define MAXSYM 1024

static char symv[MAXSYM][MAXSTR];
static int  valv[MAXSYM];

static char refv[MAXSYM][MAXSTR];
static int *pntv[MAXSYM];

static int   strc = 0;
static int   refc = 0;

static void make_sym(const char *s, int  v)
{
	strcpy(symv[strc], s);
	valv[strc] = v;
	strc++;
}

static void make_ref(const char *r, int *p)
{
	strcpy(refv[refc], r);
	pntv[refc] = p;
	refc++;
}

static void resolve(void)
{
	int i, j;

	for (i = 0; i < refc; i++)
		for (j = 0; j < strc; j++)
			if (strcmp(refv[i], symv[j]) == 0)
			{
				*(pntv[i]) = valv[j];
				break;
			}
}

/*--------------------------------------------------------------------*/

#define T_EOF 0
#define T_BEG 1
#define T_CLP 2
#define T_KEY 3
#define T_END 4
#define T_NOP 5

static int map_token(FILE *fin, double n[3], double *d,
                     char key[MAXSTR], char val[MAXSTR])
{
	char buf[MAXSTR];

	if (fgets(buf, MAXSTR, fin))
	{
		char c;
		int x0, y0, z0;
		int x1, y1, z1;
		int x2, y2, z2;

		/* Scan the beginning or end of a block. */

		if (buf[0] == '{') return T_BEG;
		if (buf[0] == '}') return T_END;

		/* Scan a key-value pair. */

		if (buf[0] == '"')
		{
			strcpy(key, strtok(buf,  "\""));
			(void)      strtok(NULL, "\"");
			strcpy(val, strtok(NULL, "\""));

			return T_KEY;
		}

		/* Scan a plane, compute its normal and distance. */

		if (sscanf(buf, "%c %d %d %d %c "
                                "%c %d %d %d %c "
                                "%c %d %d %d %c %s",
                                &c, &x0, &y0, &z0, &c,
                                &c, &x1, &y1, &z1, &c,
                                &c, &x2, &y2, &z2, &c, key) == 16)
		{
			double p0[3];
			double p1[3];
			double p2[3];
			double u[3];
			double v[3];

			p0[0] = +(double) x0 / SCALE;
			p0[1] = +(double) z0 / SCALE;
			p0[2] = -(double) y0 / SCALE;

			p1[0] = +(double) x1 / SCALE;
			p1[1] = +(double) z1 / SCALE;
			p1[2] = -(double) y1 / SCALE;

			p2[0] = +(double) x2 / SCALE;
			p2[1] = +(double) z2 / SCALE;
			p2[2] = -(double) y2 / SCALE;

			v_sub(u, p2, p0);
			v_sub(v, p1, p0);
			v_crs(n, u, v);
			v_nrm(n, n);
			
			*d = v_dot(n, p0);

			return T_CLP;
		}

		/* If it's not recognized, it must be uninteresting. */

		return T_NOP;
	}
	return T_EOF;
}

/*--------------------------------------------------------------------*/

/* Read the given material file, adding a new material to the solid.  */

static void read_mtrl(struct s_file *fp, const char *s)
{
	struct s_mtrl *mp = fp->mv + fp->mc++;
	FILE *fin;

	if ((fin = fopen(s, "r")))
	{
		fscanf(fin, "%f %f %f %f "
		            "%f %f %f %f "
		            "%f %f %f %f "
		            "%f ",
		            mp->d, mp->d + 1, mp->d + 2, mp->d + 3,
		            mp->s, mp->s + 1, mp->s + 2, mp->s + 3,
		            mp->e, mp->e + 1, mp->e + 2, mp->e + 3,
		            mp->h);
		fclose(fin);
	}
}

/* Parse a lump from the given file and add it to the solid.  Note a  */
/* small hack here in mapping materials onto sides.  Material indices */
/* cannot be assigned until faces are computed, so for now there is   */
/* assumed to be exactly one material per side, and that a side index */
/* equals that side's material index.  See clip_lump and clip_quad.   */

static void read_lump(struct s_file *fp, FILE *fin)
{
	double n[3], d;
	char k[MAXSTR];
	char v[MAXSTR];
	int t;

	struct s_lump *lp = fp->lv + fp->lc++;

	lp->s0 = fp->ic;

	while ((t = map_token(fin, n, &d, k, v)))
	{
		if (t == T_CLP)
		{
			fp->sv[fp->sc].n[0] = n[0];
			fp->sv[fp->sc].n[1] = n[1];
			fp->sv[fp->sc].n[2] = n[2];
			fp->sv[fp->sc].d    = d;

			read_mtrl(fp, k);

			fp->iv[fp->ic] = fp->sc;
			fp->ic++;
			fp->sc++;
			lp->sc++;
		}
		if (t == T_END)
			break;
	}
}

/*--------------------------------------------------------------------*/

static void make_path(struct s_file *fp, char k[][MAXSTR],
                                         char v[][MAXSTR], int c)
{
	int i, pi = fp->pc++;

	struct s_path *pp = fp->pv + pi;

	pp->p[0] = 0.0;
	pp->p[1] = 0.0;
	pp->p[2] = 0.0;
	pp->t    = 1.0;
	pp->pi   = pi;

	for (i = 0; i < c; i++)
	{
		if (!strcmp(k[i], "targetname"))
			make_sym(v[i], pi);

		if (!strcmp(k[i], "target"))
			make_ref(v[i], &pp->pi);

		if (!strcmp(k[i], "speed"))
		{
			sscanf(v[i], "%lf", &pp->t);
		}
		if (!strcmp(k[i], "origin"))
		{
			int x = 0, y = 0, z = 0;

			sscanf(v[i], "%d %d %d", &x, &y, &z);

			pp->p[0] = +(double) x / SCALE;
			pp->p[1] = +(double) z / SCALE;
			pp->p[2] = -(double) y / SCALE;
		}
	}
}

static void make_body(struct s_file *fp, char k[][MAXSTR],
                                         char v[][MAXSTR],
                                         int c, int l0)
{
	int i, bi = fp->bc++;

	struct s_body *bp = fp->bv + bi;

	bp->t  = 0.0;
	bp->pi = -1;
	bp->ni = -1;
	bp->l0 = l0;
	bp->lc = fp->lc - l0;

	for (i = 0; i < c; i++)
		if (strcmp(k[i], "target") == 0)
			make_ref(v[i], &bp->pi);
}

static void make_coin(struct s_file *fp, char k[][MAXSTR],
                                         char v[][MAXSTR], int c)
{
	int i, ci = fp->cc++;

	struct s_coin *cp = fp->cv + ci;

	cp->p[0] = 0.0;
	cp->p[1] = 0.0;
	cp->p[2] = 0.0;
	cp->n    = 1;

	for (i = 0; i < c; i++)
	{
		if (strcmp(k[i], "light") == 0)
			sscanf(v[i], "%d", &cp->n);

		if (strcmp(k[i], "origin") == 0)
		{
			int x = 0, y = 0, z = 0;

			sscanf(v[i], "%d %d %d", &x, &y, &z);

			cp->p[0] = +(double) x / SCALE;
			cp->p[1] = +(double) z / SCALE;
			cp->p[2] = -(double) y / SCALE;
		}
	}
}

static void make_ball(struct s_file *fp, char k[][MAXSTR],
                                         char v[][MAXSTR], int c)
{
	int i, ui = fp->uc++;

	struct s_ball *up = fp->uv + ui;

	up->p[0] = 0.0;
	up->p[1] = 0.0;
	up->p[2] = 0.0;
	up->r    = 1.0;

	up->e[0][0] = 1.0;
	up->e[0][1] = 0.0;
	up->e[0][2] = 0.0;
	up->e[1][0] = 0.0;
	up->e[1][1] = 1.0;
	up->e[1][2] = 0.0;
	up->e[2][0] = 0.0;
	up->e[2][1] = 0.0;
	up->e[2][2] = 1.0;

	up->v[0] = 0.0;
	up->v[1] = 0.0;
	up->v[2] = 0.0;
	up->w[0] = 0.0;
	up->w[1] = 0.0;
	up->w[2] = 0.0;

	for (i = 0; i < c; i++)
	{
		if (strcmp(k[i], "radius") == 0)
			sscanf(v[i], "%lf", &up->r);

		if (strcmp(k[i], "origin") == 0)
		{
			int x = 0, y = 0, z = 0;

			sscanf(v[i], "%d %d %d", &x, &y, &z);

			up->p[0] = +(double) (x)      / SCALE;
			up->p[1] = +(double) (z - 24) / SCALE;
			up->p[2] = -(double) (y)      / SCALE;
		}
	}

	up->p[1] += up->r + 0.00001;
}

/*--------------------------------------------------------------------*/

static void read_ent(struct s_file *fp, FILE *fin)
{
	double n[3], d;
	char k[MAXKEY][MAXSTR];
	char v[MAXKEY][MAXSTR];
	int t, i = 0, c = 0;

	int l0 = fp->lc;

	while ((t = map_token(fin, n, &d, k[c], v[c])))
	{
		if (t == T_KEY)
		{
			if (strcmp(k[c], "classname") == 0)
				i = c;
			c++;
		}
		if (t == T_BEG)
			read_lump(fp, fin);
		if (t == T_END)
			break;
	}

	if (!strcmp(v[i], "light"))             make_coin(fp, k, v, c);
	if (!strcmp(v[i], "path_corner"))       make_path(fp, k, v, c);
	if (!strcmp(v[i], "info_player_start")) make_ball(fp, k, v, c);

	if (!strcmp(v[i], "worldspawn"))  make_body(fp, k, v, c, l0);
	if (!strcmp(v[i], "func_train"))  make_body(fp, k, v, c, l0);
}

static void read_map(struct s_file *fp, FILE *fin)
{
	double n[3], d;
	char k[MAXSTR];
	char v[MAXSTR];
	int t;

	while ((t = map_token(fin, n, &d, k, v)))
		if (t == T_BEG)
			read_ent(fp, fin);
}

/*--------------------------------------------------------------------*/
/* All bodies with an associated path are assumed to be positioned    */
/* at the beginning of that path.  These bodies must be moved to the  */
/* origin in order for their path transforms to behave correctly.     */
/* This is how we get away with defining func_trains with no origin   */
/* specification.                                                     */

static void move_side(struct s_side *sp, const double p[3])
{
	sp->d -= v_dot(sp->n, p);
}

static void move_lump(struct s_file *fp,
                      struct s_lump *lp, const double p[3])
{
	int i;

	for (i = 0; i < lp->sc; i++)
		move_side(fp->sv + fp->iv[lp->s0 + i], p);
}

static void move_body(struct s_file *fp,
                      struct s_body *bp)
{
	int i;

	for (i = 0; i < bp->lc; i++)
		move_lump(fp, fp->lv + bp->l0 + i, fp->pv[bp->pi].p);
}

static void move_sol(struct s_file *fp)
{
	int i;

	for (i = 0; i < fp->bc; i++)
		if (fp->bv[i].pi >= 0)
			move_body(fp, fp->bv + i);
}

/*--------------------------------------------------------------------*/
/* Test the location of a point with respect to a side plane.         */

static int fore_side(const double p[3], const struct s_side *sp)
{
	return (v_dot(p, sp->n) - sp->d > +SMALL) ? 1 : 0;
}

static int back_side(const double p[3], const struct s_side *sp)
{
	return (v_dot(p, sp->n) - sp->d < -SMALL) ? 1 : 0;
}

static int on_side(const double p[3], const struct s_side *sp)
{
	double d = v_dot(p, sp->n) - sp->d;

	return (-SMALL < d && d < +SMALL) ? 1 : 0;
}

/*--------------------------------------------------------------------*/
/* Confirm that the addition of a vert would not result in degenerate */
/* geometry.                                                          */

static int ok_vert(const struct s_file *fp,
                   const struct s_lump *lp, const double p[3])
{
	double r[3];
	int i;

	for (i = 0; i < lp->vc; i++)
	{
		v_sub(r, p, fp->vv[lp->v0 + i].p);

		if (v_len(r) < SMALL)
			return 0;
	}
	return 1;
}

/*--------------------------------------------------------------------*/

/* The following functions take the set of planes defining a lump and */
/* compute the verts, edges, and quads that describe its boundaries.  */
/* To do this, they first find the verts, and then search these verts */
/* for valid edges and quads.  It would be more efficient to compute  */
/* edges and quads directly by clipping down infinite line segments   */
/* and planes, but this would be more complex and prone to numerical  */
/* error.                                                             */


/* Given 3 side planes, compute the point of intersection, if any.    */
/* Confirm that this point falls within the current lump, and that    */
/* it is unique.  Add it as a vert of the solid.                      */

static void clip_vert(struct s_file *fp,
                      struct s_lump *lp, int si, int sj, int sk)
{
	double M[16], X[16], I[16];
	double d[3],  p[3];
	int i;

	d[0] = fp->sv[si].d;
	d[1] = fp->sv[sj].d;
	d[2] = fp->sv[sk].d;

	m_basis(M, fp->sv[si].n,
	           fp->sv[sj].n,
	           fp->sv[sk].n);
	m_xps(X, M);
	
	if (m_inv(I, X))
	{
		m_vxfm(p, I, d);

		for (i = 0; i < lp->sc; i++)
		{
			int si = fp->iv[lp->s0 + i];

			if (fore_side(p, fp->sv + si))
				return;
		}

		if (ok_vert(fp, lp, p))
		{
			v_cpy(fp->vv[fp->vc].p, p);

			fp->iv[fp->ic] = fp->vc;
			fp->ic++;
			fp->vc++;
			lp->vc++;
		}
	}
}

/* Given two side planes, find a lump edge along their intersection   */
/* by finding a pair of vertices that fall on both planes.  Confirm   */
/* that this edge is not degenerate and add it to the solid.          */

static void clip_edge(struct s_file *fp,
                      struct s_lump *lp, int si, int sj)
{
	int i, j;

	for (i = 1; i < lp->vc; i++)
		for (j = 0; j < i; j++)
		{
			int vi = fp->iv[lp->v0 + i];
			int vj = fp->iv[lp->v0 + j];

			if (on_side(fp->vv[vi].p, fp->sv + si) &&
			    on_side(fp->vv[vj].p, fp->sv + si) &&
			    on_side(fp->vv[vi].p, fp->sv + sj) &&
			    on_side(fp->vv[vj].p, fp->sv + sj))
			{
				fp->ev[fp->ec].vi = vi;
				fp->ev[fp->ec].vj = vj;

				fp->iv[fp->ic] = fp->ec;

				fp->ic++;
				fp->ec++;
				lp->ec++;
			}
		}
}

/* Find all verts that lie on the given side of the lump.  Sort these */
/* verts to have a counter-clockwise winding about the plane normal.  */
/* Generate quads to tessalate the resulting convex polygon.          */

static void clip_quad(struct s_file *fp,
                      struct s_lump *lp, int si)
{
	int    m[16], i, j, n = 0;
	double u[3];
	double v[3];
	double w[3];

	struct s_side *sp = fp->sv + si;

	for (i = 0; i < lp->vc; i++)
	{
		int vi = fp->iv[lp->v0 + i];

		if (on_side(fp->vv[vi].p, sp))
			m[n++] = vi;
	}

	for (i = 1; i < n; i++)
		for (j = i + 1; j < n; j++)
		{
			v_sub(u, fp->vv[m[i]].p, fp->vv[m[0]].p);
			v_sub(v, fp->vv[m[j]].p, fp->vv[m[0]].p);
			v_crs(w, u, v);

			if (v_dot(w, sp->n) < 0.0)
			{
				int t = m[i];
				m[i]  = m[j];
				m[j]  =    t;
			}
		}

	/* FIXME: does not tessellate oddagons. */

	for (i = 0; i < n - 3; i += 2)
	{
		fp->qv[fp->qc].si = si;
		fp->qv[fp->qc].mi = si;
		fp->qv[fp->qc].vi = m[0];
		fp->qv[fp->qc].vj = m[i + 1];
		fp->qv[fp->qc].vk = m[i + 2];
		fp->qv[fp->qc].vl = m[i + 3];

		fp->iv[fp->ic] = fp->qc;
		fp->ic++;
		fp->qc++;
		lp->qc++;
	}
}

/* Iterate the sides of the lump, attemping to generate a new vert    */
/* for each trio of planes, a new edge for each pair of planes, and   */
/* a new set of quads for each visible plane.                         */

static void clip_lump(struct s_file *fp, struct s_lump *lp)
{
	int i, j, k;

	lp->v0 = fp->ic;
	lp->vc = 0;

	for (i = 2; i < lp->sc; i++)
		for (j = 1; j < i; j++)
			for (k = 0; k < j; k++)
				clip_vert(fp, lp, fp->iv[lp->s0 + i],
				                  fp->iv[lp->s0 + j],
				                  fp->iv[lp->s0 + k]);

	lp->e0 = fp->ic;
	lp->ec = 0;

	for (i = 1; i < lp->sc; i++)
		for (j = 0; j < i; j++)
			clip_edge(fp, lp, fp->iv[lp->s0 + i],
			                  fp->iv[lp->s0 + j]);

	lp->q0 = fp->ic;
	lp->qc = 0;

	for (i = 0; i < lp->sc; i++)
		if (fp->mv[fp->iv[lp->s0 + i]].d[3] > 0)
			clip_quad(fp, lp, fp->iv[lp->s0 + i]);
}

static void clip_sol(struct s_file *fp)
{
	int i;

	for (i = 0; i < fp->lc; i++)
		clip_lump(fp, fp->lv + i);
}

/*--------------------------------------------------------------------*/
#ifdef SNIP

static int comp_mtrl(const struct mtrl *M1, const struct mtrl *M2)
{
	if (fabs(M1->d[0] - M2->d[0]) > SMALL) return 0;
	if (fabs(M1->d[1] - M2->d[1]) > SMALL) return 0;
	if (fabs(M1->d[2] - M2->d[2]) > SMALL) return 0;
	if (fabs(M1->d[3] - M2->d[3]) > SMALL) return 0;

	if (fabs(M1->s[0] - M2->s[0]) > SMALL) return 0;
	if (fabs(M1->s[1] - M2->s[1]) > SMALL) return 0;
	if (fabs(M1->s[2] - M2->s[2]) > SMALL) return 0;
	if (fabs(M1->s[3] - M2->s[3]) > SMALL) return 0;

	if (fabs(M1->e[0] - M2->e[0]) > SMALL) return 0;
	if (fabs(M1->e[1] - M2->e[1]) > SMALL) return 0;
	if (fabs(M1->e[2] - M2->e[2]) > SMALL) return 0;
	if (fabs(M1->e[3] - M2->e[3]) > SMALL) return 0;

	if (fabs(M1->h[0] - M2->h[0]) > SMALL) return 0;

	return 1;
}

static int comp_vert(const struct vert *V1, const struct vert *V2)
{
	if (fabs(V1->p[0] - V2->p[0]) > SMALL) return 0;
	if (fabs(V1->p[1] - V2->p[1]) > SMALL) return 0;
	if (fabs(V1->p[2] - V2->p[2]) > SMALL) return 0;

	return 1;
}

static int comp_edge(const struct edge *E1, const struct edge *E2)
{
	if (E1->vi != E2->vi && E1->vi != E2->vj) return 0;
	if (E1->vj != E2->vi && E1->vj != E2->vj) return 0;

	return 1;
}

static int comp_side(const struct s_side *S1, const struct s_side *S2)
{
	if  (S1->mi != S2->mi) return 0;

	if  (fabs(S1->d - S2->d) >       SMALL) return 0;
	if (v_dot(S1->n,  S2->n) < 1.0 - SMALL) return 0;

	return 1;
}

static int comp_quad(const struct quad *F1, const struct quad *F2)
{
	if (F1->si != F2->si)
		return 0;
	if (F1->vi != F2->vi && F1->vi != F2->vj && F1->vi != F2->vk)
		return 0;
	if (F1->vj != F2->vi && F1->vj != F2->vj && F1->vj != F2->vk)
		return 0;
	if (F1->vk != F2->vi && F1->vk != F2->vj && F1->vk != F2->vk)
		return 0;

	return 1;
}

/*--------------------------------------------------------------------*/

static void swap_mtrl(struct s_body *bp, int ma, int mb)
{
	int i;

	for (i = 0; i < bp->sn; i++)
		if (bp->sv[i].mi == ma) bp->sv[i].mi = mb;
}

static void swap_vert(struct s_body *bp, int va, int vb)
{
	int i, j;

	for (i = 0; i < bp->en; i++)
	{
		if (bp->ev[i].vi == va) bp->ev[i].vi = vb;
		if (bp->ev[i].vj == va) bp->ev[i].vj = vb;
	}
	for (i = 0; i < bp->fn; i++)
	{
		if (bp->fv[i].vi == va) bp->fv[i].vi = vb;
		if (bp->fv[i].vj == va) bp->fv[i].vj = vb;
		if (bp->fv[i].vk == va) bp->fv[i].vk = vb;
	}
	for (i = 0; i < bp->ln; i++)
		for (j = 0; j < bp->lv[i].vn; j++)
			if (bp->iv[bp->lv[i].v0 + j] == va)
			    bp->iv[bp->lv[i].v0 + j]  = vb;
}

static void swap_edge(struct s_body *bp, int ea, int eb)
{
	int i, j;

	for (i = 0; i < bp->ln; i++)
		for (j = 0; j < bp->lv[i].en; j++)
			if (bp->iv[bp->lv[i].e0 + j] == ea)
			    bp->iv[bp->lv[i].e0 + j]  = eb;
}

static void swap_side(struct s_body *bp, int sa, int sb)
{
	int i, j;

	for (i = 0; i < bp->fn; i++)
		if (bp->fv[i].si == sa) bp->fv[i].si = sb;
	for (i = 0; i < bp->nn; i++)
		if (bp->nv[i].si == sa) bp->nv[i].si = sb;

	for (i = 0; i < bp->ln; i++)
		for (j = 0; j < bp->lv[i].sn; j++)
			if (bp->iv[bp->lv[i].s0 + j] == sa)
			    bp->iv[bp->lv[i].s0 + j]  = sb;
}

static void swap_quad(struct s_body *bp, int fa, int fb)
{
	int i, j;

	for (i = 0; i < bp->ln; i++)
		for (j = 0; j < bp->lv[i].fn; j++)
			if (bp->iv[bp->lv[i].f0 + j] == fa)
			    bp->iv[bp->lv[i].f0 + j]  = fb;
}

/*--------------------------------------------------------------------*/

static void uniq_mtrl(struct s_body *bp)
{
	int i, j, k = 0;

	for (i = 0; i < bp->mn; i++)
	{
		for (j = 0; j < i; j++)
			if (comp_mtrl(bp->mv + i, bp->mv + j))
			{
				swap_mtrl(bp, i, j);
				break;
			}

		if (i == j)
		{
			bp->mv[k] = bp->mv[i];
			swap_mtrl(bp, i, k);
			k++;
		}
	}

	bp->mn = k;
}

static void uniq_vert(struct s_body *bp)
{
	int i, j, k = 0;

	for (i = 0; i < bp->vn; i++)
	{
		for (j = 0; j < i; j++)
			if (comp_vert(bp->vv + i, bp->vv + j))
			{
				swap_vert(bp, i, j);
				break;
			}

		if (i == j)
		{
			bp->vv[k] = bp->vv[i];
			swap_vert(bp, i, k);
			k++;
		}
	}

	bp->vn = k;
}

static void uniq_edge(struct s_body *bp)
{
	int i, j, k = 0;

	for (i = 0; i < bp->en; i++)
	{
		for (j = 0; j < i; j++)
			if (comp_edge(bp->ev + i, bp->ev + j))
			{
				swap_edge(bp, i, j);
				break;
			}

		if (i == j)
		{
			bp->ev[k] = bp->ev[i];
			swap_edge(bp, i, k);
			k++;
		}
	}

	bp->en = k;
}

static void uniq_quad(struct s_body *bp)
{
	int i, j, k = 0;

	for (i = 0; i < bp->fn; i++)
	{
		for (j = 0; j < i; j++)
			if (comp_quad(bp->fv + i, bp->fv + j))
			{
				swap_quad(bp, i, j);
				break;
			}

		if (i == j)
		{
			bp->fv[k] = bp->fv[i];
			swap_quad(bp, i, k);
			k++;
		}
	}

	bp->fn = k;
}

static void uniq_side(struct s_body *bp)
{
	int i, j, k = 0;

	for (i = 0; i < bp->sn; i++)
	{
		for (j = 0; j < i; j++)
			if (comp_side(bp->sv + i, bp->sv + j))
			{
				swap_side(bp, i, j);
				break;
			}
		if (i == j)
		{
			bp->sv[k] = bp->sv[i];
			swap_side(bp, i, k);
			k++;
		}
	}

	bp->sn = k;
}

static void uniq_body(struct s_body *bp)
{
	uniq_mtrl(bp);
	uniq_vert(bp);
	uniq_edge(bp);
	uniq_side(bp);
	uniq_quad(bp);
}

/*--------------------------------------------------------------------*/

static void sort_body(struct s_body *bp)
{
	int i, j;

	struct quad T;

	for (i = 0; i < bp->fn; i++)
		for (j = i + 1; j < bp->fn; j++)
			if (bp->sv[bp->fv[i].si].mi >
			    bp->sv[bp->fv[j].si].mi)
			{
				T        = bp->fv[i];
				bp->fv[i] = bp->fv[j];
				bp->fv[j] =        T;

				swap_quad(bp,  i, -1);
				swap_quad(bp,  j,  i);
				swap_quad(bp, -1,  j);
			}
}

/*--------------------------------------------------------------------*/

static int slump(const struct s_body *bp,
                     const struct s_side *S,
                     const struct s_lump *L)
{
	int b = 0;
	int f = 0;
	int i;

	for (i = 0; i < L->vn; i++)
	{
		int vi = bp->iv[L->v0 + i];

		if (fore_side(bp->vv[vi].p, S)) f++;
		if (back_side(bp->vv[vi].p, S)) b++;
	}

	if (f == 0) return -1;
	if (b == 0) return +1;

	return 0;
}

static int tree_side(struct s_body *bp, int ni)
{
	int i, mi = 0, j;
	int f, mf = 0;
	int b, mb = bp->vn;
	int o, mo = 0;

	for (i = 0; i < bp->sn; i++)
	{
		b = 0;
		f = 0;

		for (j = 0; j < bp->nv[ni].ln; j++)
		{
			int lj = bp->iv[bp->nv[ni].l0 + j];
			int s  = slump(bp, bp->sv + i, bp->lv + lj);

			if (s <  0) b++;
			if (s == 0) o++;
			if (s  > 0) f++;
		}

		if ((abs(f - b) <  abs(mf - mb)) ||
		    (abs(f - b) == abs(mf - mb) && o < mo))
		{
			mf = f;
			mb = b;
			mo = o;
			mi = i;
		}
	}

	return mi;
}

static void tree_node(struct s_body *bp, int ni)
{
	if (bp->nv[ni].ln > 1)
	{
		int i, t;

		int o = 0;
		int b = 0;
		int f = 0;

		int si = tree_side(bp, ni);

		for (i = 0; i < bp->nv[ni].ln; i++)
		{
			int i0 = bp->nv[ni].l0;
			int li = bp->iv[i0 + i];

			if (slump(bp, bp->sv + si, bp->lv + li) == 0)
			{
				t             = bp->iv[i0 + i];
				bp->iv[i0 + i] = bp->iv[i0 + o];
				bp->iv[i0 + o] =             t;
				o++;
			}
		}

		b = o;

		for (i = o; i < bp->nv[ni].ln; i++)
		{
			int i0 = bp->nv[ni].l0;
			int li = bp->iv[i0 + i];

			if (slump(bp, bp->sv + si, bp->lv + li) < 0)
			{
				t             = bp->iv[i0 + i];
				bp->iv[i0 + i] = bp->iv[i0 + b];
				bp->iv[i0 + b] =             t;
				b++;
			}
		}

		f = b;

		for (i = b; i < bp->nv[ni].ln; i++)
		{
			int i0 = bp->nv[ni].l0;
			int li = bp->iv[i0 + i];

			if (slump(bp, bp->sv + si, bp->lv + li) > 0)
			{
				t             = bp->iv[i0 + i];
				bp->iv[i0 + i] = bp->iv[i0 + f];
				bp->iv[i0 + f] =             t;
				f++;
			}
		}

		if (b - o > 0 && f - b > 0)
		{
			int nb = bp->nn++;
			int nf = bp->nn++;

			bp->nv[ni].si = si;
			bp->nv[ni].ni = nb;
			bp->nv[ni].nj = nf;
			bp->nv[ni].ln = o;

			bp->nv[nb].ni = 0;
			bp->nv[nb].nj = 0;
			bp->nv[nb].l0 = bp->nv[ni].l0 + o;
			bp->nv[nb].ln = b - o;

			bp->nv[nf].ni = 0;
			bp->nv[nf].nj = 0;
			bp->nv[nf].l0 = bp->nv[ni].l0 + b;
			bp->nv[nf].ln = f - b;

			tree_node(bp, nb);
			tree_node(bp, nf);
		}
	}
}

static void tree_body(struct s_body *bp)
{
	int i;

	bp->nv[0].ni = 0;
	bp->nv[0].nj = 0;
	bp->nv[0].l0 = bp->in;
	bp->nv[0].ln = bp->ln;

	for (i = 0; i < bp->ln; i++)
		bp->iv[bp->in++] = i;

	bp->nn++;

	tree_node(bp, 0);
}
#endif
/*--------------------------------------------------------------------*/

static void dump_sol(struct s_file *fp)
{
	printf("  mtrl  vert  edge  side  quad  lump"
               "  node  path  body  coin  ball  indx\n");
	printf("%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d\n",
	       fp->mc, fp->vc, fp->ec, fp->sc, fp->qc, fp->lc,
	       fp->nc, fp->pc, fp->bc, fp->cc, fp->uc, fp->ic);
}

/*--------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
	struct s_file f;
	FILE *ip;
	FILE *op;

	if (argc > 2)
	{
		if ((ip = fopen(argv[1], "r")) &&
		    (op = fopen(argv[2], "w")))
		{
			init_sol(&f);
			read_map(&f, ip);

			resolve();

			move_sol(&f);
			clip_sol(&f);
			dump_sol(&f);
/*
			uniq_body(&bp);
			sort_body(&bp);
			tree_body(&bp);
			dump_body(&bp);
*/

			sol_stor(&f, op);

			fclose(op);
			fclose(ip);
		}
	}
	return 0;
}

