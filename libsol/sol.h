#ifndef SOL_H
#define SOL_H

/*--------------------------------------------------------------------*/

struct vert
{
	double p[3];
};

struct edge
{
	double p[3];
	double v[3];
};

struct face
{
	double n[3];
	int vi; 
	int vj; 
	int vk; 
	unsigned char c[4];
};

struct side
{
	double n[3];
	double d;
};

struct lump
{
	int vc;
	int ec;
	int fc;
	int sc;
	struct vert *vv;
	struct edge *ev;
	struct face *fv;
	struct side *sv;
};

struct body
{
	int lc;
	struct lump *lv;
};

struct ball
{
	double e[3][3];			/* basis of orientation       */
	double p[3];			/* position vector            */
	double v[3];			/* velocity vector            */
	double w[3];			/* angular velocity vector    */
	double r;			/* radius                     */
};

/*--------------------------------------------------------------------*/

int    sol_load(struct body *);
void   sol_free(struct body *);
void   sol_draw(struct body *);
void   sol_step(struct ball *, const struct ball *, double);
double sol_ball(struct ball *, const struct ball *,
                               const struct body *);

#endif
