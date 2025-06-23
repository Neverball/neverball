/*
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> /* offsetof */
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <assert.h>

#if ENABLE_RADIANT_CONSOLE
/*
 * Mapc is not an SDL app, we just want the SDL_net symbols.
 */
#define WITHOUT_SDL 1
#include <SDL_net.h>
#endif

#include "solid_base.h"

#include "vec3.h"
#include "base_image.h"
#include "base_config.h"
#include "fs.h"
#include "common.h"

#define MAXSTR 256
#define MAXKEY 16
#define SCALE  64.f
#define SMALL  0.0005f

/*
 * The overall design  of this map converter is  very stupid, but very
 * simple. It  begins by assuming  that every mtrl, vert,  edge, side,
 * and texc  in the map is  unique.  It then makes  an optimizing pass
 * that discards redundant information.  The result is optimal, though
 * the process is terribly inefficient.
 */

/*---------------------------------------------------------------------------*/

#if ENABLE_RADIANT_CONSOLE

/*
 * Message levels from radiant/console.h
 */
#define BCAST_STD 1
#define BCAST_WRN 2
#define BCAST_ERR 3

#define MAX_BCAST_MSG 512

static TCPsocket     bcast_socket;
static unsigned char bcast_msg[MAX_BCAST_MSG];
static size_t        bcast_msg_len;

static void bcast_quit(struct mapc_context *ctx);

static int bcast_error(struct mapc_context *ctx)
{
    fprintf(stderr, "%s\n", SDLNet_GetError());
    bcast_quit(ctx);
    return 0;
}

static void bcast_write_len(struct mapc_context *ctx, size_t len)
{
    unsigned char *p = &ctx->bcast_msg[ctx->bcast_msg_len];

    if (ctx->bcast_msg_len + 4u < sizeof (ctx->bcast_msg))
    {
        p[0] =  len        & 0xff;
        p[1] = (len >> 8)  & 0xff;
        p[2] = (len >> 16) & 0xff;
        p[3] = (len >> 24) & 0xff;

        ctx->bcast_msg_len += 4u;
    }
}

static void bcast_write_str(struct mapc_context *ctx, const char *str)
{
    if (str && *str)
    {
        unsigned char *p = &ctx->bcast_msg[ctx->bcast_msg_len];
        size_t         n = MIN(strlen(str), (sizeof (ctx->bcast_msg) -
                                             ctx->bcast_msg_len - 1));

        memcpy(p, str, n);
        p[n] = 0;

        ctx->bcast_msg_len += n + 1;
    }
}

static void bcast_send_str(struct mapc_context *ctx, const char *str)
{
    size_t len;

    /* Reserve 4 bytes. */

    ctx->bcast_msg_len = 4;

    /* Write the string. */

    bcast_write_str(ctx, str);

    /* Write its length in the reserved 4 bytes. */

    len = ctx->bcast_msg_len;
    ctx->bcast_msg_len = 0;
    bcast_write_len(ctx, len - 4);
    ctx->bcast_msg_len = len;

    /* Send data. */

    if (ctx->bcast_socket &&
        SDLNet_TCP_Send(ctx->bcast_socket,
                        ctx->bcast_msg,
                        ctx->bcast_msg_len) < ctx->bcast_msg_len)
        bcast_error(ctx);
}

static void bcast_send_msg(struct mapc_context *ctx, int lvl, const char *str)
{
    char buf[512];
    int maxstr;

    assert(lvl > 0 && lvl < 10);

    /*
     * These are not real time. Radiant will buffer them and randomly
     * flush the buffer whenever it feels like (usually upon
     * disconnection). It also stops processing after a level 3
     * message.
     */

    maxstr = sizeof (buf) - sizeof ("<message level=\"1\"></message>");
    sprintf(buf, "<message level=\"%1d\">%.*s</message>", lvl, maxstr, str);
    bcast_send_str(ctx, buf);
}

static int bcast_init(struct mapc_context *ctx)
{
    IPaddress addr;

    if (SDLNet_Init() == -1)
        return bcast_error(ctx);
    if (SDLNet_ResolveHost(&addr, "127.0.0.1", 39000) == -1)
        return bcast_error(ctx);
    if (!(ctx->bcast_socket = SDLNet_TCP_Open(&addr)))
        return bcast_error(ctx);

    bcast_send_str(ctx, "<?xml version=\"1.0\"?>"
                   "<q3map_feedback version=\"1\">");
    return 1;
}

static void bcast_quit(struct mapc_context *ctx)
{
    SDLNet_TCP_Close(ctx->bcast_socket);
    ctx->bcast_socket = NULL;
    SDLNet_Quit();
}

#define MESSAGE(str) do {                       \
        bcast_send_msg(BCAST_STD, (str));       \
        fprintf(stdout, "%s", str);             \
    } while (0)

#define WARNING(str) do {                       \
        bcast_send_msg(BCAST_WRN, (str));       \
        fprintf(stderr, "%s", str);             \
    } while (0)

#define ERROR(str) do {                         \
        bcast_send_msg(BCAST_ERR, (str));       \
        fprintf(stderr, "%s", str);             \
    } while (0)

#else /* ENABLE_RADIANT_CONSOLE */

#define MESSAGE(str) do {                       \
        fprintf(stdout, "%s", str);             \
    } while (0)

#define WARNING(str) do {                       \
        fprintf(stderr, "%s", str);             \
    } while (0)

#define ERROR(str) do {                         \
        fprintf(stderr, "%s", str);             \
    } while (0)

#endif /* ENABLE_RADIANT_CONSOLE */

/*---------------------------------------------------------------------------*/

/* Ohhhh... arbitrary! */

#define MAXM    1024
#define MAXV    65536
#define MAXE    131072
#define MAXS    65536
#define MAXT    131072
#define MAXO    262144
#define MAXG    65536
#define MAXL    4096
#define MAXN    2048
#define MAXP    2048
#define MAXB    1024
#define MAXH    2048
#define MAXZ    1024
#define MAXJ    1024
#define MAXX    1024
#define MAXR    2048
#define MAXU    1024
#define MAXW    1024
#define MAXD    1024
#define MAXA    16384
#define MAXI    262144

/*
 * The following is a small  symbol table data structure.  Symbols and
 * their integer  values are collected  in symv and  valv.  References
 * and pointers  to their unsatisfied integer values  are collected in
 * refv and pntv.  The resolve procedure matches references to symbols
 * and fills waiting ints with the proper values.
 */

#define MAXSYM 2048

enum
{
    SYM_NONE = 0,

    SYM_PATH,
    SYM_TARG,

    SYM_MAX
};

struct sym
{
    int  type;
    char name[MAXSTR];
    int  val;
};

struct ref
{
    int  type;
    char name[MAXSTR];
    int *ptr;
};

struct _imagedata
{
    char *s;
    int w, h;
};

/*
 * Context structure to hold all global state.
 */
struct mapc_context
{
    const char *input_file;
    int debug_output;
    int csv_output;

#if ENABLE_RADIANT_CONSOLE
    TCPsocket bcast_socket;
    unsigned char bcast_msg[MAX_BCAST_MSG];
    size_t bcast_msg_len;
#endif

    struct sym syms[MAXSYM];
    struct ref refs[MAXSYM];
    int symc;
    int refc;

    float targ_p[MAXW][3];
    int targ_wi[MAXW];
    int targ_ji[MAXW];
    int targ_n;

    struct _imagedata *imagedata;
    int image_n;
    int image_alloc;

    float plane_d[MAXS];
    float plane_n[MAXS][3];
    float plane_p[MAXS][3];
    float plane_u[MAXS][3];
    float plane_v[MAXS][3];
    int plane_f[MAXS];
    int plane_m[MAXS];

    int read_dict_entries;

    int mtrl_swaps[MAXM];
    int vert_swaps[MAXV];
    int edge_swaps[MAXE];
    int side_swaps[MAXS];
    int texc_swaps[MAXT];
    int offs_swaps[MAXO];
    int geom_swaps[MAXG];
};

static void mapc_context_init(struct mapc_context *ctx)
{
    memset(ctx, 0, sizeof(*ctx));

    ctx->debug_output = 0;
    ctx->csv_output = 0;
    ctx->input_file = NULL;

#if ENABLE_RADIANT_CONSOLE
    ctx->bcast_socket = NULL;
    ctx->bcast_msg_len = 0;
#endif

    ctx->symc = 0;
    ctx->refc = 0;
    ctx->targ_n = 0;
    ctx->imagedata = NULL;
    ctx->image_n = 0;
    ctx->image_alloc = 0;
    ctx->read_dict_entries = 0;
}

static void mapc_context_cleanup(struct mapc_context *ctx)
{
    int i;

    if (ctx->imagedata)
    {
        for (i = 0; i < ctx->image_n; i++)
            free(ctx->imagedata[i].s);
        free(ctx->imagedata);
        ctx->imagedata = NULL;
    }
    ctx->image_n = 0;
    ctx->image_alloc = 0;

#if ENABLE_RADIANT_CONSOLE
    if (ctx->bcast_socket)
    {
        SDLNet_TCP_Close(ctx->bcast_socket);
        ctx->bcast_socket = NULL;
        SDLNet_Quit();
    }
#endif
}

/*---------------------------------------------------------------------------*/

static int overflow(const char *s)
{
    char buf[64];
    sprintf(buf, "%s overflow\n", s);
    ERROR(buf);
    exit(1);
    return 0;
}

static int incm(struct s_base *fp)
{
    return (fp->mc < MAXM) ? fp->mc++ : overflow("mtrl");
}

static int incv(struct s_base *fp)
{
    return (fp->vc < MAXV) ? fp->vc++ : overflow("vert");
}

static int ince(struct s_base *fp)
{
    return (fp->ec < MAXE) ? fp->ec++ : overflow("edge");
}

static int incs(struct s_base *fp)
{
    return (fp->sc < MAXS) ? fp->sc++ : overflow("side");
}

static int inct(struct s_base *fp)
{
    return (fp->tc < MAXT) ? fp->tc++ : overflow("texc");
}

static int inco(struct s_base *fp)
{
    return (fp->oc < MAXO) ? fp->oc++ : overflow("offs");
}

static int incg(struct s_base *fp)
{
    return (fp->gc < MAXG) ? fp->gc++ : overflow("geom");
}

static int incl(struct s_base *fp)
{
    return (fp->lc < MAXL) ? fp->lc++ : overflow("lump");
}

static int incn(struct s_base *fp)
{
    return (fp->nc < MAXN) ? fp->nc++ : overflow("node");
}

static int incp(struct s_base *fp)
{
    return (fp->pc < MAXP) ? fp->pc++ : overflow("path");
}

static int incb(struct s_base *fp)
{
    return (fp->bc < MAXB) ? fp->bc++ : overflow("body");
}

static int inch(struct s_base *fp)
{
    return (fp->hc < MAXH) ? fp->hc++ : overflow("item");
}

static int incz(struct s_base *fp)
{
    return (fp->zc < MAXZ) ? fp->zc++ : overflow("goal");
}

static int incj(struct s_base *fp)
{
    return (fp->jc < MAXJ) ? fp->jc++ : overflow("jump");
}

static int incx(struct s_base *fp)
{
    return (fp->xc < MAXX) ? fp->xc++ : overflow("swch");
}

