#include <ode/ode.h>
#include <stdio.h>

#include "solid.h"
#include "solid_sim.h"
#include "solid_all.h"
#include "solid_cmd.h"

#include "array.h"
#include "vec3.h"
#include "cmd.h"

/*---------------------------------------------------------------------------*/

static dWorldID      world;
static dSpaceID      space;
static dJointGroupID group;

static Array lumps;

static dBodyID *bodies;
static dBodyID *balls;

/*---------------------------------------------------------------------------*/

/*
 * Polygons in the same order as planes, one polygon per plane, each
 * starting with the number of vertices, followed by the vertex
 * indices in counter-clockwise order.
 */

struct convex
{
    dReal    *planes;
    unsigned  nplanes;
    dReal    *points;
    unsigned  npoints;
    unsigned *polygons;
};

static void make_convex(struct convex *cx, struct s_file *fp, struct s_lump *lp)
{
    int i;

    /*
     * A debug build of ODE (0.11.1 as I write this) will print a
     * bunch of warnings about our planes not containing the origin
     * and vertices having the wrong winding.  The latter is triggered
     * by presuming that the former shall be fulfilled, but the former
     * basically doesn't matter; collision detection handles
     * origin-facing planes just fine, provided the vertices on the
     * plane are still counter-clockwise wrt the plane normal.
     */

    memset(cx, 0, sizeof (*cx));

    if ((cx->planes = malloc(sizeof (dReal) * 4 * lp->sc)))
    {
        for (i = 0; i < lp->sc; i++)
        {
            struct s_side *sp = fp->sv + fp->iv[lp->s0 + i];

            cx->planes[i * 4 + 0] = sp->n[0];
            cx->planes[i * 4 + 1] = sp->n[1];
            cx->planes[i * 4 + 2] = sp->n[2];
            cx->planes[i * 4 + 3] = sp->d;
        }

        cx->nplanes = lp->sc;
    }

    if ((cx->points = malloc(sizeof (dReal) * 3 * lp->vc)))
    {
        for (i = 0; i < lp->vc; i++)
        {
            struct s_vert *vp = fp->vv + fp->iv[lp->v0 + i];

            cx->points[i * 3 + 0] = vp->p[0];
            cx->points[i * 3 + 1] = vp->p[1];
            cx->points[i * 3 + 2] = vp->p[2];
        }

        cx->npoints = lp->vc;
    }

    if ((cx->polygons = malloc(sizeof (unsigned) * lp->fc)))
        for (i = 0; i < lp->fc; i++)
            cx->polygons[i] = fp->iv[lp->f0 + i];
}

void free_convex(struct convex *cx)
{
    free(cx->planes);
    free(cx->points);
    free(cx->polygons);
}

/*---------------------------------------------------------------------------*/

#define CAT_OBJECT 0x1
#define CAT_WORLD  0x2

static void build_node(dBodyID body, dSpaceID parent,
                       struct s_node *np,
                       struct s_file *fp)
{
    dSpaceID node = dHashSpaceCreate(parent);
    dGeomID geom;
    int i;

    for (i = 0; i < np->lc; i++)
    {
        struct s_lump *lp = fp->lv + np->l0 + i;
        struct convex *cx;

        if (lp->fl & L_DETAIL)
            continue;

        cx = array_add(lumps);

        make_convex(cx, fp, lp);

        geom = dCreateConvex(node,
                             cx->planes, cx->nplanes,
                             cx->points, cx->npoints,
                             cx->polygons);

        dGeomSetCategoryBits(geom, CAT_WORLD);
        dGeomSetCollideBits (geom, CAT_OBJECT);

        if (body) dGeomSetBody(geom, body);
    }

    if (np->ni >= 0) build_node(body, node, fp->nv + np->ni, fp);
    if (np->nj >= 0) build_node(body, node, fp->nv + np->nj, fp);
}

static void build_branch(dBodyID body, dSpaceID root,
                         struct s_body *bp,
                         struct s_file *fp)
{
    dSpaceID branch = dHashSpaceCreate(root);

    build_node(body, branch, fp->nv + bp->ni, fp);
}

void sol_init_sim(struct s_file *fp)
{
    dGeomID geom;

    int i;

    dInitODE();

    world  = dWorldCreate();
    space  = dHashSpaceCreate(0);
    group  = dJointGroupCreate(0);

    lumps  = array_new(sizeof (struct convex));

    bodies = calloc(fp->bc, sizeof (dBodyID));
    balls  = calloc(fp->uc, sizeof (dBodyID));

    for (i = 0; i < fp->bc; i++)
    {
        struct s_body *bp = fp->bv + i;

        if (bp->pi >= 0)
        {
            bodies[i] = dBodyCreate(world);
            dBodySetKinematic(bodies[i]);
        }
        else
            bodies[i] = 0;

        build_branch(bodies[i], space, bp, fp);

        if (bodies[i])
        {
            float p[3], w[3];

            sol_body_p(p, fp, bp->pi, 0.0f);
            sol_body_w(w, fp, bp);

            dBodySetPosition(bodies[i], p[0], p[1], p[2]);
            dBodySetAngularVel(bodies[i], w[0], w[1], w[2]);
        }
    }

    for (i = 0; i < fp->uc; i++)
    {
        balls[i] = dBodyCreate(world);

        geom = dCreateSphere(space, fp->uv[i].r);

        dGeomSetCategoryBits(geom, CAT_OBJECT);
        dGeomSetCollideBits(geom, CAT_WORLD);
        dGeomSetData(geom, fp->uv + i);
        dGeomSetBody(geom, balls[i]);

        dBodySetPosition(balls[i],
                         fp->uv[i].p[0],
                         fp->uv[i].p[1],
                         fp->uv[i].p[2]);
    }
}

