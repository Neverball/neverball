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

#include <stdlib.h>

#include "solid_vary.h"
#include "common.h"
#include "vec3.h"

/*---------------------------------------------------------------------------*/

/*
 * Find an existing mover for the given path index or allocate a new one.
 */
static void setup_mover(struct alloc *alloc, const struct s_vary *fp, int pi, int *mi)
{
    struct v_move *move;
    int i;

    if (mi)
        *mi = -1;

    if (pi < 0)
        return;

    for (i = 0; i < fp->mc; ++i)
    {
        if (fp->mv[i].pi == pi)
        {
            if (mi)
                *mi = i;

            return;
        }
    }

    if ((move = alloc_add(alloc)))
    {
        memset(move, 0, sizeof (*move));

        if (mi)
            *mi = fp->mc - 1;

        move->pi = pi;
    }
}

int sol_load_vary(struct s_vary *fp, struct s_base *base)
{
    struct alloc mover_alloc;
    int i;

    memset(fp, 0, sizeof (*fp));

    fp->base = base;

    alloc_new(&mover_alloc, sizeof (*fp->mv), (void **) &fp->mv, &fp->mc);

    if (fp->base->pc)
    {
        fp->pv = calloc(fp->base->pc, sizeof (*fp->pv));
        fp->pc = fp->base->pc;

        for (i = 0; i < fp->base->pc; i++)
        {
            struct v_path *pp = fp->pv + i;
            struct b_path *pq = fp->base->pv + i;

            pp->base = pq;
            pp->f    = pq->f;
        }
    }

    if (fp->base->bc)
    {
        fp->bv = calloc(fp->base->bc, sizeof (*fp->bv));
        fp->bc = fp->base->bc;

        for (i = 0; i < fp->base->bc; i++)
        {
            struct v_body *bp = fp->bv + i;
            struct b_body *bq = fp->base->bv + i;

            bp->base = bq;

            setup_mover(&mover_alloc, fp, bq->p0, &bp->mi);
            setup_mover(&mover_alloc, fp, bq->p1, &bp->mj);
        }
    }

    if (fp->base->hc)
    {
        fp->hv = calloc(fp->base->hc, sizeof (*fp->hv));
        fp->hc = fp->base->hc;

        for (i = 0; i < fp->base->hc; i++)
        {
            struct v_item *hp = fp->hv + i;
            struct b_item *hq = fp->base->hv + i;

            v_cpy(hp->p, hq->p);

            hp->t = hq->t;
            hp->n = hq->n;

            setup_mover(&mover_alloc, fp, hq->p0, &hp->mi);
            setup_mover(&mover_alloc, fp, hq->p1, &hp->mj);
        }
    }

    if (fp->base->zc)
    {
        fp->zv = calloc(fp->base->zc, sizeof (*fp->zv));
        fp->zc = fp->base->zc;

        for (i = 0; i < fp->base->zc; i++)
        {
            struct v_goal *zp = fp->zv + i;
            struct b_goal *zq = fp->base->zv + i;

            setup_mover(&mover_alloc, fp, zq->p0, &zp->mi);
            setup_mover(&mover_alloc, fp, zq->p1, &zp->mj);
        }
    }

    if (fp->base->jc)
    {
        fp->jv = calloc(fp->base->jc, sizeof (*fp->jv));
        fp->jc = fp->base->jc;

        for (i = 0; i < fp->base->jc; i++)
        {
            struct v_jump *jp = fp->jv + i;
            struct b_jump *jq = fp->base->jv + i;

            setup_mover(&mover_alloc, fp, jq->p0, &jp->mi);
            setup_mover(&mover_alloc, fp, jq->p1, &jp->mj);
        }
    }

    if (fp->base->xc)
    {
        fp->xv = calloc(fp->base->xc, sizeof (*fp->xv));
        fp->xc = fp->base->xc;

        for (i = 0; i < fp->base->xc; i++)
        {
            struct v_swch *xp = fp->xv + i;
            struct b_swch *xq = fp->base->xv + i;

            xp->base = xq;
            xp->t    = xq->t;
            xp->tm   = xq->tm;
            xp->f    = xq->f;

            setup_mover(&mover_alloc, fp, xq->p0, &xp->mi);
            setup_mover(&mover_alloc, fp, xq->p1, &xp->mj);
        }
    }

    if (fp->base->rc)
    {
        fp->rv = calloc(fp->base->rc, sizeof (*fp->rv));
        fp->rc = fp->base->rc;

        for (i = 0; i < fp->base->rc; i++)
        {
            struct v_bill *rp = fp->rv + i;
            struct b_bill *rq = fp->base->rv + i;

            setup_mover(&mover_alloc, fp, rq->p0, &rp->mi);
            setup_mover(&mover_alloc, fp, rq->p1, &rp->mj);
        }
    }

    if (fp->base->uc)
    {
        fp->uv = calloc(fp->base->uc, sizeof (*fp->uv));
        fp->uc = fp->base->uc;

        for (i = 0; i < fp->base->uc; i++)
        {
            struct v_ball *up = fp->uv + i;
            struct b_ball *uq = fp->base->uv + i;

            v_cpy(up->p, uq->p);

            up->r = uq->r;
            up->r_vel = 0.0f;

            up->sizes[0] = uq->r * GROW_SMALL;
            up->sizes[1] = uq->r;
            up->sizes[2] = uq->r * GROW_BIG;

            up->size = 1;

            up->E[0][0] = up->e[0][0] = 1.0f;
            up->E[0][1] = up->e[0][1] = 0.0f;
            up->E[0][2] = up->e[0][2] = 0.0f;

            up->E[1][0] = up->e[1][0] = 0.0f;
            up->E[1][1] = up->e[1][1] = 1.0f;
            up->E[1][2] = up->e[1][2] = 0.0f;

            up->E[2][0] = up->e[2][0] = 0.0f;
            up->E[2][1] = up->e[2][1] = 0.0f;
            up->E[2][2] = up->e[2][2] = 1.0f;
        }
    }

    return 1;
}