static int incr(struct s_base *fp)
{
    return (fp->rc < MAXR) ? fp->rc++ : overflow("bill");
}

static int incu(struct s_base *fp)
{
    return (fp->uc < MAXU) ? fp->uc++ : overflow("ball");
}

static int incw(struct s_base *fp)
{
    return (fp->wc < MAXW) ? fp->wc++ : overflow("view");
}

static int incd(struct s_base *fp)
{
    return (fp->dc < MAXD) ? fp->dc++ : overflow("dict");
}

static int inci(struct s_base *fp)
{
    return (fp->ic < MAXI) ? fp->ic++ : overflow("indx");
}

static void init_file(struct s_base *fp)
{
    fp->mc = 0;
    fp->vc = 0;
    fp->ec = 0;
    fp->sc = 0;
    fp->tc = 0;
    fp->oc = 0;
    fp->gc = 0;
    fp->lc = 0;
    fp->nc = 0;
    fp->pc = 0;
    fp->bc = 0;
    fp->hc = 0;
    fp->zc = 0;
    fp->jc = 0;
    fp->xc = 0;
    fp->rc = 0;
    fp->uc = 0;
    fp->wc = 0;
    fp->dc = 0;
    fp->ac = 0;
    fp->ic = 0;

    fp->mv = (struct b_mtrl *) calloc(MAXM, sizeof (*fp->mv));
    fp->vv = (struct b_vert *) calloc(MAXV, sizeof (*fp->vv));
    fp->ev = (struct b_edge *) calloc(MAXE, sizeof (*fp->ev));
    fp->sv = (struct b_side *) calloc(MAXS, sizeof (*fp->sv));
    fp->tv = (struct b_texc *) calloc(MAXT, sizeof (*fp->tv));
    fp->ov = (struct b_offs *) calloc(MAXO, sizeof (*fp->ov));
    fp->gv = (struct b_geom *) calloc(MAXG, sizeof (*fp->gv));
    fp->lv = (struct b_lump *) calloc(MAXL, sizeof (*fp->lv));
    fp->nv = (struct b_node *) calloc(MAXN, sizeof (*fp->nv));
    fp->pv = (struct b_path *) calloc(MAXP, sizeof (*fp->pv));
    fp->bv = (struct b_body *) calloc(MAXB, sizeof (*fp->bv));
    fp->hv = (struct b_item *) calloc(MAXH, sizeof (*fp->hv));
    fp->zv = (struct b_goal *) calloc(MAXZ, sizeof (*fp->zv));
    fp->jv = (struct b_jump *) calloc(MAXJ, sizeof (*fp->jv));
    fp->xv = (struct b_swch *) calloc(MAXX, sizeof (*fp->xv));
    fp->rv = (struct b_bill *) calloc(MAXR, sizeof (*fp->rv));
    fp->uv = (struct b_ball *) calloc(MAXU, sizeof (*fp->uv));
    fp->wv = (struct b_view *) calloc(MAXW, sizeof (*fp->wv));
    fp->dv = (struct b_dict *) calloc(MAXD, sizeof (*fp->dv));
    fp->av = (char *)          calloc(MAXA, sizeof (*fp->av));
    fp->iv = (int *)           calloc(MAXI, sizeof (*fp->iv));
}

/*---------------------------------------------------------------------------*/

static void make_sym(struct mapc_context *ctx, int type, const char *name, int val)
{
    if (ctx->symc < MAXSYM - 1)
    {
        struct sym *sym = &ctx->syms[ctx->symc];

        sym->type = type;
        strncpy(sym->name, name, MAXSTR - 1);
        sym->val = val;

        ctx->symc++;
    }
}

static void make_ref(struct mapc_context *ctx, int type, const char *name, int *ptr)
{
    if (ctx->refc < MAXSYM - 1)
    {
        struct ref *ref = &ctx->refs[ctx->refc];

        ref->type = type;
        strncpy(ref->name, name, MAXSTR - 1);
        ref->ptr = ptr;

        ctx->refc++;
    }
}

static void resolve(struct mapc_context *ctx)
{
    int i, j;

    for (i = 0; i < ctx->refc; i++)
        for (j = 0; j < ctx->symc; j++)
        {
            struct ref *ref = &ctx->refs[i];
            struct sym *sym = &ctx->syms[j];

            if (ref->type == sym->type && strcmp(ref->name, sym->name) == 0)
            {
                *(ref->ptr) = sym->val;
                break;
            }
        }
}

/*---------------------------------------------------------------------------*/

/*
 * The following globals are used to cache target_positions.  They are
 * targeted by various entities and must be resolved in a second pass.
 */

static void targets(struct mapc_context *ctx, struct s_base *fp)
{
    int i;

    for (i = 0; i < fp->wc; i++)
        v_cpy(fp->wv[i].q, ctx->targ_p[ctx->targ_wi[i]]);

    for (i = 0; i < fp->jc; i++)
        v_cpy(fp->jv[i].q, ctx->targ_p[ctx->targ_ji[i]]);
}

/*---------------------------------------------------------------------------*/

/*
 * The following code caches  image sizes.  Textures are referenced by
 * name,  but  their  sizes   are  necessary  when  computing  texture
 * coordinates.  This code  allows each file to be  accessed only once
 * regardless of the number of surfaces referring to it.
 */

#define IMAGE_REALLOC 32

static int size_load(const char *file, int *w, int *h)
{
    void *p;

    if ((p = image_load(file, w, h, NULL)))
    {
        free(p);
        return 1;
    }
    return 0;
}

static void size_image(struct mapc_context *ctx, const char *name, int *w, int *h)
{
    char path[MAXSTR];
    int i;

    if (ctx->imagedata)
        for (i = 0; i < ctx->image_n; i++)
            if (strncmp(ctx->imagedata[i].s, name, MAXSTR) == 0)
            {
                *w = ctx->imagedata[i].w;
                *h = ctx->imagedata[i].h;

                return;
            }

    *w = 0;
    *h = 0;

    for (i = 0; i < ARRAYSIZE(tex_paths); i++)
    {
        CONCAT_PATH(path, &tex_paths[i], name);

        if (size_load(path, w, h))
            break;
    }

    if (*w > 0 && *h > 0)
    {
        if (ctx->image_n + 1 >= ctx->image_alloc)
        {
            struct _imagedata *tmp =
                (struct _imagedata *) malloc(sizeof(struct _imagedata) * (ctx->image_alloc + IMAGE_REALLOC));
            if (!tmp)
            {
                printf("malloc error\n");
                exit(1);
            }
            if (ctx->imagedata)
            {
                (void) memcpy(tmp, ctx->imagedata, sizeof(struct _imagedata) * ctx->image_alloc);
                free(ctx->imagedata);
            }
            ctx->imagedata = tmp;
            ctx->image_alloc += IMAGE_REALLOC;
        }

        ctx->imagedata[ctx->image_n].s = (char *) calloc(strlen(name) + 1, 1);
        ctx->imagedata[ctx->image_n].w = *w;
        ctx->imagedata[ctx->image_n].h = *h;
        strcpy(ctx->imagedata[ctx->image_n].s, name);

        ctx->image_n++;
    }
}

/*---------------------------------------------------------------------------*/

/* Read the given material file, adding a new material to the solid.  */

static int read_mtrl(struct mapc_context *ctx, struct s_base *fp, const char *name)
{
    static char buf [MAXSTR];

    struct b_mtrl *mp;
    int mi;

    for (mi = 0; mi < fp->mc; mi++)
        if (strncmp(name, fp->mv[mi].f, MAXSTR) == 0)
            return mi;

    mp = fp->mv + incm(fp);

    if (!mtrl_read(mp, name))
    {
        SAFECPY(buf, ctx->input_file);
        SAFECAT(buf, ": unknown material \"");
        SAFECAT(buf, name);
        SAFECAT(buf, "\"\n");
        WARNING(buf);
    }

    return mi;
}

#undef scan_vec4

/*---------------------------------------------------------------------------*/

/*
 * All bodies with an associated  path are assumed to be positioned at
 * the  beginning of that  path.  These  bodies must  be moved  to the
 * origin  in order  for their  path transforms  to  behave correctly.
 * This is  how we get away  with defining func_trains  with no origin
 * specification.
 */

static void move_side(struct b_side *sp, const float p[3])
{
    sp->d -= v_dot(sp->n, p);
}

static void move_vert(struct b_vert *vp, const float p[3])
{
    v_sub(vp->p, vp->p, p);
}

static void move_lump(struct s_base *fp,
                      struct b_lump *lp, const float p[3])
{
    int i;

    for (i = 0; i < lp->sc; i++)
        move_side(fp->sv + fp->iv[lp->s0 + i], p);
    for (i = 0; i < lp->vc; i++)
        move_vert(fp->vv + fp->iv[lp->v0 + i], p);
}

static void move_body(struct s_base *fp,
                      struct b_body *bp)
{
    int i, *b;

    /* Move the lumps. */

    for (i = 0; i < bp->lc; i++)
        move_lump(fp, fp->lv + bp->l0 + i, fp->pv[bp->p0].p);

    /* Create an array to mark any verts referenced by moved geoms. */

    if (bp->gc > 0 && (b = (int *) calloc(fp->vc, sizeof (int))))
    {
        /* Mark the verts. */

        for (i = 0; i < bp->gc; i++)
        {
            const struct b_geom *gp = fp->gv + fp->iv[bp->g0 + i];

            b[fp->ov[gp->oi].vi] = 1;
            b[fp->ov[gp->oj].vi] = 1;
            b[fp->ov[gp->ok].vi] = 1;
        }

        /* Apply the motion to the marked vertices. */

        for (i = 0; i < fp->vc; ++i)
            if (b[i])
                move_vert(fp->vv + i, fp->pv[bp->p0].p);

        free(b);
    }
}

static void move_item(struct s_base *fp,
                      struct b_item *hp)
{
    v_sub(hp->p, hp->p, fp->pv[hp->p0].p);
}

static void move_goal(struct s_base *fp,
                      struct b_goal *zp)
{
    v_sub(zp->p, zp->p, fp->pv[zp->p0].p);
}

static void move_jump(struct s_base *fp,
                      struct b_jump *jp)
{
    v_sub(jp->p, jp->p, fp->pv[jp->p0].p);
}

static void move_swch(struct s_base *fp,
                      struct b_swch *xp)
{
    v_sub(xp->p, xp->p, fp->pv[xp->p0].p);
}

static void move_bill(struct s_base *fp,
                      struct b_bill *rp)
{
    v_sub(rp->p, rp->p, fp->pv[rp->p0].p);
}

