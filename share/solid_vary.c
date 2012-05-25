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

int sol_load_vary(struct s_vary *fp, const struct s_base *base)
{
    int i;

    memset(fp, 0, sizeof (*fp));

    fp->base = base;

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
        struct alloc mv;

        fp->bv = calloc(fp->base->bc, sizeof (*fp->bv));
        fp->bc = fp->base->bc;

        alloc_new(&mv, sizeof (*fp->mv), (void **) &fp->mv, &fp->mc);

        for (i = 0; i < fp->base->bc; i++)
        {
            struct b_body *bbody = fp->base->bv + i;
            struct v_body *vbody = fp->bv + i;
            struct v_move *vmove;

            vbody->base = bbody;

            vbody->mi = -1;
            vbody->mj = -1;

            if (bbody->pi >= 0 && (vmove = alloc_add(&mv)))
            {
                memset(vmove, 0, sizeof (*vmove));

                vbody->mi = fp->mc - 1;
                vmove->pi = bbody->pi;
            }

            if (bbody->pj == bbody->pi)
            {
                vbody->mj = vbody->mi;
            }
            else if (bbody->pj >= 0 && (vmove = alloc_add(&mv)))
            {
                memset(vmove, 0, sizeof (*vmove));

                vbody->mj = fp->mc - 1;
                vmove->pi = bbody->pj;
            }
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
    free(fp->uv);

    memset(fp, 0, sizeof (*fp));
}

/*---------------------------------------------------------------------------*/

#define CURR 0
#define PREV 1

int sol_lerp_cmd(struct s_lerp *fp, struct cmd_state *cs, const union cmd *cmd)
{
    struct l_ball (*uv)[2];
    struct l_ball *up;

    int i, rc = 0;

    switch (cmd->type)
    {
    case CMD_MAKE_BALL:
        if ((uv = realloc(fp->uv, sizeof (*uv) * (fp->uc + 1))))
        {
            struct v_ball *up;

            fp->uv = uv;
            fp->uc++;

            /* Sync the main structure. */

            if ((up = realloc(fp->vary->uv, sizeof (*up) * fp->uc)))
            {
                fp->vary->uv = up;
                fp->vary->uc = fp->uc;

                cs->curr_ball = fp->uc - 1;
                rc = 1;
            }
        }
        break;

    case CMD_BODY_PATH:
    case CMD_BODY_TIME:
        /* Backward compatibility: update linear mover only. */

        if (cmd->type == CMD_BODY_PATH)
        {
            int mi;

            if ((mi = fp->vary->bv[cmd->bodypath.bi].mi) >= 0)
                fp->mv[mi][CURR].pi = cmd->bodypath.pi;
        }
        if (cmd->type == CMD_BODY_TIME)
        {
            int mi;

            if ((mi = fp->vary->bv[cmd->bodytime.bi].mi) >= 0)
                fp->mv[mi][CURR].t = cmd->bodytime.t;
        }
        break;

    case CMD_MOVE_PATH:
        fp->mv[cmd->movepath.mi][CURR].pi = cmd->movepath.pi;
        break;

    case CMD_MOVE_TIME:
        fp->mv[cmd->movepath.mi][CURR].t = cmd->movetime.t;
        break;

    case CMD_BALL_RADIUS:
        fp->uv[cs->curr_ball][CURR].r = cmd->ballradius.r;
        break;

    case CMD_CLEAR_BALLS:
        free(fp->uv);
        fp->uv = NULL;
        fp->uc = 0;

        free(fp->vary->uv);
        fp->vary->uv = NULL;
        fp->vary->uc = 0;
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
            fp->vary->mv[i].t = (fp->mv[i][PREV].t * (1.0f - a) +
                                 fp->mv[i][CURR].t * a);
        else
            fp->vary->mv[i].t = fp->mv[i][CURR].t * a;

        fp->vary->mv[i].pi = fp->mv[i][CURR].pi;
    }

    for (i = 0; i < fp->uc; i++)
    {
        e_lerp(fp->vary->uv[i].e, fp->uv[i][PREV].e, fp->uv[i][CURR].e, a);
        v_lerp(fp->vary->uv[i].p, fp->uv[i][PREV].p, fp->uv[i][CURR].p, a);
        e_lerp(fp->vary->uv[i].E, fp->uv[i][PREV].E, fp->uv[i][CURR].E, a);

        fp->vary->uv[i].r = (fp->uv[i][PREV].r * (1.0f - a) +
                             fp->uv[i][CURR].r * a);
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
