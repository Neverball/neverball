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

#ifndef SOLID_VARY_H
#define SOLID_VARY_H

#include "base_config.h"
#include "solid_base.h"

/*
 * Varying solid data.
 */

/*---------------------------------------------------------------------------*/

struct v_path
{
    const struct b_path *base;

    int f;                                     /* enable flag                */
};

struct v_body
{
    const struct b_body *base;

    int mi;
    int mj;
};

struct v_move
{
    float t;                                   /* time on current path       */
    int   tm;                                  /* milliseconds               */

    int pi;
};

struct v_item
{
    float p[3];                                /* position                   */
    int   t;                                   /* type                       */
    int   n;                                   /* value                      */
};

struct v_swch
{
    const struct b_swch *base;

    float t;                                   /* current timer              */
    int   tm;                                  /* milliseconds               */
    int   f;                                   /* current state              */
    int   e;                                   /* is a ball inside it?       */
};

struct v_ball
{
    float e[3][3];                             /* basis of orientation       */
    float p[3];                                /* position vector            */
    float v[3];                                /* velocity vector            */
    float w[3];                                /* angular velocity vector    */
    float E[3][3];                             /* basis of pendulum          */
    float W[3];                                /* angular pendulum velocity  */
    float r;                                   /* radius                     */
};

struct v_chkp // New: Checkpoints
{
    const struct b_chkp *base;

    int   f;                                   /* current state              */
    int   e;                                   /* is a ball inside it?       */
};

struct s_vary
{
    struct s_base *base;

    int pc;
    int bc;
    int mc;
    int hc;
    int xc;
    int uc;
    int cc;

    struct v_path *pv;
    struct v_body *bv;
    struct v_move *mv;
    struct v_item *hv;
    struct v_swch *xv;
    struct v_ball *uv;
    struct v_chkp *cv;

    /* Accumulator for tracking time in integer milliseconds. */

    float ms_accum;
};

/*---------------------------------------------------------------------------*/

int  sol_load_vary(struct s_vary *, struct s_base *);
void sol_free_vary(struct s_vary *);

/*---------------------------------------------------------------------------*/

/*
 * Buffers changes to the varying SOL data for interpolation purposes.
 */

struct l_move
{
    float t;                                   /* time on current path       */

    int pi;
};

struct l_ball
{
    float e[3][3];                             /* basis of orientation       */
    float p[3];                                /* position vector            */
    float E[3][3];                             /* basis of pendulum          */
    float r;                                   /* radius                     */
};

struct s_lerp
{
    struct s_vary *vary;

    int mc;
    int uc;

    struct l_move (*mv)[2];
    struct l_ball (*uv)[2];
};

/*---------------------------------------------------------------------------*/

#include "cmd.h"

int  sol_load_lerp(struct s_lerp *, struct s_vary *);
void sol_free_lerp(struct s_lerp *);

void sol_lerp_copy(struct s_lerp *);
void sol_lerp_apply(struct s_lerp *, float);
int  sol_lerp_cmd(struct s_lerp *, struct cmd_state *, const union cmd *);

/*---------------------------------------------------------------------------*/

#endif
