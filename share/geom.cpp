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

#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <stdarg.h>

#include "glext.h"
#include "geom.h"
#include "ball.h"
#include "part.h"
#include "vec3.h"
#include "image.h"
#include "config.h"
#include "video.h"
#include "hmd.h"
//#include "mtrl.h"

#include "solid_draw.h"
#include "solid_sim.h"

/*---------------------------------------------------------------------------*/

const struct tex_env *curr_tex_env;

static void tex_env_conf_default(int, int);
static void tex_env_conf_shadow(int, int);
static void tex_env_conf_pose(int, int);

const struct tex_env tex_env_default = {
    tex_env_conf_default,
    1,
    {
        { GL_TEXTURE0, TEX_STAGE_TEXTURE }
    }
};

const struct tex_env tex_env_shadow = {
    tex_env_conf_shadow,
    2,
    {
        { GL_TEXTURE0, TEX_STAGE_SHADOW },
        { GL_TEXTURE1, TEX_STAGE_TEXTURE }
    }
};

const struct tex_env tex_env_shadow_clip = {
    tex_env_conf_shadow,
    3,
    {
        { GL_TEXTURE0, TEX_STAGE_SHADOW },
        { GL_TEXTURE1, TEX_STAGE_CLIP },
        { GL_TEXTURE2, TEX_STAGE_TEXTURE }
    }
};

const struct tex_env tex_env_pose = {
    tex_env_conf_pose,
    2,
    {
        { GL_TEXTURE0, TEX_STAGE_SHADOW },
        { GL_TEXTURE1, TEX_STAGE_TEXTURE }
    }
};

static void tex_env_conf_default(int stage, int enable)
{
    switch (stage)
    {

    }
}

static void tex_env_conf_shadow(int stage, int enable)
{
}

static void tex_env_conf_pose(int stage, int enable)
{
}

static void tex_env_conf(const struct tex_env *env, int enable)
{
    size_t i;

    for (i = 0; i < env->count; i++)
    {
        const struct tex_stage *st = &env->stages[i];
        glActiveTexture_(st->unit);
        glClientActiveTexture_(st->unit);
        env->conf(st->stage, enable);
    }

    /* Last stage remains selected. */
}

/*
 * Set up current texture pipeline.
 */
void tex_env_active(const struct tex_env *env)
{
    if (curr_tex_env == env)
        return;

    if (curr_tex_env)
    {
        tex_env_conf(curr_tex_env, 0);
        curr_tex_env = NULL;
    }

    tex_env_conf(env, 1);
    curr_tex_env = env;
}

/*
 * Select an appropriate texture pipeline out of several.
 */
void tex_env_select(const struct tex_env *first, ...)
{
    const struct tex_env *sel = &tex_env_default;
    const struct tex_env *env;

    va_list ap;

    va_start(ap, first);

    for (env = first; env; env = va_arg(ap, const struct tex_env *))
        if (env->count <= gli.max_texture_units && env->count >= sel->count)
            sel = env;

    va_end(ap);

    tex_env_active(sel);
}

/*
 * Select stage of the current texture pipeline.
 */