void sol_free_vary(struct s_vary *fp)
{
    free(fp->pv);
    free(fp->bv);
    free(fp->mv);
    free(fp->hv);
    free(fp->xv);
    free(fp->zv);
    free(fp->uv);

    memset(fp, 0, sizeof (*fp));
}

/*---------------------------------------------------------------------------*/

int sol_vary_cmd(struct s_vary *fp, struct cmd_state *cs, const union cmd *cmd)
{
    struct v_ball *up;
    int rc = 0;

    switch (cmd->type)
    {
    case CMD_MAKE_BALL:
        if ((up = realloc(fp->uv, sizeof (*up) * (fp->uc + 1))))
        {
            fp->uv = up;
            cs->curr_ball = fp->uc;
            fp->uc++;
            rc = 1;
        }
        break;

    case CMD_CLEAR_BALLS:
        free(fp->uv);
        fp->uv = NULL;
        fp->uc = 0;
        break;

    default:
        break;
    }

    return rc;
}

/*---------------------------------------------------------------------------*/

#define CURR 0
#define PREV 1

int sol_lerp_cmd(struct s_lerp *fp, struct cmd_state *cs, const union cmd *cmd)
{
    struct l_ball (*uv)[2];
    struct l_ball *up;

    int idx, mi, i;
    int rc = 0;

    switch (cmd->type)
    {
    case CMD_MAKE_BALL:
        if ((uv = realloc(fp->uv, sizeof (*uv) * (fp->uc + 1))))
        {
            fp->uv = uv;

            /* Update varying state. */

            if (sol_vary_cmd(fp->vary, cs, cmd))
            {
                fp->uc++;
                rc = 1;
            }
        }
        break;

    case CMD_MOVE_PATH:
        if ((mi = cmd->movepath.mi) >= 0 && mi < fp->mc)
        {
            /* Be extra paranoid. */

            if ((idx = cmd->movepath.pi) >= 0 && idx < fp->vary->base->pc)
                fp->mv[mi][CURR].pi = idx;
        }
        break;

    case CMD_MOVE_TIME:
        if ((mi = cmd->movetime.mi) >= 0 && mi < fp->mc)
        {
            fp->mv[mi][CURR].t = cmd->movetime.t;
        }
        break;

    case CMD_BODY_PATH:
        /* Backward compatibility: update linear mover only. */

        if ((idx = cmd->bodypath.bi) >= 0 && idx < fp->vary->bc &&
            (mi = fp->vary->bv[idx].mi) >= 0)
        {
            /* Be EXTRA paranoid. */

            if ((idx = cmd->bodypath.pi) >= 0 && idx < fp->vary->base->pc)
                fp->mv[mi][CURR].pi = idx;
        }
        break;

    case CMD_BODY_TIME:
        /* Same as CMD_BODY_PATH. */

        if ((idx = cmd->bodytime.bi) >= 0 && idx < fp->vary->bc &&
            (mi = fp->vary->bv[idx].mi) >= 0)
        {
            fp->mv[mi][CURR].t = cmd->bodytime.t;
        }
        break;

    case CMD_BALL_RADIUS:
        fp->uv[cs->curr_ball][CURR].r = cmd->ballradius.r;
        break;

    case CMD_CLEAR_BALLS:
        free(fp->uv);
        fp->uv = NULL;
        fp->uc = 0;

        sol_vary_cmd(fp->vary, cs, cmd);

        break;

    case CMD_BALL_POSITION:
        up = &fp->uv[cs->curr_ball][CURR];
        v_cpy(up->p, cmd->ballpos.p);
        break;

    case CMD_BALL_BASIS:
        up = &fp->uv[cs->curr_ball][CURR];
        v_cpy(up->e[0], cmd->ballbasis.e[0]);
        v_cpy(up->e[1], cmd->ballbasis.e[1]);
        v_crs(up->e[2], up->e[0], up->e[1]);
        break;

    case CMD_BALL_PEND_BASIS:
        up = &fp->uv[cs->curr_ball][CURR];
        v_cpy(up->E[0], cmd->ballpendbasis.E[0]);
        v_cpy(up->E[1], cmd->ballpendbasis.E[1]);
        v_crs(up->E[2], up->E[0], up->E[1]);
        break;

    case CMD_STEP_SIMULATION:
        /*
         * Step each mover ahead. This  way we cut down on replay size
         * significantly  while  still  keeping  things in  sync  with
         * occasional CMD_MOVE_PATH and CMD_MOVE_TIME.
         */

        for (i = 0; i < fp->mc; i++)
        {
            struct l_move *mp = &fp->mv[i][CURR];

            if (mp->pi >= 0 && fp->vary->pv[mp->pi].f)
                mp->t += cmd->stepsim.dt;
        }
        break;

    default:
        break;
    }

    return rc;
}