void sol_quit_sim(void)
{
    free(balls);
    free(bodies);

    while (array_len(lumps))
    {
        struct convex *cx = array_get(lumps, array_len(lumps) - 1);
        free_convex(cx);
        array_del(lumps);
    }

    array_free(lumps);

    dJointGroupDestroy(group);
    dSpaceDestroy(space);
    dWorldDestroy(world);

    dCloseODE();
}

/*---------------------------------------------------------------------------*/

static const dVector3 v_null = { 0.0, 0.0, 0.0 };

static const dReal *geomvel(dGeomID g)
{
    dBodyID b;
    return (b = dGeomGetBody(g)) ? dBodyGetLinearVel(b) : v_null;
}

static const dReal *pointvel(dGeomID g, dVector3 p)
{
    dBodyID b;

    if ((b = dGeomGetBody(g)))
    {
        static dVector3 v;
        dBodyGetPointVel(b, p[0], p[1], p[2], v);
        return v;
    }
    else
        return v_null;
}

/*
 * Compute the "energy" of the impact, to determine the sound amplitude.
 */
static float bump(dContactGeom *contact)
{
    float r[3];
    v_sub(r, geomvel(contact->g1), pointvel(contact->g2, contact->pos));
    return fabsf(v_dot(contact->normal, r));
}

static void spin(dContactGeom *contact)
{
    struct s_ball *up;
    const dReal *p, *v, *w;
    float r[3], d[3];

    if ((up = dGeomGetData(contact->g1)))
    {

        p = dGeomGetPosition(contact->g1);
        v = geomvel(contact->g1);
        w = pointvel(contact->g2, contact->pos);
    }
    else
    {
        up = dGeomGetData(contact->g2);

        p = dGeomGetPosition(contact->g2);
        v = geomvel(contact->g2);
        w = pointvel(contact->g1, contact->pos);
    }

    v_sub(r, p, contact->pos);
    v_sub(d, v, w);

    /* Find the new angular velocity. */

    v_crs(up->w, d, r);
    v_scl(up->w, up->w, -1.0f / (up->r * up->r));
}

static void collide(void *data, dGeomID o1, dGeomID o2)
{
    dContact contacts[8];
    dJointID joint;

    int n, i;

    float *b = (float *) data;
    float  d;

    if (dGeomIsSpace(o1) || dGeomIsSpace(o2))
    {
        dSpaceCollide2(o1, o2, data, collide);
        return;
    }

    if ((n = dCollide(o1, o2, 8, &contacts[0].geom, sizeof (dContact))) > 0)
    {
        for (i = 0; i < n; i++)
        {
            if (*b < (d = bump(&contacts[i].geom)))
                *b = d;

            spin(&contacts[i].geom);

            contacts[i].surface.mode       = dContactBounce;
            contacts[i].surface.mu         = 0;
            contacts[i].surface.bounce     = 0.718;
            contacts[i].surface.bounce_vel = 0;

            joint = dJointCreateContact(world, group, contacts + i);

            dJointAttach(joint, dGeomGetBody(o1), dGeomGetBody(o2));
        }
    }
}

/*---------------------------------------------------------------------------*/

static void import_state(struct s_file *fp, float dt)
{
    int i;

    for (i = 0; i < fp->uc; i++)
    {
        dBodySetPosition(balls[i],
                         fp->uv[i].p[0],
                         fp->uv[i].p[1],
                         fp->uv[i].p[2]);

        dBodySetLinearVel(balls[i],
                          fp->uv[i].v[0],
                          fp->uv[i].v[1],
                          fp->uv[i].v[2]);

        dGeomSphereSetRadius(dBodyGetFirstGeom(balls[i]), fp->uv[i].r);
    }

    for (i = 0; i < fp->bc; i++)
    {
        struct s_body *bp = fp->bv + i;
        float p[3], v[3], w[3];

        if (bodies[i])
        {
            sol_body_p(p, fp, bp->pi, bp->t);
            sol_body_v(v, fp, bp->pi, bp->t, dt);
            sol_body_w(w, fp, bp);

            dBodySetPosition(bodies[i], p[0], p[1], p[2]);
            dBodySetLinearVel(bodies[i], v[0], v[1], v[2]);
            dBodySetAngularVel(bodies[i], w[0], w[1], w[2]);
        }
    }
}

static void export_state(struct s_file *fp)
{
    const dReal *v;
    int i;

    for (i = 0; i < fp->uc; i++)
    {
        struct s_ball *up = fp->uv + i;

        v = dBodyGetPosition(balls[i]);
        v_cpy(up->p, v);

        v = dBodyGetLinearVel(balls[i]);
        v_cpy(up->v, v);
    }

    for (i = 0; i < fp->bc; i++)
    {
        struct s_body *bp = fp->bv + i;

        if (bodies[i])
        {
            v = dBodyGetQuaternion(bodies[i]);
            q_cpy(bp->e, v);
        }
    }
}

/*---------------------------------------------------------------------------*/

float sol_step(struct s_file *fp, const float *g, float dt, int ui, int *m)
{
    union cmd cmd;
    float b = 0.0f;

    /* The simulation is advanced only once, commands don't accumulate. */

    sol_cmd_defer = 0;

    import_state(fp, dt);

    dSpaceCollide(space, &b, collide);
    dWorldSetGravity(world, g[0], g[1], g[2]);
    dWorldQuickStep(world, dt);
    dJointGroupEmpty(group);

    sol_body_step(fp, dt);
    sol_swch_step(fp, dt);
    sol_ball_step(fp, dt);

    export_state(fp);

    cmd.type       = CMD_STEP_SIMULATION;
    cmd.stepsim.dt = dt;
    sol_cmd_enq(&cmd);

    return b;
}

/*---------------------------------------------------------------------------*/
