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
