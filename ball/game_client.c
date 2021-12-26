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
#include <math.h>
#include <assert.h>

#include "glext.h"
#include "vec3.h"
#include "geom.h"
#include "part.h"
#include "ball.h"
#include "image.h"
#include "audio.h"
#include "config.h"
#include "video.h"

#include "solid_draw.h"

#include "game_client.h"
#include "game_common.h"
#include "game_proxy.h"
#include "game_draw.h"

#include "cmd.h"

/*---------------------------------------------------------------------------*/

int game_compat_map;                    /* Client/server map compat flag     */

/*---------------------------------------------------------------------------*/

#define CURR 0
#define PREV 1

static struct game_draw gd;
static struct game_lerp gl;

static float timer  = 0.0f;             /* Clock time                        */
static int   gained = 0;                /* Time increased mid-level          */
static int   status = GAME_NONE;        /* Outcome of the game               */
static int   coins  = 0;                /* Collected coins                   */

static struct cmd_state cs;             /* Command state                     */

struct
{
    int x, y;
} version;                              /* Current map version               */

/*---------------------------------------------------------------------------*/

static void game_run_cmd(const union cmd *cmd)
{
    if (gd.state)
    {
        struct game_view *view = &gl.view[CURR];
        struct game_tilt *tilt = &gl.tilt[CURR];

        struct s_vary *vary = &gd.vary;
        struct v_item *hp;

        float v[4];
        float dt;

        int idx;

        if (cs.next_update)
        {
            game_lerp_copy(&gl);
            cs.next_update = 0;
        }

        switch (cmd->type)
        {
        case CMD_END_OF_UPDATE:
            cs.got_tilt_axes = 0;
            cs.next_update = 1;

            if (cs.first_update)
            {
                game_lerp_copy(&gl);
                /* Hack to sync state before the next update. */
                game_lerp_apply(&gl, &gd);
                cs.first_update = 0;
                break;
            }

            /* Compute gravity for particle effects. */

            if (status == GAME_GOAL)
                game_tilt_grav(v, GRAVITY_UP, tilt);
            else
                game_tilt_grav(v, GRAVITY_DN, tilt);

            /* Step particle, goal and jump effects. */

            if (cs.ups > 0)
            {
                dt = 1.0f / cs.ups;

                if (gd.goal_e && gl.goal_k[CURR] < 1.0f)
                    gl.goal_k[CURR] += dt;

                if (gd.jump_b)
                {
                    gl.jump_dt[CURR] += dt;

                    if (gl.jump_dt[PREV] >= 1.0f)
                        gd.jump_b = 0;
                }

                part_step(v, dt);
            }

            break;

        case CMD_MAKE_BALL:
            sol_lerp_cmd(&gl.lerp, &cs, cmd);
            break;

        case CMD_MAKE_ITEM:
            /* Allocate and initialize a new item. */

            if ((hp = realloc(vary->hv, sizeof (*hp) * (vary->hc + 1))))
            {
                vary->hv = hp;
                hp = &vary->hv[vary->hc];
                vary->hc++;

                memset(hp, 0, sizeof (*hp));

                v_cpy(hp->p, cmd->mkitem.p);

                hp->t = cmd->mkitem.t;
                hp->n = cmd->mkitem.n;
            }

            break;

        case CMD_PICK_ITEM:
            /* Set up particle effects and discard the item. */

            if ((idx = cmd->pkitem.hi) >= 0 && idx < vary->hc)
            {
                hp = &vary->hv[idx];

                item_color(hp, v);
                part_burst(hp->p, v);

                hp->t = ITEM_NONE;
            }
            break;

        case CMD_TILT_ANGLES:
            if (!cs.got_tilt_axes)
            {
                /*
                 * Neverball <= 1.5.1 does not send explicit tilt
                 * axes, rotation happens directly around view
                 * vectors.  So for compatibility if at the time of
                 * receiving tilt angles we have not yet received the
                 * tilt axes, we use the view vectors.
                 */

                game_tilt_axes(tilt, view->e);
            }

            tilt->rx = cmd->tiltangles.x;
            tilt->rz = cmd->tiltangles.z;
            break;

        case CMD_SOUND:
            /* Play the sound. */

            if (cmd->sound.n)
                audio_play(cmd->sound.n, cmd->sound.a);

            break;

        case CMD_TIMER:
            timer = cmd->timer.t;
            break;

        case CMD_STATUS:
            status = cmd->status.t;
            break;

        case CMD_COINS:
            coins = cmd->coins.n;
            break;

        case CMD_JUMP_ENTER:
            gd.jump_b  = 1;
            gd.jump_e  = 0;
            gl.jump_dt[PREV] = 0.0f;
            gl.jump_dt[CURR] = 0.0f;
            break;

        case CMD_JUMP_EXIT:
            gd.jump_e = 1;
            break;

        case CMD_MOVE_PATH:
        case CMD_MOVE_TIME:
        case CMD_BODY_PATH:
        case CMD_BODY_TIME:
            sol_lerp_cmd(&gl.lerp, &cs, cmd);
            break;

        case CMD_GOAL_OPEN:
            /*
             * Enable the goal and make sure it's fully visible if
             * this is the first update.
             */

            if (!gd.goal_e)
            {
                gd.goal_e = 1;
                gl.goal_k[CURR] = cs.first_update ? 1.0f : 0.0f;
            }
            break;

        case CMD_SWCH_ENTER:
            if ((idx = cmd->swchenter.xi) >= 0 && idx < vary->xc)
                vary->xv[idx].e = 1;
            break;

        case CMD_SWCH_TOGGLE:
            if ((idx = cmd->swchtoggle.xi) >= 0 && idx < vary->xc)
                vary->xv[idx].f = !vary->xv[idx].f;
            break;

        case CMD_SWCH_EXIT:
            if ((idx = cmd->swchexit.xi) >= 0 && idx < vary->xc)
                vary->xv[idx].e = 0;
            break;

        case CMD_UPDATES_PER_SECOND:
            cs.ups = cmd->ups.n;
            break;

        case CMD_BALL_RADIUS:
            sol_lerp_cmd(&gl.lerp, &cs, cmd);
            break;

        case CMD_CLEAR_ITEMS:
            free(vary->hv);
            vary->hv = NULL;
            vary->hc = 0;
            break;

        case CMD_CLEAR_BALLS:
            sol_lerp_cmd(&gl.lerp, &cs, cmd);
            break;

        case CMD_BALL_POSITION:
            sol_lerp_cmd(&gl.lerp, &cs, cmd);
            break;

        case CMD_BALL_BASIS:
            sol_lerp_cmd(&gl.lerp, &cs, cmd);
            break;

        case CMD_BALL_PEND_BASIS:
            sol_lerp_cmd(&gl.lerp, &cs, cmd);
            break;

        case CMD_VIEW_POSITION:
            v_cpy(view->p, cmd->viewpos.p);
            break;

        case CMD_VIEW_CENTER:
            v_cpy(view->c, cmd->viewcenter.c);
            break;

        case CMD_VIEW_BASIS:
            v_cpy(view->e[0], cmd->viewbasis.e[0]);
            v_cpy(view->e[1], cmd->viewbasis.e[1]);
            v_crs(view->e[2], view->e[0], view->e[1]);
            break;

        case CMD_CURRENT_BALL:
            if ((idx = cmd->currball.ui) >= 0 && idx < vary->uc)
                cs.curr_ball = idx;
            break;

        case CMD_PATH_FLAG:
            if ((idx = cmd->pathflag.pi) >= 0 && idx < vary->pc)
                vary->pv[idx].f = cmd->pathflag.f;
            break;

        case CMD_STEP_SIMULATION:
            sol_lerp_cmd(&gl.lerp, &cs, cmd);
            break;

        case CMD_MAP:
            /*
             * Note a version (mis-)match between the loaded map and what
             * the server has. (This doesn't actually load a map.)
             */
            game_compat_map = (version.x == cmd->map.version.x);
            break;

        case CMD_TILT_AXES:
            cs.got_tilt_axes = 1;
            v_cpy(tilt->x, cmd->tiltaxes.x);
            v_cpy(tilt->z, cmd->tiltaxes.z);
            break;

        case CMD_NONE:
        case CMD_MAX:
            break;
        }
    }
}