static void move_file(struct mapc_context *ctx, struct s_base *fp)
{
    int i;

    for (i = 0; i < fp->bc; i++)
        if (fp->bv[i].p0 >= 0)
            move_body(fp, fp->bv + i);

    for (i = 0; i < fp->hc; i++)
        if (fp->hv[i].p0 >= 0)
            move_item(fp, fp->hv + i);

    for (i = 0; i < fp->zc; i++)
        if (fp->zv[i].p0 >= 0)
            move_goal(fp, fp->zv + i);

    for (i = 0; i < fp->jc; i++)
        if (fp->jv[i].p0 >= 0)
            move_jump(fp, fp->jv + i);

    for (i = 0; i < fp->xc; i++)
        if (fp->xv[i].p0 >= 0)
            move_swch(fp, fp->xv + i);

    for (i = 0; i < fp->rc; i++)
        if (fp->rv[i].p0 >= 0)
            move_bill(fp, fp->rv + i);

    /* Paths must be moved last and all at once. */

    if (fp->pc > 0)
    {
        float (*posv)[3] = calloc(fp->pc, sizeof (float[3]));

        if (posv)
        {
            for (i = 0; i < fp->pc; i++)
            {
                struct b_path *pp = fp->pv + i;

                if (pp->p0 >= 0)
                {
                    struct b_path *pq = fp->pv + pp->p0;

                    v_sub(posv[i], pp->p, pq->p);
                }
            }

            for (i = 0; i < fp->pc; i++)
            {
                struct b_path *pp = fp->pv + i;

                if (pp->p0 >= 0)
                    v_cpy(fp->pv[i].p, posv[i]);
            }

            free(posv);
            posv = NULL;
        }
    }
}

/*---------------------------------------------------------------------------*/

/*
 * This is a basic OBJ loader.  It is by no means fully compliant with
 * the  OBJ  specification, but  it  works  well  with the  output  of
 * Wings3D.  All faces must be triangles and all vertices must include
 * normals and  texture coordinates.  Material  names are taken  to be
 * references to Neverball materials, rather than MTL definitions.
 */

static void read_vt(struct s_base *fp, const char *line)
{
    struct b_texc *tp = fp->tv + inct(fp);

    sscanf(line, "%f %f", tp->u, tp->u + 1);
}

static void read_vn(struct s_base *fp, const char *line)
{
    struct b_side *sp = fp->sv + incs(fp);

    sscanf(line, "%f %f %f", sp->n, sp->n + 1, sp->n + 2);
}

static void read_v(struct s_base *fp, const char *line)
{
    struct b_vert *vp = fp->vv + incv(fp);

    sscanf(line, "%f %f %f", vp->p, vp->p + 1, vp->p + 2);
}

static void read_f(struct s_base *fp, const char *line,
                   int v0, int t0, int s0, int mi)
{
    struct b_geom *gp = fp->gv + incg(fp);

    struct b_offs *op = fp->ov + (gp->oi = inco(fp));
    struct b_offs *oq = fp->ov + (gp->oj = inco(fp));
    struct b_offs *or = fp->ov + (gp->ok = inco(fp));

    char c1;
    char c2;

    sscanf(line, "%d%c%d%c%d %d%c%d%c%d %d%c%d%c%d",
           &op->vi, &c1, &op->ti, &c2, &op->si,
           &oq->vi, &c1, &oq->ti, &c2, &oq->si,
           &or->vi, &c1, &or->ti, &c2, &or->si);

    op->vi += (v0 - 1);
    oq->vi += (v0 - 1);
    or->vi += (v0 - 1);
    op->ti += (t0 - 1);
    oq->ti += (t0 - 1);
    or->ti += (t0 - 1);
    op->si += (s0 - 1);
    oq->si += (s0 - 1);
    or->si += (s0 - 1);

    gp->mi  = mi;
}

static void read_obj(struct mapc_context *ctx, struct s_base *fp, const char *name, int mi)
{
    char line[MAXSTR];
    char mtrl[MAXSTR];
    fs_file fin;

    int v0 = fp->vc;
    int t0 = fp->tc;
    int s0 = fp->sc;

    if ((fin = fs_open_read(name)))
    {
        while (fs_gets(line, MAXSTR, fin))
        {
            if (strncmp(line, "usemtl", 6) == 0)
            {
                sscanf(line + 6, "%s", mtrl);
                mi = read_mtrl(ctx, fp, mtrl);
            }

            else if (strncmp(line, "f", 1) == 0)
            {
                if (fp->mv[mi].d[3] > 0.0f)
                    read_f(fp, line + 1, v0, t0, s0, mi);
            }

            else if (strncmp(line, "vt", 2) == 0) read_vt(fp, line + 2);
            else if (strncmp(line, "vn", 2) == 0) read_vn(fp, line + 2);
            else if (strncmp(line, "v",  1) == 0) read_v (fp, line + 1);
        }
        fs_close(fin);
    }
}

/*---------------------------------------------------------------------------*/

static void make_plane(struct mapc_context *ctx, int   pi, float x0, float y0, float      z0,
                       float x1, float y1, float z1,
                       float x2, float y2, float z2,
                       float tu, float tv, float r,
                       float su, float sv, int   fl, const char *s)
{
    static const float base[6][3][3] = {
        {{  0,  0,  1 }, {  1,  0,  0 }, {  0,  1,  0 }},
        {{  0,  0, -1 }, {  1,  0,  0 }, {  0,  1,  0 }},
        {{  1,  0,  0 }, {  0,  0, -1 }, {  0,  1,  0 }},
        {{ -1,  0,  0 }, {  0,  0, -1 }, {  0,  1,  0 }},
        {{  0,  1,  0 }, {  1,  0,  0 }, {  0,  0, -1 }},
        {{  0, -1,  0 }, {  1,  0,  0 }, {  0,  0, -1 }},
    };

    float R[16];
    float p0[3], p1[3], p2[3];
    float u[3],  v[3],  p[3];
    float k, d = 0.0f;
    int   i, n = 0;
    int   w, h;

    size_image(ctx, s, &w, &h);

    ctx->plane_f[pi] = fl ? L_DETAIL : 0;

    p0[0] = +x0 / SCALE;
    p0[1] = +z0 / SCALE;
    p0[2] = -y0 / SCALE;

    p1[0] = +x1 / SCALE;
    p1[1] = +z1 / SCALE;
    p1[2] = -y1 / SCALE;

    p2[0] = +x2 / SCALE;
    p2[1] = +z2 / SCALE;
    p2[2] = -y2 / SCALE;

    v_sub(u, p0, p1);
    v_sub(v, p2, p1);

    v_crs(ctx->plane_n[pi], u, v);
    v_nrm(ctx->plane_n[pi], ctx->plane_n[pi]);

    ctx->plane_d[pi] = v_dot(ctx->plane_n[pi], p1);

    for (i = 0; i < 6; i++)
        if ((k = v_dot(ctx->plane_n[pi], base[i][0])) >= d)
        {
            d = k;
            n = i;
        }

    p[0] = 0.f;
    p[1] = 0.f;
    p[2] = 0.f;

    /* Always rotate around the positive axis */

    m_rot(R, base[n - (n % 2)][0], V_RAD(r));

    v_mad(p, p, base[n][1], +su * tu / SCALE);
    v_mad(p, p, base[n][2], -sv * tv / SCALE);

    m_vxfm(ctx->plane_u[pi], R, base[n][1]);
    m_vxfm(ctx->plane_v[pi], R, base[n][2]);
    m_vxfm(ctx->plane_p[pi], R, p);

    v_scl(ctx->plane_u[pi], ctx->plane_u[pi], 64.f / w);
    v_scl(ctx->plane_v[pi], ctx->plane_v[pi], 64.f / h);

    v_scl(ctx->plane_u[pi], ctx->plane_u[pi], 1.f / su);
    v_scl(ctx->plane_v[pi], ctx->plane_v[pi], 1.f / sv);
}

/*---------------------------------------------------------------------------*/

#define T_EOF 0
#define T_BEG 1
#define T_CLP 2
#define T_KEY 3
#define T_END 4
#define T_NOP 5

static int map_token(struct mapc_context *ctx, fs_file fin, int pi, char key[MAXSTR], char val[MAXSTR])
{
    char buf[MAXSTR];

    if (fs_gets(buf, MAXSTR, fin))
    {
        float x0, y0, z0;
        float x1, y1, z1;
        float x2, y2, z2;
        float tu, tv, r;
        float su, sv;
        int fl = 0;

        /* Scan the beginning or end of a block. */

        if (buf[0] == '{') return T_BEG;
        if (buf[0] == '}') return T_END;

        /* Scan a key-value pair. */

        if (buf[0] == '\"')
        {
            strcpy(key, strtok(buf,  "\""));
            (void)      strtok(NULL, "\"");
            strcpy(val, strtok(NULL, "\""));

            return T_KEY;
        }

        /* Scan a plane. */

        if (sscanf(buf,
                   "( %f %f %f ) "
                   "( %f %f %f ) "
                   "( %f %f %f ) "
                   "%s %f %f %f %f %f %d",
                   &x0, &y0, &z0,
                   &x1, &y1, &z1,
                   &x2, &y2, &z2,
                   key, &tu, &tv, &r, &su, &sv, &fl) >= 15)
        {
            make_plane(ctx, pi, x0, y0, z0,
                       x1, y1, z1,
                       x2, y2, z2,
                       tu, tv, r, su, sv, fl, key);
            return T_CLP;
        }

        /* If it's not recognized, it must be uninteresting. */

        return T_NOP;
    }
    return T_EOF;
}

/*---------------------------------------------------------------------------*/

/* Parse a lump from the given file and add it to the solid. */

static void read_lump(struct mapc_context *ctx, struct s_base *fp, fs_file fin)
{
    char k[MAXSTR];
    char v[MAXSTR];
    int t;

    struct b_lump *lp = fp->lv + incl(fp);

    lp->s0 = fp->ic;

    while ((t = map_token(ctx, fin, fp->sc, k, v)))
    {
        if (t == T_CLP)
        {
            fp->sv[fp->sc].n[0] = ctx->plane_n[fp->sc][0];
            fp->sv[fp->sc].n[1] = ctx->plane_n[fp->sc][1];
            fp->sv[fp->sc].n[2] = ctx->plane_n[fp->sc][2];
            fp->sv[fp->sc].d    = ctx->plane_d[fp->sc];

            ctx->plane_m[fp->sc] = read_mtrl(ctx, fp, k);

            fp->iv[fp->ic] = fp->sc;
            inci(fp);
            incs(fp);
            lp->sc++;
        }
        if (t == T_END)
            break;
    }
}

/*---------------------------------------------------------------------------*/

