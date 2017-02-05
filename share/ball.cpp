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
#include <string.h>

#include "vec3.h"
#include "glext.h"
#include "config.h"
#include "common.h"

#include "solid_draw.h"
#include "solid_sim.h"
#include "video.h"

/*---------------------------------------------------------------------------*/

static int has_solid = 0;
static int has_inner = 0;
static int has_outer = 0;

static struct s_full solid;
static struct s_full inner;
static struct s_full outer;

#define F_PENDULUM   1
#define F_DRAWBACK   2
#define F_DRAWCLIP   4
#define F_DEPTHMASK  8
#define F_DEPTHTEST 16

static int solid_flags;
static int inner_flags;
static int outer_flags;

/*---------------------------------------------------------------------------*/

#define SET(B, v, b) ((v) ? ((B) | (b)) : ((B) & ~(b)))

static int ball_opts(const struct s_base *base)
{
    int flags = F_DEPTHTEST;
    int di;

    for (di = 0; di < base->dc; ++di)
    {
        char *k = base->av + base->dv[di].ai;
        char *v = base->av + base->dv[di].aj;

        if (strcmp(k, "pendulum")  == 0)
            flags = SET(flags, atoi(v), F_PENDULUM);
        if (strcmp(k, "drawback")  == 0)
            flags = SET(flags, atoi(v), F_DRAWBACK);
        if (strcmp(k, "drawclip")  == 0)
            flags = SET(flags, atoi(v), F_DRAWCLIP);
        if (strcmp(k, "depthmask") == 0)
            flags = SET(flags, atoi(v), F_DEPTHMASK);
        if (strcmp(k, "depthtest") == 0)
            flags = SET(flags, atoi(v), F_DEPTHTEST);
    }

    return flags;
}

void ball_init(void)
{
    char *solid_file = concat_string(config_get_s(CONFIG_BALL_FILE),
                                     "-solid.sol", NULL);
    char *inner_file = concat_string(config_get_s(CONFIG_BALL_FILE),
                                     "-inner.sol", NULL);
    char *outer_file = concat_string(config_get_s(CONFIG_BALL_FILE),
                                     "-outer.sol", NULL);

    solid_flags = 0;
    inner_flags = 0;
    outer_flags = 0;

    if ((has_solid = sol_load_full(&solid, solid_file, 0)))
        solid_flags = ball_opts(&solid.base);

    if ((has_inner = sol_load_full(&inner, inner_file, 0)))
        inner_flags = ball_opts(&inner.base);

    if ((has_outer = sol_load_full(&outer, outer_file, 0)))
        outer_flags = ball_opts(&outer.base);

    free(solid_file);
    free(inner_file);
    free(outer_file);
}

void ball_free(void)
{
    if (has_outer) sol_free_full(&outer);
    if (has_inner) sol_free_full(&inner);
    if (has_solid) sol_free_full(&solid);

    has_solid = has_inner = has_outer = 0;
}

void ball_step(float dt)
{
    if (has_solid) sol_move(&solid.vary, NULL, dt);
    if (has_inner) sol_move(&inner.vary, NULL, dt);
    if (has_outer) sol_move(&outer.vary, NULL, dt);
}

/*---------------------------------------------------------------------------*/

static void ball_draw_solid(struct s_rend *rend,
                            const float *ball_M,
                            const float *ball_bill_M, float t)
{
    if (has_solid)
    {
        const int mask = (solid_flags & F_DEPTHMASK);
        const int test = (solid_flags & F_DEPTHTEST);

        //glPushMatrix();
        //{
        //    glMultMatrixf(ball_M);

            ptransformer->push();
            ptransformer->multiply(ball_M);
            if (solid.base.rc)
            {
                sol_bill(&solid.draw, rend, ball_bill_M, t);
            }
            sol_draw(&solid.draw, rend, mask, test);
            ptransformer->pop();
        //}
        //glPopMatrix();
    }
}

static void ball_draw_inner(struct s_rend *rend,
                            const float *pend_M,
                            const float *bill_M,
                            const float *pend_bill_M, float t)
{
    if (has_inner)
    {
        const int pend = (inner_flags & F_PENDULUM);
        const int mask = (inner_flags & F_DEPTHMASK);
        const int test = (inner_flags & F_DEPTHTEST);

        if (pend)
        {
            //glPushMatrix();
            //glMultMatrixf(pend_M);
            ptransformer->push();
            ptransformer->multiply(pend_M);
        }

        sol_draw(&inner.draw, rend, mask, test);

        if (inner.base.rc)
        {
            {
                if (pend)
                    sol_bill(&inner.draw, rend, pend_bill_M, t);
                else
                    sol_bill(&inner.draw, rend, bill_M,      t);
            }
        }

        if (pend) {
            ptransformer->pop();
            //glPopMatrix();
        }
    }
}

static void ball_draw_outer(struct s_rend *rend,
                            const float *pend_M,
                            const float *bill_M,
                            const float *pend_bill_M, float t)
{
    if (has_outer)
    {
        const int pend = (outer_flags & F_PENDULUM);
        const int mask = (outer_flags & F_DEPTHMASK);
        const int test = (outer_flags & F_DEPTHTEST);

        if (pend)
        {
            ptransformer->push();
            ptransformer->multiply(pend_M);
            //glPushMatrix();
            //glMultMatrixf(pend_M);
        }

        sol_draw(&outer.draw, rend, mask, test);

        if (outer.base.rc)
        {
            {
                if (pend)
                    sol_bill(&outer.draw, rend, pend_bill_M, t);
                else
                    sol_bill(&outer.draw, rend, bill_M,      t);
            }
        }

        if (pend) {
            ptransformer->pop();
            //glPopMatrix();
        }
    }
}

/*---------------------------------------------------------------------------*/

static void ball_pass_inner(struct s_rend *rend,
                            const float *ball_M,
                            const float *pend_M,
                            const float *bill_M,
                            const float *ball_bill_M,
                            const float *pend_bill_M, float t)
{
    ball_draw_inner(rend, pend_M, bill_M, pend_bill_M, t);
}

static void ball_pass_solid(struct s_rend *rend,
                            const float *ball_M,
                            const float *pend_M,
                            const float *bill_M,
                            const float *ball_bill_M,
                            const float *pend_bill_M, float t)
{
    ball_pass_inner(rend, ball_M, pend_M, bill_M, ball_bill_M, pend_bill_M, t);
    ball_draw_solid(rend, ball_M, ball_bill_M, t);
}

static void ball_pass_outer(struct s_rend *rend,
                            const float *ball_M,
                            const float *pend_M,
                            const float *bill_M,
                            const float *ball_bill_M,
                            const float *pend_bill_M, float t)
{
    ball_pass_solid(rend, ball_M, pend_M, bill_M, ball_bill_M, pend_bill_M, t);
    ball_draw_outer(rend, pend_M, bill_M, pend_bill_M, t);
}

/*---------------------------------------------------------------------------*/

void ball_draw(struct s_rend *rend,
               const float *ball_M,
               const float *pend_M,
               const float *bill_M, float t)
{
    float ball_T[16], ball_bill_M[16];
    float pend_T[16], pend_bill_M[16];

    m_xps(ball_T, ball_M);
    m_xps(pend_T, pend_M);

    m_mult(ball_bill_M, ball_T, bill_M);
    m_mult(pend_bill_M, pend_T, bill_M);

    ball_pass_outer(rend, ball_M, pend_M, bill_M, ball_bill_M, pend_bill_M, t);
}

/*---------------------------------------------------------------------------*/