void game_client_sync(fs_file demo_fp)
{
    union cmd *cmdp;

    while ((cmdp = game_proxy_deq()))
    {
        if (demo_fp)
            cmd_put(demo_fp, cmdp);

        game_run_cmd(cmdp);

        cmd_free(cmdp);
    }
}

/*---------------------------------------------------------------------------*/

int  game_client_init(const char *file_name)
{
    char *back_name = "", *grad_name = "";
    int i;

    coins  = 0;
    status = GAME_NONE;

    game_client_free(file_name);

    /* Load SOL data. */

    if (!game_base_load(file_name))
        return (gd.state = 0);

    if (!sol_load_vary(&gd.vary, &game_base))
    {
        game_base_free(NULL);
        return (gd.state = 0);
    }

    if (!sol_load_draw(&gd.draw, &gd.vary, config_get_d(CONFIG_SHADOW)))
    {
        sol_free_vary(&gd.vary);
        game_base_free(NULL);
        return (gd.state = 0);
    }

    gd.state = 1;

    /* Initialize game state. */

    game_tilt_init(&gd.tilt);
    game_view_init(&gd.view);

    gd.jump_e  = 1;
    gd.jump_b  = 0;
    gd.jump_dt = 0.0f;

    gd.goal_e = 0;
    gd.goal_k = 0.0f;

    /* Initialize interpolation. */

    game_lerp_init(&gl, &gd);

    /* Initialize fade. */

    gd.fade_k =  1.0f;
    gd.fade_d = -2.0f;

    /* Load level info. */

    version.x = 0;
    version.y = 0;

    for (i = 0; i < gd.vary.base->dc; i++)
    {
        char *k = gd.vary.base->av + gd.vary.base->dv[i].ai;
        char *v = gd.vary.base->av + gd.vary.base->dv[i].aj;

        if (strcmp(k, "back") == 0) back_name = v;
        if (strcmp(k, "grad") == 0) grad_name = v;

        if (strcmp(k, "version") == 0)
            sscanf(v, "%d.%d", &version.x, &version.y);
    }

    /*
     * If the version of the loaded map is 1, assume we have a version
     * match with the server.  In this way 1.5.0 replays don't trigger
     * bogus map compatibility warnings.  Post-1.5.0 replays will have
     * CMD_MAP override this.
     */

    game_compat_map = version.x == 1;

    /* Initialize particles. */

    part_reset();

    /* Initialize command state. */

    cmd_state_init(&cs);

    /* Initialize background. */

    back_init(grad_name);
    sol_load_full(&gd.back, back_name, 0);

    /* Initialize lighting. */

    light_reset();

    return gd.state;
}