static void make_path(struct mapc_context *ctx, struct s_base *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i, pi = incp(fp);

    struct b_path *pp = fp->pv + pi;

    pp->p[0] = 0.f;
    pp->p[1] = 0.f;
    pp->p[2] = 0.f;
    pp->t    = 1.f;
    pp->pi   = pi;
    pp->f    = 1;
    pp->s    = 1;

    pp->p0 = pp->p1 = -1;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "targetname") == 0)
            make_sym(ctx, SYM_PATH, v[i], pi);

        if (strcmp(k[i], "target") == 0 || strcmp(k[i], "target1") == 0)
            make_ref(ctx, SYM_PATH, v[i], &pp->pi);

        if (strcmp(k[i], "target2") == 0)
        {
            make_ref(ctx, SYM_PATH, v[i], &pp->p0);
            pp->fl |= P_PARENTED;
        }

        if (strcmp(k[i], "target3") == 0)
        {
            make_ref(ctx, SYM_PATH, v[i], &pp->p1);
            pp->fl |= P_PARENTED;
        }

        if (strcmp(k[i], "state") == 0)
            pp->f = atoi(v[i]);

        if (strcmp(k[i], "speed") == 0)
            sscanf(v[i], "%f", &pp->t);

        if (strcmp(k[i], "smooth") == 0)
            pp->s = atoi(v[i]);

        if (strcmp(k[i], "origin") == 0)
        {
            float x = 0.f, y = 0.f, z = 0.f;

            sscanf(v[i], "%f %f %f", &x, &y, &z);

            pp->p[0] = +x / SCALE;
            pp->p[1] = +z / SCALE;
            pp->p[2] = -y / SCALE;
        }

        /*
         * Radiant sets "angle" for yaw-only rotations, "angles"
         * otherwise.  Angles takes priority, so check for angle
         * first.
         */

        if (strcmp(k[i], "angle") == 0)
        {
            static const float Y[3] = { 0.0f, 1.0f, 0.0f };

            float y = 0.0f;

            /* Yaw. */

            sscanf(v[i], "%f", &y);
            q_by_axisangle(pp->e, Y, V_RAD(+y));
            pp->fl |= P_ORIENTED;
        }

        if (strcmp(k[i], "angles") == 0)
        {
            static const float X[3] = { 1.0f, 0.0f, 0.0f };
            static const float Y[3] = { 0.0f, 1.0f, 0.0f };
            static const float Z[3] = { 0.0f, 0.0f, 1.0f };

            float x = 0.0f, y = 0.0f, z = 0.0f;
            float d[4], e[4];

            /* Pitch, yaw and roll. */

            sscanf(v[i], "%f %f %f", &x, &y, &z);

            q_by_axisangle(pp->e, Y, V_RAD(+y));

            q_by_axisangle(d, Z, V_RAD(-x));
            q_mul(e, pp->e, d);
            q_nrm(pp->e, e);

            q_by_axisangle(d, X, V_RAD(+z));
            q_mul(e, pp->e, d);
            q_nrm(pp->e, e);

            pp->fl |= P_ORIENTED;
        }
    }
}

static void make_dict(struct s_base *fp,
                      const char *k,
                      const char *v)
{
    int space_left, space_needed, di = incd(fp);

    struct b_dict *dp = fp->dv + di;

    space_left   = MAXA - fp->ac;
    space_needed = strlen(k) + 1 + strlen(v) + 1;

    if (space_needed > space_left)
    {
        fp->dc--;
        return;
    }

    dp->ai = fp->ac;
    dp->aj = dp->ai + strlen(k) + 1;
    fp->ac = dp->aj + strlen(v) + 1;

    memcpy(fp->av + dp->ai, k, strlen(k) + 1);
    memcpy(fp->av + dp->aj, v, strlen(v) + 1);
}

static void make_body(struct mapc_context *ctx, struct s_base *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c, int l0)
{
    int i, mi = 0, bi = incb(fp);

    int g0 = fp->gc;
    int v0 = fp->vc;

    float p[3];

    float x = 0.f;
    float y = 0.f;
    float z = 0.f;

    struct b_body *bp = fp->bv + bi;

    bp->p0 = -1;
    bp->p1 = -1;
    bp->ni = -1;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "target") == 0 || strcmp(k[i], "target1") == 0)
            make_ref(ctx, SYM_PATH, v[i], &bp->p0);

        else if (strcmp(k[i], "target2") == 0)
            make_ref(ctx, SYM_PATH, v[i], &bp->p1);

        else if (strcmp(k[i], "material") == 0)
            mi = read_mtrl(ctx, fp, v[i]);

        else if (strcmp(k[i], "model") == 0)
            read_obj(ctx, fp, v[i], mi);

        else if (strcmp(k[i], "origin") == 0)
            sscanf(v[i], "%f %f %f", &x, &y, &z);

        else if (ctx->read_dict_entries && strcmp(k[i], "classname") != 0)
            make_dict(fp, k[i], v[i]);
    }

    bp->l0 = l0;
    bp->lc = fp->lc - l0;
    bp->g0 = fp->ic;
    bp->gc = fp->gc - g0;

    for (i = 0; i < bp->gc; i++)
        fp->iv[inci(fp)] = g0++;

    p[0] = +x / SCALE;
    p[1] = +z / SCALE;
    p[2] = -y / SCALE;

    for (i = v0; i < fp->vc; i++)
        v_add(fp->vv[i].p, fp->vv[i].p, p);

    ctx->read_dict_entries = 0;
}

static void make_item(struct mapc_context *ctx, struct s_base *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i, hi = inch(fp);

    struct b_item *hp = fp->hv + hi;

    hp->p[0] = 0.f;
    hp->p[1] = 0.f;
    hp->p[2] = 0.f;

    hp->t = ITEM_NONE;
    hp->n = 0;

    hp->p0 = hp->p1 = -1;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "classname") == 0)
        {
            if (strcmp(v[i], "light") == 0)
                hp->t = ITEM_COIN;
            else if (strcmp(v[i], "item_health_large") == 0)
                hp->t = ITEM_GROW;
            else if (strcmp(v[i], "item_health_small") == 0)
                hp->t = ITEM_SHRINK;
            else if (strcmp(v[i], "item_clock") == 0)
                hp->t = ITEM_CLOCK;
        }

        if (strcmp(k[i], "light") == 0)
            sscanf(v[i], "%d", &hp->n);

        if (strcmp(k[i], "origin") == 0)
        {
            float x = 0.f, y = 0.f, z = 0.f;

            sscanf(v[i], "%f %f %f", &x, &y, &z);

            hp->p[0] = +x / SCALE;
            hp->p[1] = +z / SCALE;
            hp->p[2] = -y / SCALE;
        }

        if (strcmp(k[i], "target") == 0 || strcmp(k[i], "target1") == 0)
            make_ref(ctx, SYM_PATH, v[i], &hp->p0);

        else if (strcmp(k[i], "target2") == 0)
            make_ref(ctx, SYM_PATH, v[i], &hp->p1);
    }
}

static void make_bill(struct mapc_context *ctx, struct s_base *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i, ri = incr(fp);

    struct b_bill *rp = fp->rv + ri;

    memset(rp, 0, sizeof (struct b_bill));
    rp->t = 1.0f;

    rp->p0 = rp->p1 = -1;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "width") == 0)
            sscanf(v[i], "%f %f %f", rp->w, rp->w + 1, rp->w + 2);
        if (strcmp(k[i], "height") == 0)
            sscanf(v[i], "%f %f %f", rp->h, rp->h + 1, rp->h + 2);

        if (strcmp(k[i], "xrot") == 0)
            sscanf(v[i], "%f %f %f", rp->rx, rp->rx + 1, rp->rx + 2);
        if (strcmp(k[i], "yrot") == 0)
            sscanf(v[i], "%f %f %f", rp->ry, rp->ry + 1, rp->ry + 2);
        if (strcmp(k[i], "zrot") == 0)
            sscanf(v[i], "%f %f %f", rp->rz, rp->rz + 1, rp->rz + 2);

        if (strcmp(k[i], "time") == 0)
            sscanf(v[i], "%f", &rp->t);
        if (strcmp(k[i], "dist") == 0)
            sscanf(v[i], "%f", &rp->d);
        if (strcmp(k[i], "flag") == 0)
            sscanf(v[i], "%d", &rp->fl);

        if (strcmp(k[i], "image") == 0)
        {
            rp->mi = read_mtrl(ctx, fp, v[i]);
            fp->mv[rp->mi].fl |= M_CLAMP_S | M_CLAMP_T;
        }

        if (strcmp(k[i], "origin") == 0)
        {
            float x = 0.f, y = 0.f, z = 0.f;

            sscanf(v[i], "%f %f %f", &x, &y, &z);

            rp->p[0] = +x / SCALE;
            rp->p[1] = +z / SCALE;
            rp->p[2] = -y / SCALE;
        }

        if (strcmp(k[i], "target") == 0 || strcmp(k[i], "target1") == 0)
            make_ref(ctx, SYM_PATH, v[i], &rp->p0);

        else if (strcmp(k[i], "target2") == 0)
            make_ref(ctx, SYM_PATH, v[i], &rp->p1);
    }
}

static void make_goal(struct mapc_context *ctx, struct s_base *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i, zi = incz(fp);

    struct b_goal *zp = fp->zv + zi;

    zp->p[0] = 0.f;
    zp->p[1] = 0.f;
    zp->p[2] = 0.f;
    zp->r    = 0.75;

    zp->p0 = zp->p1 = -1;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "radius") == 0)
            sscanf(v[i], "%f", &zp->r);

        if (strcmp(k[i], "origin") == 0)
        {
            float x = 0.f, y = 0.f, z = 0.f;

            sscanf(v[i], "%f %f %f", &x, &y, &z);

            zp->p[0] = +(x)      / SCALE;
            zp->p[1] = +(z - 24) / SCALE;
            zp->p[2] = -(y)      / SCALE;
        }

        if (strcmp(k[i], "target") == 0 || strcmp(k[i], "target1") == 0)
            make_ref(ctx, SYM_PATH, v[i], &zp->p0);

        else if (strcmp(k[i], "target2") == 0)
            make_ref(ctx, SYM_PATH, v[i], &zp->p1);
    }
}

static void make_view(struct mapc_context *ctx, struct s_base *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i, wi = incw(fp);

    struct b_view *wp = fp->wv + wi;

    wp->p[0] = 0.f;
    wp->p[1] = 0.f;
    wp->p[2] = 0.f;
    wp->q[0] = 0.f;
    wp->q[1] = 0.f;
    wp->q[2] = 0.f;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "target") == 0)
            make_ref(ctx, SYM_TARG, v[i], ctx->targ_wi + wi);

        if (strcmp(k[i], "origin") == 0)
        {
            float x = 0.f, y = 0.f, z = 0.f;

            sscanf(v[i], "%f %f %f", &x, &y, &z);

            wp->p[0] = +x / SCALE;
            wp->p[1] = +z / SCALE;
            wp->p[2] = -y / SCALE;
        }
    }
}

static void make_jump(struct mapc_context *ctx, struct s_base *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i, ji = incj(fp);

    struct b_jump *jp = fp->jv + ji;

    jp->p[0] = 0.f;
    jp->p[1] = 0.f;
    jp->p[2] = 0.f;
    jp->q[0] = 0.f;
    jp->q[1] = 0.f;
    jp->q[2] = 0.f;
    jp->r    = 0.5;

    jp->p0 = jp->p1 = -1;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "radius") == 0)
            sscanf(v[i], "%f", &jp->r);

        if (strcmp(k[i], "target") == 0)
            make_ref(ctx, SYM_TARG, v[i], ctx->targ_ji + ji);

        if (strcmp(k[i], "origin") == 0)
        {
            float x = 0.f, y = 0.f, z = 0.f;

            sscanf(v[i], "%f %f %f", &x, &y, &z);

            jp->p[0] = +x / SCALE;
            jp->p[1] = +z / SCALE;
            jp->p[2] = -y / SCALE;
        }

        if (strcmp(k[i], "target2") == 0)
            make_ref(ctx, SYM_PATH, v[i], &jp->p0);

        if (strcmp(k[i], "target3") == 0)
            make_ref(ctx, SYM_PATH, v[i], &jp->p1);
    }
}

