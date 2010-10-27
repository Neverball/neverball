/*
 * Copyright (C) 2003-2010 Neverball authors
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

#include "game_common.h"
#include "vec3.h"
#include "config.h"
#include "solid.h"

/*---------------------------------------------------------------------------*/

const char *status_to_str(int s)
{
    switch (s)
    {
    case GAME_NONE:    return _("Aborted");
    case GAME_TIME:    return _("Time-out");
    case GAME_GOAL:    return _("Success");
    case GAME_FALL:    return _("Fall-out");
    default:           return _("Unknown");
    }
}

const char *view_to_str(int v)
{
    switch (v)
    {
    case VIEW_CHASE:   return _("Chase");
    case VIEW_LAZY:    return _("Lazy");
    case VIEW_MANUAL:  return _("Manual");
    default:           return _("Unknown");
    }
}

/*---------------------------------------------------------------------------*/

const float GRAVITY_UP[] = { 0.0f, +9.8f, 0.0f };
const float GRAVITY_DN[] = { 0.0f, -9.8f, 0.0f };

void game_tilt_init(struct game_tilt *tilt)
{
    tilt->x[0] = 1.0f;
    tilt->x[1] = 0.0f;
    tilt->x[2] = 0.0f;

    tilt->rx = 0.0f;

    tilt->z[0] = 0.0f;
    tilt->z[1] = 0.0f;
    tilt->z[2] = 1.0f;

    tilt->rz = 0.0f;
}

/*
 * Compute appropriate tilt axes from the view basis.
 */
void game_tilt_axes(struct game_tilt *tilt, float view_e[3][3])
{
    v_cpy(tilt->x, view_e[0]);
    v_cpy(tilt->z, view_e[2]);
}

void game_tilt_grav(float h[3], const float g[3], const struct game_tilt *tilt)
{
    float X[16];
    float Z[16];
    float M[16];

    /* Compute the gravity vector from the given world rotations. */

    m_rot (Z, tilt->z, V_RAD(tilt->rz));
    m_rot (X, tilt->x, V_RAD(tilt->rx));
    m_mult(M, Z, X);
    m_vxfm(h, M, g);
}

/*---------------------------------------------------------------------------*/

void game_view_init(struct game_view *view)
{
    view->dp  = config_get_d(CONFIG_VIEW_DP) / 100.0f;
    view->dc  = config_get_d(CONFIG_VIEW_DC) / 100.0f;
    view->dz  = config_get_d(CONFIG_VIEW_DZ) / 100.0f;
    view->a   = 0.0f;

    view->c[0] = 0.0f;
    view->c[1] = view->dc;
    view->c[2] = 0.0f;

    view->p[0] =     0.0f;
    view->p[1] = view->dp;
    view->p[2] = view->dz;

    view->e[0][0] = 1.0f;
    view->e[0][1] = 0.0f;
    view->e[0][2] = 0.0f;
    view->e[1][0] = 0.0f;
    view->e[1][1] = 1.0f;
    view->e[1][2] = 0.0f;
    view->e[2][0] = 0.0f;
    view->e[2][1] = 0.0f;
    view->e[2][2] = 1.0f;
}

void game_view_fly(struct game_view *view, const s_file *fp, float k)
{
    /* float  x[3] = { 1.f, 0.f, 0.f }; */
    float  y[3] = { 0.f, 1.f, 0.f };
    float  z[3] = { 0.f, 0.f, 1.f };
    float c0[3] = { 0.f, 0.f, 0.f };
    float p0[3] = { 0.f, 0.f, 0.f };
    float c1[3] = { 0.f, 0.f, 0.f };
    float p1[3] = { 0.f, 0.f, 0.f };
    float  v[3];

    game_view_init(view);

    /* k = 0.0 view is at the ball. */

    if (fp->uc > 0)
    {
        v_cpy(c0, fp->uv[0].p);
        v_cpy(p0, fp->uv[0].p);
    }

    v_mad(p0, p0, y, view->dp);
    v_mad(p0, p0, z, view->dz);
    v_mad(c0, c0, y, view->dc);

    /* k = +1.0 view is s_view 0 */

    if (k >= 0 && fp->wc > 0)
    {
        v_cpy(p1, fp->wv[0].p);
        v_cpy(c1, fp->wv[0].q);
    }

    /* k = -1.0 view is s_view 1 */

    if (k <= 0 && fp->wc > 1)
    {
        v_cpy(p1, fp->wv[1].p);
        v_cpy(c1, fp->wv[1].q);
    }

    /* Interpolate the views. */

    v_sub(v, p1, p0);
    v_mad(view->p, p0, v, k * k);

    v_sub(v, c1, c0);
    v_mad(view->c, c0, v, k * k);

    /* Orthonormalize the view basis. */

    v_sub(view->e[2], view->p, view->c);
    v_crs(view->e[0], view->e[1], view->e[2]);
    v_crs(view->e[2], view->e[0], view->e[1]);
    v_nrm(view->e[0], view->e[0]);
    v_nrm(view->e[2], view->e[2]);
}

/*---------------------------------------------------------------------------*/

void lockstep_clr(struct lockstep *ls)
{
    ls->at = 0;
}

void lockstep_run(struct lockstep *ls, float dt)
{
    ls->at += dt;

    while (ls->at >= ls->dt)
    {
        ls->step(ls->dt);
        ls->at -= ls->dt;
    }
}

/*---------------------------------------------------------------------------*/
