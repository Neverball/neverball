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

#include "vec3.h"
#include "glext.h"
#include "ball.h"
#include "part.h"
#include "geom.h"
#include "config.h"
#include "video.h"
#include "mtrl.h"

#include "solid_draw.h"

#include "game_draw.h"

/*---------------------------------------------------------------------------*/

static void game_draw_balls(struct s_rend *rend,
                            const struct s_vary *vary,
                            const float *bill_M, float t)
{
    float c[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    float ball_M[16];
    float pend_M[16];

    m_basis(ball_M, vary->uv[0].e[0], vary->uv[0].e[1], vary->uv[0].e[2]);
    m_basis(pend_M, vary->uv[0].E[0], vary->uv[0].E[1], vary->uv[0].E[2]);

    ptransformer->push();
    ptransformer->flags &= ~M_SHADOWED;
    ptransformer->exflags |= M_REFLECTIVE;
    ptransformer->translate(vary->uv[0].p[0], vary->uv[0].p[1] + BALL_FUDGE, vary->uv[0].p[2]);
    ptransformer->scale(vary->uv[0].r, vary->uv[0].r, vary->uv[0].r);
    ptransformer->voffsetAccum = 0.0f;
    ptransformer->colormod = pgl::floatv4(c[0], c[1], c[2], c[3]);

    currentIntersector = intersectorBall;
    //currentIntersector = intersector;
    ball_draw(rend, ball_M, pend_M, bill_M, t);
    currentIntersector = intersector;

    ptransformer->exflags &= ~M_REFLECTIVE;
    ptransformer->colormod = pgl::floatv4(1.0f);
    ptransformer->pop();
}

static void game_draw_items(struct s_rend *rend,
                            const struct s_vary *vary,
                            const float *bill_M, float t)
{
    for (int hi = 0; hi < vary->hc; hi++)
    {
        struct v_item *hp = &vary->hv[hi];
        if (hp->t == ITEM_NONE) {
            continue;
        }
        
        ptransformer->push();
        ptransformer->translate(hp->p[0], hp->p[1], hp->p[2]);
        ptransformer->flags |= M_REFLECTIVE | M_SHADOWED; //Reflective
        item_draw(rend, hp, bill_M, t);
        ptransformer->flags &= ~(M_REFLECTIVE | M_SHADOWED);
        ptransformer->pop();
    }
}

static void game_draw_beams(struct s_rend *rend, const struct game_draw *gd)
{
    static const GLfloat goal_c[4]       =   { 1.0f, 1.0f, 0.0f, 0.5f };
    static const GLfloat jump_c[2][4]    =  {{ 0.7f, 0.5f, 1.0f, 0.5f },
                                             { 0.7f, 0.5f, 1.0f, 0.8f }};
    static const GLfloat swch_c[2][2][4] = {{{ 1.0f, 0.0f, 0.0f, 0.5f },
                                             { 1.0f, 0.0f, 0.0f, 0.8f }},
                                            {{ 0.0f, 1.0f, 0.0f, 0.5f },
                                             { 0.0f, 1.0f, 0.0f, 0.8f }}};

    const struct s_base *base =  gd->vary.base;
    const struct s_vary *vary = &gd->vary;

    ptransformer->exflags |= M_SHADOWED;

    /* Goal beams */
    if (gd->goal_e) {
        for (int i = 0; i < base->zc; i++) {
            beam_draw(rend, base->zv[i].p, goal_c, base->zv[i].r, gd->goal_k * 3.0f);
        }
    }

    /* Jump beams */
    for (int i = 0; i < base->jc; i++) {
        beam_draw(rend, base->jv[i].p, jump_c[gd->jump_e ? 0 : 1], base->jv[i].r, 2.0f);
    }

    /* Switch beams */
    for (int i = 0; i < base->xc; i++) {
        if (!vary->xv[i].base->i) {
            beam_draw(rend, base->xv[i].p, swch_c[vary->xv[i].f][vary->xv[i].e], base->xv[i].r, 2.0f);
        }
    }

    ptransformer->exflags &= ~M_SHADOWED;
}

static void game_draw_goals(struct s_rend *rend,
                            const struct game_draw *gd, float t)
{
    ptransformer->exflags |= M_SHADOWED;

    const struct s_base *base = gd->vary.base;
    if (gd->goal_e) {
        for (int i = 0; i < base->zc; i++) {
            goal_draw(rend, base->zv[i].p, base->zv[i].r, gd->goal_k, t);
        }
    }

    ptransformer->exflags &= ~M_SHADOWED;
}

static void game_draw_jumps(struct s_rend *rend,
                            const struct game_draw *gd, float t)
{
    ptransformer->exflags |= M_SHADOWED;

    const struct s_base *base = gd->vary.base;
    for (int i = 0; i < base->jc; i++) {
        jump_draw(rend, base->jv[i].p, base->jv[i].r, 1.0f);
    }

    ptransformer->exflags &= ~M_SHADOWED;
}

/*---------------------------------------------------------------------------*/

static void game_draw_tilt(const struct game_draw *gd, int d)
{
    const struct game_tilt *tilt = &gd->tilt;
    const float *ball_p = gd->vary.uv[0].p;
    ptransformer->translate(+ball_p[0], +ball_p[1] * d, +ball_p[2]);
    ptransformer->rotate(-tilt->rz * d, tilt->z[0], tilt->z[1], tilt->z[2]);
    ptransformer->rotate(-tilt->rx * d, tilt->x[0], tilt->x[1], tilt->x[2]);
    ptransformer->translate(-ball_p[0], -ball_p[1] * d, -ball_p[2]);
}

/*---------------------------------------------------------------------------*/

static void game_draw_light(const struct game_draw *gd, int d, float t)
{
    GLfloat p[4];
    light_conf();
    p[0] = cosf(t);
    p[1] = 0.0f;
    p[2] = sinf(t);
    p[3] = 0.0f;
}

static void game_draw_back(struct s_rend *rend,
                           const struct game_draw *gd,
                           int pose, int d, float t)
{
    ptransformer->exflags |= M_SHADOWED | M_REFLECTIVE;
    ptransformer->push();
    if (d < 0)
    {
        const struct game_tilt *tilt = &gd->tilt;
        ptransformer->rotate(tilt->rz * 2, tilt->z[0], tilt->z[1], tilt->z[2]);
        ptransformer->rotate(tilt->rx * 2, tilt->x[0], tilt->x[1], tilt->x[2]);
    }

    const struct game_view *view = &gd->view;
    if (pose == POSE_BALL) {
        ptransformer->multiply(glm::inverse(glm::lookAt(*(pgl::floatv3 *)view->p, *(pgl::floatv3 *)view->c, pgl::floatv3(0.0f, 1.0f, 0.0f))));
    }
    else {
        ptransformer->translate(view->p[0], view->p[1] * d, view->p[2]);
    }

    ptransformer->voffsetAccum = 0.0f;

    currentIntersector = intersectorBack;
    back_draw(rend);
    if (config_get_d(CONFIG_BACKGROUND))
    {
        currentIntersector = intersectorBillboard;
        sol_back(&gd->back.draw, rend, 0, FAR_DIST, t);
    }
    currentIntersector = intersector;

    ptransformer->pop();
    ptransformer->exflags &= ~(M_SHADOWED | M_REFLECTIVE);
    
    
}

static void game_clip_ball(const struct game_draw *gd, int d, const float *p)
{
    GLfloat r, c[3], pz[4], nz[4];

    c[0] = p[0];
    c[1] = p[1] * d;
    c[2] = p[2];

    pz[0] = gd->view.p[0] - c[0];
    pz[1] = gd->view.p[1] - c[1];
    pz[2] = gd->view.p[2] - c[2];

    r = sqrt(pz[0] * pz[0] + pz[1] * pz[1] + pz[2] * pz[2]);

    pz[0] /= r;
    pz[1] /= r;
    pz[2] /= r;
    pz[3] = -(pz[0] * c[0] +
              pz[1] * c[1] +
              pz[2] * c[2]);

    nz[0] = -pz[0];
    nz[1] = -pz[1];
    nz[2] = -pz[2];
    nz[3] = -pz[3];

    pz[1] *= d;
    nz[1] *= d;
}

static void game_draw_fore(struct s_rend *rend,
                           struct game_draw *gd,
                           int pose, const float *M,
                           int d, float t)
{
    const float *ball_p = gd->vary.uv[0].p;

    struct s_draw *draw = &gd->draw;

    currentIntersector = intersector;
    ptransformer->push();
    {
        game_draw_tilt(gd, d);
        game_clip_ball(gd, d, ball_p);

        currentIntersector = intersector;

        switch (pose)
        {
        case POSE_LEVEL:
            sol_draw(draw, rend, 0, 1);
            break;

        case POSE_BALL:
            game_draw_balls(rend, draw->vary, M, t);
            break;

        case POSE_NONE:
            sol_draw(draw, rend, 0, 1);
            game_draw_items(rend, draw->vary, M, t);
            game_draw_balls(rend, draw->vary, M, t);
            break;
        }

        ptransformer->flags |= M_SHADOWED;
        sol_bill(draw, rend, M, t);
        ptransformer->flags &= ~M_SHADOWED;

        game_draw_beams(rend, gd);
        game_draw_goals(rend, gd, t);
        game_draw_jumps(rend, gd, t);
    }
    ptransformer->pop();
}

static void game_raytrace(const pgl::floatv3 eye, const pgl::floatv3 target, const glm::mat4 persp) {
    pmaterials->loadToVGA();

    ptracer->enableShadows(config_get_d(CONFIG_SHADOW));
    ptracer->enableReflections(config_get_d(CONFIG_REFLECTION));

    ptracer->camera(eye, target, persp);
    for (int j = 0;j < 12;j++) {
        if (ptracer->getRayCount() <= 0) break;
        ptracer->resetHits();
        ptracer->intersection(intersectorBillboard);
        ptracer->intersection(intersectorBack);
        ptracer->intersection(intersector);
        ptracer->intersection(intersectorBall);

        ptracer->missing();
        if (ptracer->getRayCount() <= 0) break;
        ptracer->shade(pmaterials);
        ptracer->reclaim();
        if (ptracer->getRayCount() <= 0) break;
    }

    ptracer->clearSampler();
    ptracer->sample();
    ptracer->render();
}

static void game_shadow_conf(int pose, int enable)
{
    if (enable && config_get_d(CONFIG_SHADOW))
    {
        switch (pose)
        {
        case POSE_LEVEL:
            /* No shadow. */
            //tex_env_active(&tex_env_default);
            break;

        case POSE_BALL:
            /* Shadow only. */
            //tex_env_select(&tex_env_pose, &tex_env_default, NULL);
            break;

        default:
            /* Regular shadow. */
            //tex_env_select(&tex_env_shadow_clip, &tex_env_shadow, &tex_env_default, NULL);
            break;
        }
    }
    else
    {
        //tex_env_active(&tex_env_default);
    }
}


/*---------------------------------------------------------------------------*/

void game_draw(struct game_draw *gd, int pose, float t)
{
    float fov = (float) config_get_d(CONFIG_VIEW_FOV);
    if (gd->jump_b) fov *= 2.f * fabsf(gd->jump_dt - 0.5f);

    if (gd->state)
    {
        const struct game_view *view = &gd->view;
        struct s_rend rend;

        gd->draw.shadow_ui = 0;
        game_shadow_conf(pose, 1);
        r_draw_enable(&rend);

        pgl::floatv3 eye = pgl::floatv3(0.0f);
        pgl::floatv3 target = pgl::floatv3(0.0f, 0.0f, -1.0f);
        pgl::floatv3 up = pgl::floatv3(0.0f, 1.0f, 0.0f);
        glm::mat4 persp = glm::perspective(fov / 180.f * ((pgl::floatv)M_PI * 1.0f), (pgl::floatv)video.window_w / (pgl::floatv)video.window_h, 0.01f, 1000.0f);

        intersector->clearTribuffer();
        intersectorBillboard->clearTribuffer();
        intersectorBall->clearTribuffer();
        intersectorBack->clearTribuffer();
        currentIntersector = intersector;

        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glPushMatrix();

        pmaterials->submats.resize(0);
        ptransformer->colormod = pgl::floatv4(1.0f);
        ptransformer->voffsetAccum = 0.0f;
        ptransformer->reset();
        ptransformer->push();
        ptransformer->flags = 0;
        ptransformer->exflags = 0;
        {
            float T[16], U[16], M[16], v[3];
            v[0] = +view->p[0];
            v[1] = -view->p[1];
            v[2] = +view->p[2];

            video_calc_view(T, view->c, view->p, view->e[1]);
            video_calc_view(U, view->c, v, view->e[1]);

            m_xps(M, T);
            v_sub(v, view->c, view->p);

            game_draw_back(&rend, gd, pose, +1, t);
            game_draw_fore(&rend, gd, pose, T, +1, t);
        }
        ptransformer->pop();

        glPopMatrix();

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        eye = *(pgl::floatv3 *)view->p;
        target = *(pgl::floatv3 *)view->c;
        game_raytrace(eye, target, persp);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glCullFace(GL_BACK);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        video_push_persp((float)config_get_d(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
        {
            part_draw_coin(&rend, glm::lookAt(eye, target, pgl::floatv3(0.0f, 1.0f, 0.0f)) * ptransformer->getCurrent());
        }
        video_pop_matrix();

        sol_fade_gl(&gd->draw, &rend, gd->fade_k);
        r_draw_disable(&rend);
        game_shadow_conf(pose, 0);
    }
}

/*---------------------------------------------------------------------------*/

#define CURR 0
#define PREV 1

void game_lerp_init(struct game_lerp *gl, struct game_draw *gd)
{
    gl->alpha = 1.0f;

    sol_load_lerp(&gl->lerp, &gd->vary);

    gl->tilt[PREV] = gl->tilt[CURR] = gd->tilt;
    gl->view[PREV] = gl->view[CURR] = gd->view;

    gl->goal_k[PREV] = gl->goal_k[CURR] = gd->goal_k;
    gl->jump_dt[PREV] = gl->jump_dt[CURR] = gd->jump_dt;
}

void game_lerp_free(struct game_lerp *gl)
{
    sol_free_lerp(&gl->lerp);
}

void game_lerp_copy(struct game_lerp *gl)
{
    sol_lerp_copy(&gl->lerp);

    gl->tilt[PREV] = gl->tilt[CURR];
    gl->view[PREV] = gl->view[CURR];

    gl->goal_k[PREV] = gl->goal_k[CURR];
    gl->jump_dt[PREV] = gl->jump_dt[CURR];
}

void game_lerp_apply(struct game_lerp *gl, struct game_draw *gd)
{
    float a = gl->alpha;

    /* Solid. */

    sol_lerp_apply(&gl->lerp, a);

    /* Particles. */

    part_lerp_apply(a);

    /* Tilt. */

    v_lerp(gd->tilt.x, gl->tilt[PREV].x, gl->tilt[CURR].x, a);
    v_lerp(gd->tilt.z, gl->tilt[PREV].z, gl->tilt[CURR].z, a);

    gd->tilt.rx = flerp(gl->tilt[PREV].rx, gl->tilt[CURR].rx, a);
    gd->tilt.rz = flerp(gl->tilt[PREV].rz, gl->tilt[CURR].rz, a);

    /* View. */

    v_lerp(gd->view.c, gl->view[PREV].c, gl->view[CURR].c, a);
    v_lerp(gd->view.p, gl->view[PREV].p, gl->view[CURR].p, a);
    e_lerp(gd->view.e, gl->view[PREV].e, gl->view[CURR].e, a);

    /* Effects. */

    gd->goal_k = flerp(gl->goal_k[PREV], gl->goal_k[CURR], a);
    gd->jump_dt = flerp(gl->jump_dt[PREV], gl->jump_dt[CURR], a);
}

/*---------------------------------------------------------------------------*/