static void make_swch(struct mapc_context *ctx, struct s_base *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i, xi = incx(fp);

    struct b_swch *xp = fp->xv + xi;

    xp->p[0] = 0.f;
    xp->p[1] = 0.f;
    xp->p[2] = 0.f;
    xp->r    = 0.5;
    xp->pi   = 0;
    xp->t    = 0;
    xp->f    = 0;
    xp->i    = 0;

    xp->p0 = xp->p1 = -1;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "radius") == 0)
            sscanf(v[i], "%f", &xp->r);

        if (strcmp(k[i], "target") == 0)
            make_ref(ctx, SYM_PATH, v[i], &xp->pi);

        if (strcmp(k[i], "timer") == 0)
            sscanf(v[i], "%f", &xp->t);

        if (strcmp(k[i], "state") == 0)
            xp->f = atoi(v[i]);

        if (strcmp(k[i], "invisible") == 0)
            xp->i = atoi(v[i]);

        if (strcmp(k[i], "origin") == 0)
        {
            float x = 0.f, y = 0.f, z = 0.f;

            sscanf(v[i], "%f %f %f", &x, &y, &z);

            xp->p[0] = +x / SCALE;
            xp->p[1] = +z / SCALE;
            xp->p[2] = -y / SCALE;
        }

        if (strcmp(k[i], "target2") == 0)
            make_ref(ctx, SYM_PATH, v[i], &xp->p0);

        else if (strcmp(k[i], "target3") == 0)
            make_ref(ctx, SYM_PATH, v[i], &xp->p1);
    }
}

static void make_targ(struct mapc_context *ctx, struct s_base *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i;

    ctx->targ_p[ctx->targ_n][0] = 0.f;
    ctx->targ_p[ctx->targ_n][1] = 0.f;
    ctx->targ_p[ctx->targ_n][2] = 0.f;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "targetname") == 0)
            make_sym(ctx, SYM_TARG, v[i], ctx->targ_n);

        if (strcmp(k[i], "origin") == 0)
        {
            float x = 0.f, y = 0.f, z = 0.f;

            sscanf(v[i], "%f %f %f", &x, &y, &z);

            ctx->targ_p[ctx->targ_n][0] = +x / SCALE;
            ctx->targ_p[ctx->targ_n][1] = +z / SCALE;
            ctx->targ_p[ctx->targ_n][2] = -y / SCALE;
        }
    }

    if (++ctx->targ_n == MAXW)
        overflow("target");
}

static void make_ball(struct mapc_context *ctx, struct s_base *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i, ui = incu(fp);

    struct b_ball *up = fp->uv + ui;

    up->p[0] = 0.0f;
    up->p[1] = 0.0f;
    up->p[2] = 0.0f;
    up->r    = 0.25f;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "radius") == 0)
            sscanf(v[i], "%f", &up->r);

        if (strcmp(k[i], "origin") == 0)
        {
            float x = 0.f, y = 0.f, z = 0.f;

            sscanf(v[i], "%f %f %f", &x, &y, &z);

            up->p[0] = +(x)      / SCALE;
            up->p[1] = +(z - 24) / SCALE;
            up->p[2] = -(y)      / SCALE;
        }
    }

    up->p[1] += up->r + SMALL;
}

/*---------------------------------------------------------------------------*/

static void read_ent(struct mapc_context *ctx, struct s_base *fp, fs_file fin)
{
    char k[MAXKEY][MAXSTR];
    char v[MAXKEY][MAXSTR];
    int t, i = 0, c = 0;

    int l0 = fp->lc;

    while ((t = map_token(ctx, fin, -1, k[c], v[c])))
    {
        if (t == T_KEY)
        {
            if (strcmp(k[c], "classname") == 0)
                i = c;
            c++;
        }
        if (t == T_BEG) read_lump(ctx, fp, fin);
        if (t == T_END) break;
    }

    if (!strcmp(v[i], "light"))                    make_item(ctx, fp, k, v, c);
    if (!strcmp(v[i], "item_health_large"))        make_item(ctx, fp, k, v, c);
    if (!strcmp(v[i], "item_health_small"))        make_item(ctx, fp, k, v, c);
    if (!strcmp(v[i], "item_clock"))               make_item(ctx, fp, k, v, c);
    if (!strcmp(v[i], "info_camp"))                make_swch(ctx, fp, k, v, c);
    if (!strcmp(v[i], "info_null"))                make_bill(ctx, fp, k, v, c);
    if (!strcmp(v[i], "path_corner"))              make_path(ctx, fp, k, v, c);
    if (!strcmp(v[i], "info_player_start"))        make_ball(ctx, fp, k, v, c);
    if (!strcmp(v[i], "info_player_intermission")) make_view(ctx, fp, k, v, c);
    if (!strcmp(v[i], "info_player_deathmatch"))   make_goal(ctx, fp, k, v, c);
    if (!strcmp(v[i], "target_teleporter"))        make_jump(ctx, fp, k, v, c);
    if (!strcmp(v[i], "target_position"))          make_targ(ctx, fp, k, v, c);
    if (!strcmp(v[i], "worldspawn"))
    {
        ctx->read_dict_entries = 1;
        make_body(ctx, fp, k, v, c, l0);
    }
    if (!strcmp(v[i], "func_train"))               make_body(ctx, fp, k, v, c, l0);
    if (!strcmp(v[i], "misc_model"))               make_body(ctx, fp, k, v, c, l0);

    /* TrenchBroom compatibility: if func_group has any lumps, add it as a body; ignore otherwise. */

    if (!strcmp(v[i], "func_group") && fp->lc > l0)
        make_body(ctx, fp, k, v, c, l0);
}

static void read_map(struct mapc_context *ctx, struct s_base *fp, fs_file fin)
{
    char k[MAXSTR];
    char v[MAXSTR];
    int t;

    while ((t = map_token(ctx, fin, -1, k, v)))
        if (t == T_BEG)
            read_ent(ctx, fp, fin);
}

/*---------------------------------------------------------------------------*/

/* Test the location of a point with respect to a side plane. */

static int fore_side(const float p[3], const struct b_side *sp)
{
    return (v_dot(p, sp->n) - sp->d > +SMALL) ? 1 : 0;
}

static int on_side(const float p[3], const struct b_side *sp)
{
    float d = v_dot(p, sp->n) - sp->d;

    return (-SMALL < d && d < +SMALL) ? 1 : 0;
}

/*---------------------------------------------------------------------------*/
/*
 * Confirm  that  the addition  of  a vert  would  not  result in  degenerate
 * geometry.
 */

