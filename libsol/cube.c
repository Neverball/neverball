#include <stdlib.h>
#include <vec.h>
#include "sol.h"

static void make_vert(struct vert *V, double x, double y, double z)
{
	V->p[0] = x;
	V->p[1] = y;
	V->p[2] = z;
}

static void make_edge(struct edge *E, struct vert *V, int vi, int vj)
{
	E->p[0] = V[vi].p[0];
	E->p[1] = V[vi].p[1];
	E->p[2] = V[vi].p[2];

	E->v[0] = V[vj].p[0] - V[vi].p[0];
	E->v[1] = V[vj].p[1] - V[vi].p[1];
	E->v[2] = V[vj].p[2] - V[vi].p[2];
}

static void make_side(struct side *S, double d,
                                      double n0, double n1, double n2)
{
	S->d    = d;
	S->n[0] = n0;
	S->n[1] = n1;
	S->n[2] = n2;
}

static void make_face(struct face *F, struct side *S,
                      int vi, int vj, int vk, unsigned char c[4])
{
	F->n[0] = S->n[0];
	F->n[1] = S->n[1];
	F->n[2] = S->n[2];

	F->vi = vi;
	F->vj = vj;
	F->vk = vk;

	F->c[0] = c[0];
	F->c[1] = c[1];
	F->c[2] = c[2];
	F->c[3] = c[3];
}

static void make_lump(struct lump *L, double x0, double x1,
                                      double y0, double y1,
                                      double z0, double z1,
                                      unsigned char c[4])
{
	L->vc =  8;
	L->ec = 12;
	L->fc = 12;
	L->sc =  6;

	L->vv = (struct vert *) calloc(L->vc, sizeof (struct vert));
	L->ev = (struct edge *) calloc(L->ec, sizeof (struct edge));
	L->fv = (struct face *) calloc(L->fc, sizeof (struct face));
	L->sv = (struct side *) calloc(L->sc, sizeof (struct side));

	make_vert(L->vv + 0, x0, y0, z0);
	make_vert(L->vv + 1, x1, y0, z0);
	make_vert(L->vv + 2, x0, y1, z0);
	make_vert(L->vv + 3, x1, y1, z0);
	make_vert(L->vv + 4, x0, y0, z1);
	make_vert(L->vv + 5, x1, y0, z1);
	make_vert(L->vv + 6, x0, y1, z1);
	make_vert(L->vv + 7, x1, y1, z1);

	make_edge(L->ev +  0, L->vv, 0, 1);
	make_edge(L->ev +  1, L->vv, 2, 3);
	make_edge(L->ev +  2, L->vv, 4, 5);
	make_edge(L->ev +  3, L->vv, 6, 7);

	make_edge(L->ev +  4, L->vv, 0, 2);
	make_edge(L->ev +  5, L->vv, 1, 3);
	make_edge(L->ev +  6, L->vv, 4, 6);
	make_edge(L->ev +  7, L->vv, 5, 7);

	make_edge(L->ev +  8, L->vv, 0, 4);
	make_edge(L->ev +  9, L->vv, 1, 5);
	make_edge(L->ev + 10, L->vv, 2, 6);
	make_edge(L->ev + 11, L->vv, 3, 7);

	make_side(L->sv + 0, +x1, +1.0,  0.0,  0.0);
	make_side(L->sv + 1, -x0, -1.0,  0.0,  0.0);
	make_side(L->sv + 2, +y1,  0.0, +1.0,  0.0);
	make_side(L->sv + 3, -y0,  0.0, -1.0,  0.0);
	make_side(L->sv + 4, +z1,  0.0,  0.0, +1.0);
	make_side(L->sv + 5, -z0,  0.0,  0.0, -1.0);

	make_face(L->fv +  0, L->sv + 0, 1, 3, 7, c);
	make_face(L->fv +  1, L->sv + 0, 7, 5, 1, c);
	make_face(L->fv +  2, L->sv + 1, 0, 4, 6, c);
	make_face(L->fv +  3, L->sv + 1, 6, 2, 0, c);
	make_face(L->fv +  4, L->sv + 2, 2, 6, 7, c);
	make_face(L->fv +  5, L->sv + 2, 7, 3, 2, c);
	make_face(L->fv +  6, L->sv + 3, 0, 1, 5, c);
	make_face(L->fv +  7, L->sv + 3, 5, 4, 0, c);
	make_face(L->fv +  8, L->sv + 4, 4, 5, 7, c);
	make_face(L->fv +  9, L->sv + 4, 7, 6, 4, c);
	make_face(L->fv + 10, L->sv + 5, 0, 2, 3, c);
	make_face(L->fv + 11, L->sv + 5, 3, 1, 0, c);
}

void make_body(struct body *B)
{
	unsigned char g[4] = { 0x00, 0xff, 0x00, 0xff };
	unsigned char o[4] = { 0xff, 0x80, 0x00, 0xff };
	unsigned char y[4] = { 0xff, 0xff, 0x00, 0xff };

	B->lc = 5;
	B->lv = (struct lump *) calloc(B->lc, sizeof (struct lump));

	make_lump(B->lv + 0, -8.0, +8.0, -2.0, +0.0, -8.0, +8.0, g);
	make_lump(B->lv + 1, -8.0, -6.0, +0.0, +4.0, -8.0, +8.0, o);
	make_lump(B->lv + 2, +6.0, +8.0, +0.0, +4.0, -8.0, +8.0, o);
	make_lump(B->lv + 3, -6.0, +6.0, +0.0, +4.0, -8.0, -6.0, y);
	make_lump(B->lv + 4, -6.0, +6.0, +0.0, +4.0, +6.0, +8.0, y);
}

