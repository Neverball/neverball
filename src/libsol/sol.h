#ifndef SOL_H
#define SOL_H

#include <GL/gl.h>

#define PATHMAX 44

/*---------------------------------------------------------------------------*/

struct s_imag
{
    int   o;
    int   b;
    int   w;
    int   h;
    void *p;
};

struct s_list
{
    GLuint o;                                  /* opaque geometry list       */
    GLuint f;                                  /* floor geometry list        */
    GLuint t;                                  /* transparent geometry list  */
};

struct s_mtrl
{
    float a[4];                                /* ambient color              */
    float d[4];                                /* diffuse color              */
    float s[4];                                /* specular color             */
    float e[4];                                /* emission color             */
    float h[1];                                /* specular exponent          */
    char  f[PATHMAX];                          /* texture file name          */
};

struct s_vert
{
    double p[3];                               /* vertex position            */
};

struct s_edge
{
    int vi;
    int vj;
};

struct s_side
{
    double n[3];                               /* plane normal vector        */
    double d;                                  /* distance from origin       */
};

struct s_texc
{
    double u[2];                               /* texture coordinate         */
};

struct s_geom
{
    int si;
    int mi;
    int ti, vi;
    int tj, vj;
    int tk, vk;
};

struct s_lump
{
    int fl;
    int v0;
    int vc;
    int e0;
    int ec;
    int g0;
    int gc;
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
    double p[3];                               /* starting position          */
    double t;                                  /* travel time                */

    int pi;
};

struct s_body
{
    double t;                                  /* time on current path       */

    int pi;
    int ni;
    int l0;
    int lc;
};

struct s_coin
{
    double p[3];                               /* position                   */
    int    n;                                  /* value                      */
};

struct s_ball
{
    double e[3][3];                            /* basis of orientation       */
    double p[3];                               /* position vector            */
    double v[3];                               /* velocity vector            */
    double w[3];                               /* angular velocity vector    */
    double r;                                  /* radius                     */
};

struct s_file
{
    int mc;
    int vc;
    int ec;
    int sc;
    int tc;
    int gc;
    int lc;
    int nc;
    int pc;
    int bc;
    int cc;
    int uc;
    int ic;

    struct s_imag *xv;
    struct s_list *dv;

    struct s_mtrl *mv;
    struct s_vert *vv;
    struct s_edge *ev;
    struct s_side *sv;
    struct s_texc *tv;
    struct s_geom *gv;
    struct s_lump *lv;
    struct s_node *nv;
    struct s_path *pv;
    struct s_body *bv;
    struct s_coin *cv;
    struct s_ball *uv;
    int           *iv;
};

/*---------------------------------------------------------------------------*/

void sol_render(const struct s_file *, int);

int  sol_load(struct s_file *, const char *);
int  sol_stor(struct s_file *, const char *);
void sol_free(struct s_file *);

int  sol_inside(const struct s_file *);
void sol_update(struct s_file *, double, const double[3], double *, double *);

#endif