static int ok_vert(const struct s_base *fp,
                   const struct b_lump *lp, const float p[3])
{
    float r[3];
    int i;

    for (i = 0; i < lp->vc; i++)
    {
        float *q = fp->vv[fp->iv[lp->v0 + i]].p;

        v_sub(r, p, q);

        if (v_len(r) < SMALL)
            return 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

/*
 * The following functions take the  set of planes defining a lump and
 * find the verts, edges, and  geoms that describe its boundaries.  To
 * do this, they first find the verts, and then search these verts for
 * valid edges and  geoms.  It may be more  efficient to compute edges
 * and  geoms directly  by clipping  down infinite  line  segments and
 * planes,  but this  would be  more  complex and  prone to  numerical
 * error.
 */

/*
 * Given 3  side planes,  compute the point  of intersection,  if any.
 * Confirm that this point falls  within the current lump, and that it
 * is unique.  Add it as a vert of the solid.
 */
static void clip_vert(struct mapc_context *ctx, struct s_base *fp,
                      struct b_lump *lp, int si, int sj, int sk)
{
    float M[16], X[16], I[16];
    float d[3],  p[3];
    int i;

    d[0] = fp->sv[si].d;
    d[1] = fp->sv[sj].d;
    d[2] = fp->sv[sk].d;

    m_basis(M, fp->sv[si].n, fp->sv[sj].n, fp->sv[sk].n);
    m_xps(X, M);

    if (m_inv(I, X))
    {
        m_vxfm(p, I, d);

        for (i = 0; i < lp->sc; i++)
        {
            int sl = fp->iv[lp->s0 + i];

            if (fore_side(p, fp->sv + sl))
                return;
        }

        if (ok_vert(fp, lp, p))
        {
            int vi = incv(fp);

            v_cpy(fp->vv[vi].p, p);

            fp->iv[inci(fp)] = vi;
            lp->vc++;
        }
    }
}

/*
 * Given two  side planes,  find an edge  along their  intersection by
 * finding a pair of vertices that fall on both planes.  Add it to the
 * solid.
 */
static void clip_edge(struct mapc_context *ctx,
                      struct s_base *fp,
                      struct b_lump *lp, int si, int sj)
{
    int i, j;

    for (i = 1; i < lp->vc; i++)
    {
        int vi = fp->iv[lp->v0 + i];

        if (!on_side(fp->vv[vi].p, fp->sv + si) ||
            !on_side(fp->vv[vi].p, fp->sv + sj))
            continue;

        for (j = 0; j < i; j++)
        {
            int vj = fp->iv[lp->v0 + j];

            if (on_side(fp->vv[vj].p, fp->sv + si) &&
                on_side(fp->vv[vj].p, fp->sv + sj))
            {
                fp->ev[fp->ec].vi = vi;
                fp->ev[fp->ec].vj = vj;

                fp->iv[fp->ic] = fp->ec;

                inci(fp);
                ince(fp);
                lp->ec++;
            }
        }
    }
}

/*
 * Find all verts that lie on  the given side of the lump.  Sort these
 * verts to  have a counter-clockwise winding about  the plane normal.
 * Create geoms to tessellate the resulting convex polygon.
 */
static void clip_geom(struct mapc_context *ctx, struct s_base *fp,
                      struct b_lump *lp, int si)
{
    int   m[256], t[256], d, i, j, n = 0;
    float u[3];
    float v[3];
    float w[3];

    struct b_side *sp = fp->sv + si;

    /* Find em. */

    for (i = 0; i < lp->vc; i++)
    {
        int vi = fp->iv[lp->v0 + i];

        if (on_side(fp->vv[vi].p, sp))
        {
            m[n] = vi;
            t[n] = inct(fp);

            v_add(v, fp->vv[vi].p, ctx->plane_p[si]);

            fp->tv[t[n]].u[0] = v_dot(v, ctx->plane_u[si]);
            fp->tv[t[n]].u[1] = v_dot(v, ctx->plane_v[si]);

            if (++n >= ARRAYSIZE(m))
            {
                ERROR("Over 256 vertices on one side, skipping the rest\n");
                break;
            }
        }
    }

    /* Sort em. */

    for (i = 1; i < n; i++)
        for (j = i + 1; j < n; j++)
        {
            v_sub(u, fp->vv[m[i]].p, fp->vv[m[0]].p);
            v_sub(v, fp->vv[m[j]].p, fp->vv[m[0]].p);
            v_crs(w, u, v);

            if (v_dot(w, sp->n) < 0.f)
            {
                d     = m[i];
                m[i]  = m[j];
                m[j]  =    d;

                d     = t[i];
                t[i]  = t[j];
                t[j]  =    d;
            }
        }

    /* Index em. */

    for (i = 0; i < n - 2; i++)
    {
        const int gi = incg(fp);

        struct b_geom *gp = fp->gv + gi;

        struct b_offs *op = fp->ov + (gp->oi = inco(fp));
        struct b_offs *oq = fp->ov + (gp->oj = inco(fp));
        struct b_offs *or = fp->ov + (gp->ok = inco(fp));

        gp->mi = ctx->plane_m[si];

        op->ti = t[0];
        oq->ti = t[i + 1];
        or->ti = t[i + 2];

        op->si = si;
        oq->si = si;
        or->si = si;

        op->vi = m[0];
        oq->vi = m[i + 1];
        or->vi = m[i + 2];

        fp->iv[fp->ic] = gi;
        lp->gc++;
        inci(fp);
    }
}

/*
 * Iterate the sides of the lump, attempting to generate a new vert for
 * each trio of planes, a new edge  for each pair of planes, and a new
 * set of geom for each visible plane.
 */
static void clip_lump(struct mapc_context *ctx, struct s_base *fp, struct b_lump *lp)
{
    int i, j, k;

    lp->v0 = fp->ic;
    lp->vc = 0;

    for (i = 2; i < lp->sc; i++)
        for (j = 1; j < i; j++)
            for (k = 0; k < j; k++)
                clip_vert(ctx, fp, lp,
                          fp->iv[lp->s0 + i],
                          fp->iv[lp->s0 + j],
                          fp->iv[lp->s0 + k]);

    lp->e0 = fp->ic;
    lp->ec = 0;

    for (i = 1; i < lp->sc; i++)
        for (j = 0; j < i; j++)
            clip_edge(ctx, fp, lp,
                      fp->iv[lp->s0 + i],
                      fp->iv[lp->s0 + j]);

    lp->g0 = fp->ic;
    lp->gc = 0;

    for (i = 0; i < lp->sc; i++)
        if (fp->mv[ctx->plane_m[fp->iv[lp->s0 + i]]].d[3] > 0.0f)
            clip_geom(ctx, fp, lp,
                      fp->iv[lp->s0 + i]);

    for (i = 0; i < lp->sc; i++)
        if (ctx->plane_f[fp->iv[lp->s0 + i]])
            lp->fl |= L_DETAIL;
}

static void clip_file(struct mapc_context *ctx, struct s_base *fp)
{
    int i;

    for (i = 0; i < fp->lc; i++)
        clip_lump(ctx, fp, fp->lv + i);
}

/*---------------------------------------------------------------------------*/

/*
 * For each body element type,  determine if element 'p' is equivalent
 * to element  'q'.  This  is more than  a simple memory  compare.  It
 * effectively  snaps mtrls and  verts together,  and may  reverse the
 * winding of  an edge or a geom.   This is done in  order to maximize
 * the number of elements that can be eliminated.
 */

static int comp_mtrl(const struct b_mtrl *mp, const struct b_mtrl *mq)
{
    if (fabsf(mp->d[0] - mq->d[0]) > SMALL) return 0;
    if (fabsf(mp->d[1] - mq->d[1]) > SMALL) return 0;
    if (fabsf(mp->d[2] - mq->d[2]) > SMALL) return 0;
    if (fabsf(mp->d[3] - mq->d[3]) > SMALL) return 0;

    if (fabsf(mp->a[0] - mq->a[0]) > SMALL) return 0;
    if (fabsf(mp->a[1] - mq->a[1]) > SMALL) return 0;
    if (fabsf(mp->a[2] - mq->a[2]) > SMALL) return 0;
    if (fabsf(mp->a[3] - mq->a[3]) > SMALL) return 0;

    if (fabsf(mp->s[0] - mq->s[0]) > SMALL) return 0;
    if (fabsf(mp->s[1] - mq->s[1]) > SMALL) return 0;
    if (fabsf(mp->s[2] - mq->s[2]) > SMALL) return 0;
    if (fabsf(mp->s[3] - mq->s[3]) > SMALL) return 0;

    if (fabsf(mp->e[0] - mq->e[0]) > SMALL) return 0;
    if (fabsf(mp->e[1] - mq->e[1]) > SMALL) return 0;
    if (fabsf(mp->e[2] - mq->e[2]) > SMALL) return 0;
    if (fabsf(mp->e[3] - mq->e[3]) > SMALL) return 0;

    if (fabsf(mp->h[0] - mq->h[0]) > SMALL) return 0;

    if (strncmp(mp->f, mq->f, PATHMAX)) return 0;

    return 1;
}

static int comp_vert(const struct b_vert *vp, const struct b_vert *vq)
{
    if (fabsf(vp->p[0] - vq->p[0]) > SMALL) return 0;
    if (fabsf(vp->p[1] - vq->p[1]) > SMALL) return 0;
    if (fabsf(vp->p[2] - vq->p[2]) > SMALL) return 0;

    return 1;
}

static int comp_edge(const struct b_edge *ep, const struct b_edge *eq)
{
    if (ep->vi != eq->vi && ep->vi != eq->vj) return 0;
    if (ep->vj != eq->vi && ep->vj != eq->vj) return 0;

    return 1;
}

static int comp_side(const struct b_side *sp, const struct b_side *sq)
{
    if (fabsf(sp->d - sq->d) > SMALL) return 0;
    if (v_dot(sp->n,  sq->n) < 1.0f)  return 0;

    return 1;
}

static int comp_texc(const struct b_texc *tp, const struct b_texc *tq)
{
    if (fabsf(tp->u[0] - tq->u[0]) > SMALL) return 0;
    if (fabsf(tp->u[1] - tq->u[1]) > SMALL) return 0;

    return 1;
}

static int comp_offs(const struct b_offs *op, const struct b_offs *oq)
{
    if (op->ti != oq->ti) return 0;
    if (op->si != oq->si) return 0;
    if (op->vi != oq->vi) return 0;

    return 1;
}

static int comp_geom(const struct b_geom *gp, const struct b_geom *gq)
{
    if (gp->mi != gq->mi) return 0;
    if (gp->oi != gq->oi) return 0;
    if (gp->oj != gq->oj) return 0;
    if (gp->ok != gq->ok) return 0;

    return 1;
}

/*---------------------------------------------------------------------------*/

/*
 * For each file  element type, replace all references  to element 'i'
 * with a  reference to element  'j'.  These are used  when optimizing
 * and sorting the file.
 */

static void swap_mtrl(struct s_base *fp, int mi, int mj)
{
    int i;

    for (i = 0; i < fp->gc; i++)
        if (fp->gv[i].mi == mi) fp->gv[i].mi = mj;
    for (i = 0; i < fp->rc; i++)
        if (fp->rv[i].mi == mi) fp->rv[i].mi = mj;
}

static void swap_vert(struct s_base *fp, int vi, int vj)
{
    int i, j;

    for (i = 0; i < fp->ec; i++)
    {
        if (fp->ev[i].vi == vi) fp->ev[i].vi = vj;
        if (fp->ev[i].vj == vi) fp->ev[i].vj = vj;
    }

    for (i = 0; i < fp->oc; i++)
        if (fp->ov[i].vi == vi) fp->ov[i].vi = vj;

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].vc; j++)
            if (fp->iv[fp->lv[i].v0 + j] == vi)
                fp->iv[fp->lv[i].v0 + j]  = vj;
}

static void apply_mtrl_swaps(struct mapc_context *ctx, struct s_base *fp)
{
    int i;

    for (i = 0; i < fp->gc; i++)
        fp->gv[i].mi = ctx->mtrl_swaps[fp->gv[i].mi];
    for (i = 0; i < fp->rc; i++)
        fp->rv[i].mi = ctx->mtrl_swaps[fp->rv[i].mi];
}


static void apply_vert_swaps(struct mapc_context *ctx, struct s_base *fp)
{
    int i, j;

    for (i = 0; i < fp->ec; i++)
    {
        fp->ev[i].vi = ctx->vert_swaps[fp->ev[i].vi];
        fp->ev[i].vj = ctx->vert_swaps[fp->ev[i].vj];
    }

    for (i = 0; i < fp->oc; i++)
        fp->ov[i].vi = ctx->vert_swaps[fp->ov[i].vi];

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].vc; j++)
            fp->iv[fp->lv[i].v0 + j] = ctx->vert_swaps[fp->iv[fp->lv[i].v0 + j]];
}

static void apply_edge_swaps(struct mapc_context *ctx, struct s_base *fp)
{
    int i, j;

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].ec; j++)
            fp->iv[fp->lv[i].e0 + j] = ctx->edge_swaps[fp->iv[fp->lv[i].e0 + j]];
}

static void apply_side_swaps(struct mapc_context *ctx, struct s_base *fp)
{
    int i, j;

    for (i = 0; i < fp->oc; i++)
        fp->ov[i].si = ctx->side_swaps[fp->ov[i].si];
    for (i = 0; i < fp->nc; i++)
        fp->nv[i].si = ctx->side_swaps[fp->nv[i].si];

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].sc; j++)
            fp->iv[fp->lv[i].s0 + j] = ctx->side_swaps[fp->iv[fp->lv[i].s0 + j]];
}

static void apply_texc_swaps(struct mapc_context *ctx, struct s_base *fp)
{
    int i;

    for (i = 0; i < fp->oc; i++)
        fp->ov[i].ti = ctx->texc_swaps[fp->ov[i].ti];
}

static void apply_offs_swaps(struct mapc_context *ctx, struct s_base *fp)
{
    int i;

    for (i = 0; i < fp->gc; i++)
    {
        fp->gv[i].oi = ctx->offs_swaps[fp->gv[i].oi];
        fp->gv[i].oj = ctx->offs_swaps[fp->gv[i].oj];
        fp->gv[i].ok = ctx->offs_swaps[fp->gv[i].ok];
    }
}

static void apply_geom_swaps(struct mapc_context *ctx, struct s_base *fp)
{
    int i, j;

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].gc; j++)
            fp->iv[fp->lv[i].g0 + j] = ctx->geom_swaps[fp->iv[fp->lv[i].g0 + j]];

    for (i = 0; i < fp->bc; i++)
        for (j = 0; j < fp->bv[i].gc; j++)
            fp->iv[fp->bv[i].g0 + j] = ctx->geom_swaps[fp->iv[fp->bv[i].g0 + j]];
}

/*---------------------------------------------------------------------------*/

static void uniq_mtrl(struct mapc_context *ctx, struct s_base *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->mc; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_mtrl(fp->mv + i, fp->mv + j))
                break;

        ctx->mtrl_swaps[i] = j;

        if (j == k)
        {
            if (i != k)
                fp->mv[k] = fp->mv[i];
            k++;
        }
    }

    apply_mtrl_swaps(ctx, fp);

    fp->mc = k;
}

static void uniq_vert(struct mapc_context *ctx, struct s_base *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->vc; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_vert(fp->vv + i, fp->vv + j))
                break;

        ctx->vert_swaps[i] = j;

        if (j == k)
        {
            if (i != k)
                fp->vv[k] = fp->vv[i];
            k++;
        }
    }

    apply_vert_swaps(ctx, fp);

    fp->vc = k;
}

static void uniq_edge(struct mapc_context *ctx, struct s_base *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->ec; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_edge(fp->ev + i, fp->ev + j))
                break;

        ctx->edge_swaps[i] = j;

        if (j == k)
        {
            if (i != k)
                fp->ev[k] = fp->ev[i];
            k++;
        }
    }

    apply_edge_swaps(ctx, fp);

    fp->ec = k;
}