int tex_env_stage(int stage)
{
    size_t i;

    if (curr_tex_env)
    {
        for (i = 0; i < curr_tex_env->count; i++)
        {
            const struct tex_stage *st = &curr_tex_env->stages[i];

            if (st->stage == stage)
            {
                glActiveTexture_(st->unit);
                glClientActiveTexture_(st->unit);
                return 1;
            }
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

enum
{
    GEOM_NONE = -1,

    GEOM_COIN,
    GEOM_COIN5,
    GEOM_COIN10,
    GEOM_GROW,
    GEOM_SHRINK,

    GEOM_MAX
};

static const char item_sols[GEOM_MAX][PATHMAX] = {
    "item/coin/coin.sol",
    "item/coin/coin5.sol",
    "item/coin/coin10.sol",
    "item/grow/grow.sol",
    "item/shrink/shrink.sol"
};

static struct s_full beam;
static struct s_full jump;
static struct s_full goal;
static struct s_full flag;
static struct s_full mark;
static struct s_full vect;
static struct s_full back;
static struct s_full item[GEOM_MAX];

static int back_state = 0;

/*---------------------------------------------------------------------------*/

void geom_init(void)
{
    sol_load_full(&beam, "geom/beam/beam.sol", 0);
    sol_load_full(&jump, "geom/jump/jump.sol", 0);
    sol_load_full(&goal, "geom/goal/goal.sol", 0);
    sol_load_full(&flag, "geom/flag/flag.sol", 0);
    sol_load_full(&mark, "geom/mark/mark.sol", 0);
    sol_load_full(&vect, "geom/vect/vect.sol", 0);

    for (int i = 0; i < GEOM_MAX; i++) {
        sol_load_full(&item[i], item_sols[i], 0);
    }
}

void geom_free(void)
{
    sol_free_full(&vect);
    sol_free_full(&mark);
    sol_free_full(&flag);
    sol_free_full(&goal);
    sol_free_full(&jump);
    sol_free_full(&beam);

    for (int i = 0; i < GEOM_MAX; i++) {
        sol_free_full(&item[i]);
    }
}

void geom_step(float dt)
{
    sol_move(&goal.vary, NULL, dt);
    sol_move(&jump.vary, NULL, dt);

    for (int i = 0; i < GEOM_MAX; i++) {
        sol_move(&item[i].vary, NULL, dt);
    }

    ball_step(dt);
}

/*---------------------------------------------------------------------------*/

static struct s_draw *item_file(const struct v_item *hp)
{
    int g = GEOM_COIN;

    if (hp)
    {
        switch (hp->t)
        {
        case ITEM_GROW:   g = GEOM_GROW;   break;
        case ITEM_SHRINK: g = GEOM_SHRINK; break;
        default:
            if      (hp->n >= 10) g = GEOM_COIN10;
            else if (hp->n >= 5)  g = GEOM_COIN5;
            else                  g = GEOM_COIN;
            break;
        }
    }

    return &item[g].draw;
}

void item_color(const struct v_item *hp, float *c)
{
    const struct s_draw *draw = item_file(hp);

    c[0] = 1.0f;
    c[1] = 1.0f;
    c[2] = 1.0f;
    c[3] = 1.0f;

    if (draw && draw->base && draw->base->mtrls)
    {
        struct mtrl *mp = mtrl_get(draw->base->mtrls[0]);

        if (mp)
        {
            c[0] = mp->base.d[0];
            c[1] = mp->base.d[1];
            c[2] = mp->base.d[2];
            c[3] = mp->base.d[3];
        }
    }
}

void item_draw(struct s_rend *rend,
               const struct v_item *hp,
               const GLfloat *M, float t)
{
    const GLfloat s = ITEM_RADIUS;

    struct s_draw *draw = item_file(hp);

    ptransformer->push();
    ptransformer->scale(s, s, s);

    sol_bill(draw, rend, M, t);
    sol_draw(draw, rend, 0, 1);

    ptransformer->pop();
}

/*---------------------------------------------------------------------------*/

void back_init(const char *name)
{
    back_free();

    /* Load the background SOL and modify its material in-place to use the   */
    /* named gradient texture.                                               */

    if (sol_load_full(&back, "geom/back/back.sol", 0)) {
        struct mtrl *mp = mtrl_get(back.base.mtrls[0]);
        mp->po = make_image_from_file_pgl(name, IF_MIPMAP);
        mp->base.d[0] = 1.0f;
        mp->base.d[1] = 1.0f;
        mp->base.d[2] = 1.0f;
        mp->base.d[3] = 1.0f;
        //mp->base.fl |= M_TRANSPARENT;

        if (mp->po) {
            mp->po->wrap<0>(pgl::TextureWrap::Repeat);
        }

        back_state = 1;
    }
}

void back_free(void)
{
    if (back_state) {
        sol_free_full(&back);
    }
    back_state = 0;
}

/*---------------------------------------------------------------------------*/

/* Draw a column of light with position p, color c, radius r, and height h. */

void beam_draw(struct s_rend *rend, const GLfloat *p,
                                    const GLfloat *c, GLfloat r, GLfloat h)
{
    ptransformer->push();
    ptransformer->translate(p[0], p[1], p[2]);
    ptransformer->scale(r, h, r);
    ptransformer->colormod = pgl::floatv4(c[0], c[1], c[2], c[3]);
    ptransformer->flags |= M_TRANSPARENT;
    sol_draw(&beam.draw, rend, 1, 1);
    ptransformer->flags = 0;
    ptransformer->colormod = pgl::floatv4(1.0f);
    ptransformer->pop();
}

void goal_draw(struct s_rend *rend, const GLfloat *p, GLfloat r, GLfloat h, GLfloat t)
{
    GLfloat height = (hmd_stat() ? 0.3f : 1.0f) * video.device_h;

    glPointSize(height / 6);

    ptransformer->push();
    ptransformer->translate(p[0], p[1], p[2]);
    ptransformer->scale(r, h, r);

    sol_draw(&goal.draw, rend, 1, 1);

    ptransformer->pop();
}

void jump_draw(struct s_rend *rend, const GLfloat *p, GLfloat r, GLfloat h)
{
    GLfloat height = (hmd_stat() ? 0.3f : 1.0f) * video.device_h;

    glPointSize(height / 12);

    ptransformer->push();
    ptransformer->translate(p[0], p[1], p[2]);
    ptransformer->scale(r, h, r);

    sol_draw(&jump.draw, rend, 1, 1);

    ptransformer->pop();
}

void flag_draw(struct s_rend *rend, const GLfloat *p)
{
    ptransformer->push();
    ptransformer->translate(p[0], p[1], p[2]);
    ptransformer->colormod = pgl::floatv4(1.0f);
    sol_draw(&flag.draw, rend, 1, 1);
    ptransformer->pop();
}

void mark_draw(struct s_rend *rend)
{
    sol_draw(&mark.draw, rend, 1, 1);
}

void vect_draw(struct s_rend *rend)
{
    sol_draw(&vect.draw, rend, 0, 1);
    sol_draw(&vect.draw, rend, 0, 0);
}

void back_draw(struct s_rend *rend)
{
    ptransformer->push();
    ptransformer->scale(-BACK_DIST, BACK_DIST, -BACK_DIST);
    ptransformer->flags |= M_TRANSPARENT;
    sol_draw(&back.draw, rend, 1, 1);
    ptransformer->flags = 0;
    ptransformer->pop();
}

void back_draw_easy(void)
{
    struct s_rend rend;

    r_draw_enable(&rend);
    back_draw(&rend);
    r_draw_disable(&rend);
}

/*---------------------------------------------------------------------------*/

/*
 * A note about lighting and shadow: technically speaking, it's wrong.
 * The  light  position  and   shadow  projection  behave  as  if  the
 * light-source rotates with the  floor.  However, the skybox does not
 * rotate, thus the light should also remain stationary.
 *
 * The  correct behavior  would eliminate  a significant  3D  cue: the
 * shadow of  the ball indicates  the ball's position relative  to the
 * floor even  when the ball is  in the air.  This  was the motivating
 * idea  behind the  shadow  in  the first  place,  so correct  shadow
 * projection would only magnify the problem.
 */

static GLuint shad_text;
static GLuint clip_text;

static GLubyte clip_data[] = { 0xff, 0xff, 0x0, 0x0 };

void shad_init(void)
{
}

void shad_free(void)
{
}

void shad_draw_set(void)
{
}

void shad_draw_clr(void)
{
}

/*---------------------------------------------------------------------------*/

/*
 * Configurable lights.
 */

#define LIGHT_MAX 3

struct light
{
    GLfloat p[4];
    GLfloat d[4];
    GLfloat a[4];
    GLfloat s[4];
};

static const GLfloat default_ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

static const struct light default_lights[LIGHT_MAX] = {
    {
        { -8.0f, +32.0f, -8.0f, 0.0f },

        { 1.0f, 0.8f, 0.8f, 1.0f },
        { 0.7f, 0.7f, 0.7f, 1.0f },
        { 1.0f, 0.8f, 0.8f, 1.0f }
    },
    {
        { +8.0f, +32.0f, +8.0f, 0.0f },

        { 0.8f, 1.0f, 0.8f, 1.0f },
        { 0.7f, 0.7f, 0.7f, 1.0f },
        { 0.8f, 1.0f, 0.8f, 1.0f },
    },
    {
        { 0.0f, 0.0f, 1.0f, 0.0f },

        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
    }
};

static GLfloat      light_ambient[4];
static struct light lights[LIGHT_MAX];

void light_reset(void)
{
    memcpy(lights,        default_lights,  sizeof (lights));
    memcpy(light_ambient, default_ambient, sizeof (light_ambient));
}

void light_conf(void)
{
}

void light_load(void)
{
    static char buf[MAXSTR];

    int light = -1;

    fs_file fp;
    float v[4];
    int i;

    light_reset();

    if ((fp = fs_open("lights.txt", "r")))
    {
        while (fs_gets(buf, sizeof (buf), fp))
        {
            strip_newline(buf);

            if      (sscanf(buf, "light %d", &i) == 1)
            {
                if (i >= 0 && i < LIGHT_MAX)
                    light = i;
            }
            else if (sscanf(buf, "position %f %f %f %f",
                            &v[0], &v[1], &v[2], &v[3]) == 4)
            {
                if (light >= 0)
                    q_cpy(lights[light].p, v);
            }
            else if (sscanf(buf, "diffuse %f %f %f %f",
                            &v[0], &v[1], &v[2], &v[3]) == 4)
            {
                if (light >= 0)
                    q_cpy(lights[light].d, v);
            }
            else if (sscanf(buf, "ambient %f %f %f %f",
                            &v[0], &v[1], &v[2], &v[3]) == 4)
            {
                if (light >= 0)
                    q_cpy(lights[light].a, v);
                else
                    q_cpy(light_ambient, v);
            }
            else if (sscanf(buf, "specular %f %f %f %f",
                            &v[0], &v[1], &v[2], &v[3]) == 4)
            {
                if (light >= 0)
                    q_cpy(lights[light].s, v);
            }
        }
        fs_close(fp);
    }
}

/*---------------------------------------------------------------------------*/