void sol_lerp_copy(struct s_lerp *fp)
{
    int i;

    for (i = 0; i < fp->mc; i++)
        fp->mv[i][PREV] = fp->mv[i][CURR];

    for (i = 0; i < fp->uc; i++)
        fp->uv[i][PREV] = fp->uv[i][CURR];
}

void sol_lerp_apply(struct s_lerp *fp, float a)
{
    int i;

    for (i = 0; i < fp->mc; i++)
    {
        if (fp->mv[i][PREV].pi == fp->mv[i][CURR].pi)
            fp->vary->mv[i].t = flerp(fp->mv[i][PREV].t, fp->mv[i][CURR].t, a);
        else
            fp->vary->mv[i].t = fp->mv[i][CURR].t * a;

        fp->vary->mv[i].pi = fp->mv[i][CURR].pi;
    }

    for (i = 0; i < fp->uc; i++)
    {
        e_lerp(fp->vary->uv[i].e, fp->uv[i][PREV].e, fp->uv[i][CURR].e, a);
        v_lerp(fp->vary->uv[i].p, fp->uv[i][PREV].p, fp->uv[i][CURR].p, a);
        e_lerp(fp->vary->uv[i].E, fp->uv[i][PREV].E, fp->uv[i][CURR].E, a);

        fp->vary->uv[i].r = flerp(fp->uv[i][PREV].r, fp->uv[i][CURR].r, a);
    }
}

int sol_load_lerp(struct s_lerp *fp, struct s_vary *vary)
{
    int i;

    fp->vary = vary;

    if (fp->vary->mc)
    {
        fp->mv = calloc(fp->vary->mc, sizeof (*fp->mv));
        fp->mc = fp->vary->mc;

        for (i = 0; i < fp->vary->mc; i++)
            fp->mv[i][CURR].pi = fp->vary->mv[i].pi;
    }

    if (fp->vary->uc)
    {
        fp->uv = calloc(fp->vary->uc, sizeof (*fp->uv));
        fp->uc = fp->vary->uc;

        for (i = 0; i < fp->vary->uc; i++)
        {
            e_cpy(fp->uv[i][CURR].e, fp->vary->uv[i].e);
            v_cpy(fp->uv[i][CURR].p, fp->vary->uv[i].p);
            e_cpy(fp->uv[i][CURR].E, fp->vary->uv[i].E);

            fp->uv[i][CURR].r = fp->vary->uv[i].r;
        }
    }

    sol_lerp_copy(fp);

    return 1;
}

void sol_free_lerp(struct s_lerp *fp)
{
    if (fp->mv) free(fp->mv);
    if (fp->uv) free(fp->uv);

    memset(fp, 0, sizeof (*fp));
}

/*---------------------------------------------------------------------------*/