static void uniq_offs(struct mapc_context *ctx, struct s_base *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->oc; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_offs(fp->ov + i, fp->ov + j))
                break;

        ctx->offs_swaps[i] = j;

        if (j == k)
        {
            if (i != k)
                fp->ov[k] = fp->ov[i];
            k++;
        }
    }

    apply_offs_swaps(ctx,fp);

    fp->oc = k;
}

static void uniq_geom(struct mapc_context *ctx, struct s_base *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->gc; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_geom(fp->gv + i, fp->gv + j))
                break;

        ctx->geom_swaps[i] = j;

        if (j == k)
        {
            if (i != k)
                fp->gv[k] = fp->gv[i];
            k++;
        }
    }

    apply_geom_swaps(ctx,fp);

    fp->gc = k;
}

static void uniq_texc(struct mapc_context *ctx, struct s_base *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->tc; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_texc(fp->tv + i, fp->tv + j))
                break;

        ctx->texc_swaps[i] = j;

        if (j == k)
        {
            if (i != k)
                fp->tv[k] = fp->tv[i];
            k++;
        }
    }

    apply_texc_swaps(ctx, fp);

    fp->tc = k;
}

static void uniq_side(struct mapc_context *ctx, struct s_base *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->sc; i++)
    {
        for (j = 0; j < k; j++)
            if (comp_side(fp->sv + i, fp->sv + j))
                break;

        ctx->side_swaps[i] = j;

        if (j == k)
        {
            if (i != k)
                fp->sv[k] = fp->sv[i];
            k++;
        }
    }

    apply_side_swaps(ctx,fp);

    fp->sc = k;
}

static void uniq_file(struct mapc_context *ctx, struct s_base *fp)
{
    /* Debug mode skips optimization, producing oversized output files. */

    if (ctx->debug_output == 0)
    {
        uniq_mtrl(ctx, fp);
        uniq_vert(ctx, fp);
        uniq_edge(ctx, fp);
        uniq_side(ctx, fp);
        uniq_texc(ctx, fp);
        uniq_offs(ctx, fp);
        uniq_geom(ctx, fp);
    }
}

/*---------------------------------------------------------------------------*/

struct b_trip
{
    int vi;
    int mi;
    int si;
    int gi;
};

static int comp_trip(const void *p, const void *q)
{
    const struct b_trip *tp = (const struct b_trip *) p;
    const struct b_trip *tq = (const struct b_trip *) q;

    if (tp->vi < tq->vi) return -1;
    if (tp->vi > tq->vi) return +1;
    if (tp->mi < tq->mi) return -1;
    if (tp->mi > tq->mi) return +1;

    return 0;
}

static void smth_file(struct mapc_context *ctx, struct s_base *fp)
{
    struct b_trip temp, *T;

    if (ctx->debug_output == 0)
    {
        if ((T = (struct b_trip *) malloc(fp->gc * 3 * sizeof (struct b_trip))))
        {
            int gi, i, j, k, l, c = 0;

            /* Create a list of all non-faceted vertex triplets. */

            for (gi = 0; gi < fp->gc; ++gi)
            {
                struct b_geom *gp = fp->gv + gi;

                T[c].vi = fp->ov[gp->oi].vi;
                T[c].si = fp->ov[gp->oi].si;
                T[c].mi = gp->mi;
                T[c].gi = gi;
                c++;

                T[c].vi = fp->ov[gp->oj].vi;
                T[c].si = fp->ov[gp->oj].si;
                T[c].mi = gp->mi;
                T[c].gi = gi;
                c++;

                T[c].vi = fp->ov[gp->ok].vi;
                T[c].si = fp->ov[gp->ok].si;
                T[c].mi = gp->mi;
                T[c].gi = gi;
                c++;
            }

            /* Sort all triplets by vertex index and material. */

            qsort(T, c, sizeof (struct b_trip), comp_trip);

            /* For each set of triplets sharing vertex index and material... */

            for (i = 0; i < c; i = l)
            {
                int acc = 0;

                float N[3], angle = fp->mv[T[i].mi].angle;
                const float   *Ni = fp->sv[T[i].si].n;

                /* Sort the set by side similarity to the first. */

                for (j = i + 1; j < c && (T[j].vi == T[i].vi &&
                                          T[j].mi == T[i].mi); ++j)
                {
                    for (k = j + 1; k < c && (T[k].vi == T[i].vi &&
                                              T[k].mi == T[i].mi); ++k)
                    {
                        const float *Nj = fp->sv[T[j].si].n;
                        const float *Nk = fp->sv[T[k].si].n;

                        if (T[j].si != T[k].si && v_dot(Nk, Ni) > v_dot(Nj, Ni))
                        {
                            temp = T[k];
                            T[k] = T[j];
                            T[j] = temp;
                        }
                    }
                }

                /* Accumulate all similar side normals. */

                N[0] = Ni[0];
                N[1] = Ni[1];
                N[2] = Ni[2];

                for (l = i + 1; l < c && (T[l].vi == T[i].vi &&
                                          T[l].mi == T[i].mi); ++l)
                    if (T[l].si != T[i].si)
                    {
                        const float *Nl = fp->sv[T[l].si].n;
                        float deg = V_DEG(facosf(v_dot(Ni, Nl)));

                        if (ROUND(deg * 1000.0f) > ROUND(angle * 1000.0f))
                            break;

                        N[0] += Nl[0];
                        N[1] += Nl[1];
                        N[2] += Nl[2];

                        acc++;
                    }

                /* If at least two normals have been accumulated... */

                if (acc)
                {
                    /* Store the accumulated normal as a new side. */

                    int ss = incs(fp);

                    v_nrm(fp->sv[ss].n, N);
                    fp->sv[ss].d = 0.0f;

                    /* Assign the new normal to the merged triplets. */

                    for (j = i; j < l; ++j)
                        T[j].si = ss;
                }
            }

            /* Assign the remapped normals to the original geoms. */

            for (i = 0; i < c; ++i)
            {
                struct b_geom *gp = fp->gv + T[i].gi;
                struct b_offs *op = fp->ov + gp->oi;
                struct b_offs *oq = fp->ov + gp->oj;
                struct b_offs *or = fp->ov + gp->ok;

                if (op->vi == T[i].vi) op->si = T[i].si;
                if (oq->vi == T[i].vi) oq->si = T[i].si;
                if (or->vi == T[i].vi) or->si = T[i].si;
            }

            free(T);
        }

        uniq_side(ctx, fp);
        uniq_offs(ctx, fp);
    }
}

/*---------------------------------------------------------------------------*/

static void sort_file(struct mapc_context *ctx, struct s_base *fp)
{
    int i, j, k;

    /* Sort materials by type to minimize state changes. */

    for (i = 1; i < fp->mc; i++)
        for (j = 0, k = 1; j < fp->mc - i; j++, k++)
            if (fp->mv[j].fl > fp->mv[k].fl)
            {
                struct b_mtrl t;

                t         = fp->mv[j];
                fp->mv[j] = fp->mv[k];
                fp->mv[k] =         t;

                swap_mtrl(fp,  j, -1);
                swap_mtrl(fp,  k,  j);
                swap_mtrl(fp, -1,  k);
            }

    /* Sort billboards by material within distance. */

    for (i = 0; i < fp->rc; i++)
        for (j = i + 1; j < fp->rc; j++)
            if ((fp->rv[j].d  > fp->rv[i].d) ||
                (fp->rv[j].d == fp->rv[i].d &&
                 fp->rv[j].mi > fp->rv[i].mi))
            {
                struct b_bill t;

                t         = fp->rv[i];
                fp->rv[i] = fp->rv[j];
                fp->rv[j] =         t;
            }

    /* Sort items by type and value. */

    for (i = 0; i < fp->hc; i++)
        for (j = i + 1; j < fp->hc; j++)
            if ((fp->hv[j].t > fp->hv[i].t) ||
                (fp->hv[j].t == fp->hv[i].t &&
                 fp->hv[j].n > fp->hv[i].n))
            {
                struct b_item t;

                t         = fp->hv[i];
                fp->hv[i] = fp->hv[j];
                fp->hv[j] =         t;
            }

    /* Sort body lumps by flags. */

    for (i = 0; i < fp->bc; i++)
    {
        const struct b_body *bp = &fp->bv[i];

        int li, lj;

        for (li = bp->l0; li < bp->l0 + bp->lc; ++li)
            for (lj = li + 1; lj < bp->l0 + bp->lc; ++lj)
                if (fp->lv[li].fl > fp->lv[lj].fl)
                {
                    struct b_lump t;

                    t          = fp->lv[li];
                    fp->lv[li] = fp->lv[lj];
                    fp->lv[lj] = t;
                }
    }

    /* Ensure the first vertex is the lowest. */

    for (i = 0; i < fp->vc; i++)
        if (fp->vv[0].p[1] > fp->vv[i].p[1])
        {
            struct b_vert t;

            t         = fp->vv[0];
            fp->vv[0] = fp->vv[i];
            fp->vv[i] =         t;

            swap_vert(fp,  0, -1);
            swap_vert(fp,  i,  0);
            swap_vert(fp, -1,  i);
        }
}

/*---------------------------------------------------------------------------*/

static int test_lump_side(const struct s_base *fp,
                          const struct b_lump *lp,
                          const struct b_side *sp,
                          float bsphere[4])
{
    int si;
    int vi;

    int f = 0;
    int b = 0;

    float d;

    if (!lp->vc)
        return 0;

    /* Check if the bounding sphere of the lump is completely on one side. */

    d = v_dot(bsphere, sp->n) - sp->d;

    if (fabsf(d) > bsphere[3])
        return d > 0 ? 1 : -1;

    /* If the given side is part of the given lump, then the lump is behind. */

    for (si = 0; si < lp->sc; si++)
        if (fp->sv + fp->iv[lp->s0 + si] == sp)
            return -1;

    /* Check if each lump vertex is in front of, behind, on the side. */

    for (vi = 0; vi < lp->vc; vi++)
    {
        d = v_dot(fp->vv[fp->iv[lp->v0 + vi]].p, sp->n) - sp->d;

        if (d > 0) f++;
        if (d < 0) b++;
    }

    /* If no verts are behind, the lump is in front, and vice versa. */

    if (f > 0 && b == 0) return +1;
    if (b > 0 && f == 0) return -1;

    /* Else, the lump crosses the side. */

    return 0;
}