void game_client_free(const char *next)
{
    if (gd.state)
    {
        game_proxy_clr();

        game_lerp_free(&gl);

        sol_free_draw(&gd.draw);
        sol_free_vary(&gd.vary);

        game_base_free(next);

        sol_free_full(&gd.back);
        back_free();
    }
    gd.state = 0;
}

/*---------------------------------------------------------------------------*/

int enable_interpolation = 1;

void game_client_blend(float a)
{
    if (enable_interpolation)
        gl.alpha = a;
    else
        gl.alpha = 1.0f;
}

void game_client_draw(int pose, float t)
{
    game_lerp_apply(&gl, &gd);
    game_draw(&gd, pose, t);
}

/*---------------------------------------------------------------------------*/

int curr_clock(void)
{
    return (int) (timer * 100.f);
}

int curr_gained(void)
{
    return gained * 100;
}

void incr_gained(int amt)
{
    gained += amt;
}

void clear_gain(void)
{
    gained = 0;
}

int curr_coins(void)
{
    return coins;
}

int curr_status(void)
{
    return status;
}

/*---------------------------------------------------------------------------*/

void game_look(float phi, float theta)
{
    struct game_view *view = &gl.view[CURR];

    view->c[0] = view->p[0] + fsinf(V_RAD(theta)) * fcosf(V_RAD(phi));
    view->c[1] = view->p[1] +                       fsinf(V_RAD(phi));
    view->c[2] = view->p[2] - fcosf(V_RAD(theta)) * fcosf(V_RAD(phi));

    gl.view[PREV] = gl.view[CURR];
}

/*---------------------------------------------------------------------------*/

void game_kill_fade(void)
{
    gd.fade_k = 0.0f;
    gd.fade_d = 0.0f;
}

void game_step_fade(float dt)
{
    if ((gd.fade_k < 1.0f && gd.fade_d > 0.0f) ||
        (gd.fade_k > 0.0f && gd.fade_d < 0.0f))
        gd.fade_k += gd.fade_d * dt;

    if (gd.fade_k < 0.0f)
    {
        gd.fade_k = 0.0f;
        gd.fade_d = 0.0f;
    }
    if (gd.fade_k > 1.0f)
    {
        gd.fade_k = 1.0f;
        gd.fade_d = 0.0f;
    }
}

void game_fade(float d)
{
    gd.fade_d = d;
}

/*---------------------------------------------------------------------------*/

void game_client_fly(float k)
{
    game_view_fly(&gl.view[CURR], &gd.vary, k);

    gl.view[PREV] = gl.view[CURR];
}

/*---------------------------------------------------------------------------*/
