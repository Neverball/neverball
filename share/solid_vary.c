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
        fp->bv = calloc(fp->base->bc, sizeof (*fp->bv));
        fp->bc = fp->base->bc;

        for (i = 0; i < fp->base->bc; i++)
        {
            struct v_body *bp = fp->bv + i;
            struct b_body *bq = fp->base->bv + i;

            bp->base = bq;
            bp->pi   = bq->pi;
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
    if (fp->pv) free(fp->pv);
    if (fp->bv) free(fp->bv);
    if (fp->hv) free(fp->hv);
    if (fp->xv) free(fp->xv);
    if (fp->uv) free(fp->uv);

    memset(fp, 0, sizeof (*fp));
}

/*---------------------------------------------------------------------------*/

#define CURR 0
#define PREV 1

static int curr_ball;

int sol_lerp_cmd(struct s_lerp *fp, const union cmd *cmd)
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

                curr_ball = fp->uc - 1;
                rc = 1;
            }
        }
        break;

    case CMD_BODY_PATH:
        fp->bv[cmd->bodypath.bi][CURR].pi = cmd->bodypath.pi;
        break;

    case CMD_BODY_TIME:
        fp->bv[cmd->bodytime.bi][CURR].t = cmd->bodytime.t;
        break;

    case CMD_BALL_RADIUS:
        fp->uv[curr_ball][CURR].r = cmd->ballradius.r;
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
        up = &fp->uv[curr_ball][CURR];
        v_cpy(up->p, cmd->ballpos.p);
        break;

    case CMD_BALL_BASIS:
        up = &fp->uv[curr_ball][CURR];
        v_cpy(up->e[0], cmd->ballbasis.e[0]);
        v_cpy(up->e[1], cmd->ballbasis.e[1]);
        v_crs(up->e[2], up->e[0], up->e[1]);
        break;

    case CMD_BALL_PEND_BASIS:
        up = &fp->uv[curr_ball][CURR];
        v_cpy(up->E[0], cmd->ballpendbasis.E[0]);
        v_cpy(up->E[1], cmd->ballpendbasis.E[1]);
        v_crs(up->E[2], up->E[0], up->E[1]);
        break;

    case CMD_CURRENT_BALL:
        curr_ball = cmd->currball.ui;
        break;

    case CMD_STEP_SIMULATION:
        /*
         * Simulate body motion.
         *
         * This is done on the client side due to replay file size
         * concerns and isn't done as part of CMD_END_OF_UPDATE to
         * match the server state as closely as possible.  Body time
         * is still synchronized with the server on a semi-regular
         * basis and path indices are handled through CMD_BODY_PATH,
         * thus this code doesn't need to be as sophisticated as
         * sol_body_step.
         */

        for (i = 0; i < fp->bc; i++)
        {
            struct l_body *bp = &fp->bv[i][CURR];
            struct v_path *pp = &fp->vary->pv[bp->pi];

            if (bp->pi >= 0 && pp->f)
                bp->t += cmd->stepsim.dt;
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

    for (i = 0; i < fp->bc; i++)
        fp->bv[i][PREV] = fp->bv[i][CURR];

    for (i = 0; i < fp->uc; i++)
        fp->uv[i][PREV] = fp->uv[i][CURR];
}

void sol_lerp_apply(struct s_lerp *fp, float a)
{
    int i;

    for (i = 0; i < fp->bc; i++)
    {
        if (fp->bv[i][PREV].pi == fp->bv[i][CURR].pi)
            fp->vary->bv[i].t = (fp->bv[i][PREV].t * (1.0f - a) +
                                 fp->bv[i][CURR].t * a);
        else
            fp->vary->bv[i].t = fp->bv[i][CURR].t * a;

        fp->vary->bv[i].pi = fp->bv[i][CURR].pi;
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

    if (fp->vary->bc)
    {
        fp->bv = calloc(fp->vary->bc, sizeof (*fp->bv));
        fp->bc = fp->vary->bc;

        for (i = 0; i < fp->vary->bc; i++)
            fp->bv[i][CURR].pi = fp->vary->bv[i].pi;
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
    if (fp->bv) free(fp->bv);
    if (fp->uv) free(fp->uv);

    memset(fp, 0, sizeof (*fp));
}

/*---------------------------------------------------------------------------*/