static int node_node(struct mapc_context *ctx, struct s_base *fp, int l0, int lc, float bsphere[][4])
{
    if (lc < 8)
    {
        /* Base case.  Dump all given lumps into a leaf node. */

        fp->nv[fp->nc].si = -1;
        fp->nv[fp->nc].ni = -1;
        fp->nv[fp->nc].nj = -1;
        fp->nv[fp->nc].l0 = l0;
        fp->nv[fp->nc].lc = lc;

        return incn(fp);
    }
    else
    {
        int sj  = 0;
        int sjd = lc;
        int sjo = lc;
        int si;
        int li = 0, lic = 0;
        int lj = 0, ljc = 0;
        int lk = 0, lkc = 0;
        int i;

        /* Find the side that most evenly splits the given lumps. */

        for (si = 0; si < fp->sc; si++)
        {
            int o = 0;
            int d = 0;
            int k = 0;

            for (li = 0; li < lc; li++)
                if ((k = test_lump_side(fp,
                                        fp->lv + l0 + li,
                                        fp->sv + si,
                                        bsphere[l0 + li])))
                    d += k;
                else
                    o++;

            d = abs(d);

            if ((d < sjd) || (d == sjd && o < sjo))
            {
                sj  = si;
                sjd = d;
                sjo = o;
            }
        }

        /* Flag each lump with its position WRT the side. */

        for (li = 0; li < lc; li++)
            if (ctx->debug_output)
            {
                fp->lv[l0+li].fl = (fp->lv[l0+li].fl & 1) | 0x20;
            }
            else
            {
                switch (test_lump_side(fp,
                                       fp->lv + l0 + li,
                                       fp->sv + sj,
                                       bsphere[l0 + li]))
                {
                case +1:
                    fp->lv[l0+li].fl = (fp->lv[l0+li].fl & 1) | 0x10;
                    break;

                case  0:
                    fp->lv[l0+li].fl = (fp->lv[l0+li].fl & 1) | 0x20;
                    break;

                case -1:
                    fp->lv[l0+li].fl = (fp->lv[l0+li].fl & 1) | 0x40;
                    break;
                }
            }

        /* Sort all lumps in the range by their flag values. */

        for (li = 1; li < lc; li++)
            for (lj = 0; lj < li; lj++)
                if (fp->lv[l0 + li].fl < fp->lv[l0 + lj].fl)
                {
                    struct b_lump l;
                    float f;

                    for (i = 0; i < 4; i++)
                    {
                        f                   = bsphere[l0 + li][i];
                        bsphere[l0 + li][i] = bsphere[l0 + lj][i];
                        bsphere[l0 + lj][i] =                   f;
                    }

                    l               = fp->lv[l0 + li];
                    fp->lv[l0 + li] = fp->lv[l0 + lj];
                    fp->lv[l0 + lj] =               l;
                }

        /* Establish the in-front, on, and behind lump ranges. */

        li = lic = 0;
        lj = ljc = 0;
        lk = lkc = 0;

        for (i = lc - 1; i >= 0; i--)
            switch (fp->lv[l0 + i].fl & 0xf0)
            {
            case 0x10: li = l0 + i; lic++; break;
            case 0x20: lj = l0 + i; ljc++; break;
            case 0x40: lk = l0 + i; lkc++; break;
            }

        /* Add the lumps on the side to the node. */

        i = incn(fp);

        fp->nv[i].si = sj;
        fp->nv[i].ni = node_node(ctx, fp, li, lic, bsphere);

        fp->nv[i].nj = node_node(ctx, fp, lk, lkc, bsphere);
        fp->nv[i].l0 = lj;
        fp->nv[i].lc = ljc;

        return i;
    }
}

/*
 * Compute a bounding sphere for a lump (not optimal)
 */
static void lump_bounding_sphere(struct s_base *fp,
                                 struct b_lump *lp,
                                 float bsphere[4])
{
    float bbox[6];
    float r;
    int i;

    if (!lp->vc)
        return;

    bbox[0] = bbox[3] = fp->vv[fp->iv[lp->v0]].p[0];
    bbox[1] = bbox[4] = fp->vv[fp->iv[lp->v0]].p[1];
    bbox[2] = bbox[5] = fp->vv[fp->iv[lp->v0]].p[2];

    for (i = 1; i < lp->vc; i++)
    {
        struct b_vert *vp = fp->vv + fp->iv[lp->v0 + i];
        int j;

        for (j = 0; j < 3; j++)
            if (vp->p[j] < bbox[j])
                bbox[j] = vp->p[j];

        for (j = 0; j < 3; j++)
            if (vp->p[j] > bbox[j + 3])
                bbox[j + 3] = vp->p[j];
    }

    r = 0;

    for (i = 0; i < 3; i++)
    {
        bsphere[i] = (bbox[i] + bbox[i + 3]) / 2;
        r += (bsphere[i] - bbox[i]) * (bsphere[i] - bbox[i]);
    }

    bsphere[3] = fsqrtf(r);
}

static void node_file(struct mapc_context *ctx, struct s_base *fp)
{
    static float bsphere[MAXL][4];
    int i;

    /* Compute a bounding sphere for each lump. */

    for (i = 0; i < fp->lc; i++)
        if (fp->lv[i].fl == 0)
            lump_bounding_sphere(fp, fp->lv + i, bsphere[i]);

    for (i = 0; i < fp->bc; i++)
    {
        int lc;

        /* Count solid lumps. This assumes lumps have been sorted by flags. */

        for (lc = 0; lc < fp->bv[i].lc; lc++)
            if (fp->lv[fp->bv[i].l0 + lc].fl != 0)
                break;

        /* Sort the solid lumps of each body into BSP nodes. */

        fp->bv[i].ni = node_node(ctx, fp, fp->bv[i].l0, lc, bsphere);
    }
}

/*---------------------------------------------------------------------------*/

struct dump_stats
{
    size_t off;
    char name[5];
    char desc[32];
    int *ptr;
};

/*
 * This initializer looked a lot better in the C99 version.
 */
static struct dump_stats stats[] = {
    { offsetof (struct s_base, mc), "mtrl", "materials" },
    { offsetof (struct s_base, vc), "vert", "vertices" },
    { offsetof (struct s_base, ec), "edge", "edges" },
    { offsetof (struct s_base, sc), "side", "sides" },
    { offsetof (struct s_base, tc), "texc", "texcoords" },
    { offsetof (struct s_base, oc), "offs", "offsets" },
    { offsetof (struct s_base, gc), "geom", "geoms" },
    { offsetof (struct s_base, lc), "lump", "lumps" },
    { offsetof (struct s_base, pc), "path", "paths" },
    { offsetof (struct s_base, nc), "node", "nodes" },
    { offsetof (struct s_base, bc), "body", "bodies" },
    { offsetof (struct s_base, hc), "item", "items" },
    { offsetof (struct s_base, zc), "goal", "goals" },
    { offsetof (struct s_base, wc), "view", "viewpoints" },
    { offsetof (struct s_base, jc), "jump", "teleports" },
    { offsetof (struct s_base, xc), "swch", "switches" },
    { offsetof (struct s_base, rc), "bill", "billboards" },
    { offsetof (struct s_base, uc), "ball", "balls" },
    { offsetof (struct s_base, ac), "char", "chars" },
    { offsetof (struct s_base, dc), "dict", "dicts" },
    { offsetof (struct s_base, ic), "indx", "indices" }
};

static void dump_init(struct s_base *fp)
{
    int i;

    for (i = 0; i < ARRAYSIZE(stats); i++)
        stats[i].ptr = (int *) &((unsigned char *) fp)[stats[i].off];
}

static void dump_file(struct mapc_context *ctx, struct s_base *p, const char *name, double t)
{
    int i, j;
    int c = 0;
    int n = 0;

    dump_init(p);

    /* Count the number of solid lumps. */

    for (i = 0; i < p->lc; i++)
        if ((p->lv[i].fl & 1) == 0)
            n++;

    /* Count the total value of all coins. */

    for (i = 0; i < p->hc; i++)
        if (p->hv[i].t == ITEM_COIN)
            c += p->hv[i].n;

#if ENABLE_RADIANT_CONSOLE
    if (ctx->bcast_socket)
    {
        char msg[512];
        char buf[64];

        sprintf(msg, "%s (%d/$%d) %.3f\n", name, n, c, t);

        for (i = 0; i < ARRAYSIZE(stats); i++)
        {
            sprintf(buf, "\t%d %s\n", *stats[i].ptr, stats[i].desc);
            SAFECAT(msg, buf);
        }

        bcast_send_msg(ctx, BCAST_STD, msg);
    }
#endif

    if (ctx->csv_output)
    {
        printf("file,n,c,t,");

        for (i = 0; i < ARRAYSIZE(stats); i++)
            printf("%s%s", stats[i].name, (i + 1 < ARRAYSIZE(stats) ?
                                           "," : "\n"));
        printf("%s,%d,%d,%.3f,", name, n, c, t);

        for (i = 0; i < ARRAYSIZE(stats); i++)
            printf("%d%s", *stats[i].ptr, (i + 1 < ARRAYSIZE(stats) ?
                                           "," : "\n"));
    }
    else
    {
        const int COLS = 11;

        printf("%s (%d/$%d) %.3f\n", name, n, c, t);

        for (i = 0, j = 0; i < ARRAYSIZE(stats); i++)
        {
            printf("%6.6s", stats[i].name);

            if ((i + 1) % COLS == 0 || i + 1 == ARRAYSIZE(stats))
            {
                printf("\n");

                for (; j <= i; j++)
                    printf("%6d", *stats[j].ptr);

                printf("\n");
            }
        }
    }
}

int main(int argc, char *argv[])
{
    char src[MAXSTR] = "";
    char dst[MAXSTR] = "";
    struct s_base f;
    fs_file fin;
    struct mapc_context ctx;

    struct timeval time0;
    struct timeval time1;

    mapc_context_init(&ctx);

    if (!fs_init(argc > 0 ? argv[0] : NULL))
    {
        fprintf(stderr, "Failure to initialize virtual file system: %s\n",
                fs_error());
        return 1;
    }

    if (argc > 2)
    {
        int argi;

        /* Store input file in context */
        ctx.input_file = argv[1];

        for (argi = 3; argi < argc; ++argi)
        {
            if (strcmp(argv[argi], "--debug") == 0)
            {
                ctx.debug_output = 1;
            }
            if (strcmp(argv[argi], "--csv")   == 0)
            {
                ctx.csv_output = 1;
                fs_set_logging(0);
            }
#if ENABLE_RADIANT_CONSOLE
            if (strcmp(argv[argi], "--bcast") == 0) bcast_init(&ctx);
#endif
            if (strcmp(argv[argi], "--data")  == 0)
            {
                if (++argi < argc)
                    fs_add_path(argv[argi]);
            }
        }

        strncpy(src, argv[1], MAXSTR - 1);
        strncpy(dst, argv[1], MAXSTR - 1);

        if (strcmp(dst + strlen(dst) - 4, ".map") == 0)
            strcpy(dst + strlen(dst) - 4, ".sol");
        else
            strcat(dst, ".sol");

        fs_add_path     (dir_name(src));
        fs_set_write_dir(dir_name(dst));

        fs_add_path_with_archives(argv[2]);

        if ((fin = fs_open_read(base_name(src))))
        {
            gettimeofday(&time0, 0);
            {
                init_file(&f);
                read_map(&ctx, &f, fin);

                resolve(&ctx);
                targets(&ctx, &f);

                clip_file(&ctx, &f);
                move_file(&ctx, &f);
                uniq_file(&ctx, &f);
                smth_file(&ctx, &f);
                sort_file(&ctx, &f);
                node_file(&ctx, &f);

                sol_stor_base(&f, base_name(dst));
            }
            gettimeofday(&time1, 0);

            dump_file(&ctx, &f, dst, (time1.tv_sec  - time0.tv_sec) +
                                     (time1.tv_usec - time0.tv_usec) / 1000000.0);

            fs_close(fin);
        }

#if ENABLE_RADIANT_CONSOLE
        bcast_quit(&ctx);
#endif

    }
    else fprintf(stderr, "Usage: %s <map> <data> [--debug] [--csv] [--data <dir>]\n", argv[0]);

    mapc_context_cleanup(&ctx);

    return 0;
}

