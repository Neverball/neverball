#ifndef SOL_H
#define SOL_H

#include <stdio.h>

/*--------------------------------------------------------------------*/

struct s_mtrl
{
	float d[4];			/* diffuse color              */
	float s[4];			/* specular color             */
	float e[4];			/* emission color             */
	float h[1];			/* specular exponent          */
};

struct s_vert
{
	double p[3];			/* vertex position            */
};

struct s_edge
{
	int vi;
	int vj;
};

struct s_side
{
	double n[3];			/* plane normal vector        */
	double d;			/* distance from origin       */
};

struct s_quad
{
	int si;
	int mi;
	int vi;
	int vj;
	int vk;
	int vl;
};

struct s_lump
{
	int v0;
	int vc;
	int e0;
	int ec;
	int q0;
	int qc;
	int s0;
	int sc;
};

struct s_node
{
	int si;
	int ni;
	int nj;
	int l0;
	int lc;
};

struct s_path
{
	double p[3];			/* starting position          */
	double t;			/* travel time                */

	int pi;
};

struct s_body
{
	double t;			/* time on current path       */

	int pi;
	int ni;
	int l0;
	int lc;
};

struct s_coin
{
	double p[3];			/* position                   */
	int    n;			/* value                      */
};

struct s_ball
{
	double e[3][3];			/* basis of orientation       */
	double p[3];			/* position vector            */
	double v[3];			/* velocity vector            */
	double w[3];			/* angular velocity vector    */
	double r;			/* radius                     */
};

struct s_file
{
	int mc;
	int vc;
	int ec;
	int sc;
	int qc;
	int lc;
	int nc;
	int pc;
	int bc;
	int cc;
	int uc;
	int ic;

	struct s_mtrl *mv;
	struct s_vert *vv;
	struct s_edge *ev;
	struct s_side *sv;
	struct s_quad *qv;
	struct s_lump *lv;
	struct s_node *nv;
	struct s_path *pv;
	struct s_body *bv;
	struct s_coin *cv;
	struct s_ball *uv;
	int           *iv;
};

/*--------------------------------------------------------------------*/

int    sol_load(struct s_file *, FILE *);
int    sol_stor(struct s_file *, FILE *);
void   sol_free(struct s_file *);

void   sol_render(const struct s_file *);
void   sol_update(struct s_file *, double, const double[3]);

#endif
