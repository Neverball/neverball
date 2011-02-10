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
#include "item.h"
#include "back.h"
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
static int   status = GAME_NONE;        /* Outcome of the game               */
static int   coins  = 0;                /* Collected coins                   */

static int ups;                         /* Updates per second                */
static int first_update;                /* First update flag                 */
static int curr_ball;                   /* Current ball index                */

struct
{
    int x, y;
} version;                              /* Current map version               */

/*
 * Neverball <= 1.5.1 does not send explicit tilt axes, rotation
 * happens directly around view vectors.  So for compatibility if at
 * the time of receiving tilt angles we have not yet received the tilt
 * axes, we use the view vectors.
 */

static int got_tilt_axes;

static int next_update;

/*---------------------------------------------------------------------------*/

static void game_run_cmd(const union cmd *cmd)
{
    if (gd.state)
    {
        struct game_view *view = &gl.view[CURR];
        struct game_tilt *tilt = &gl.tilt[CURR];

        struct s_vary *vary = &gd.file.vary;
        struct v_item *hp;

        float v[3];
        float dt;

        if (next_update)
        {
            game_lerp_copy(&gl);
            next_update = 0;
        }

        switch (cmd->type)
        {
        case CMD_END_OF_UPDATE:
            got_tilt_axes = 0;

            next_update = 1;

            if (first_update)
            {
                game_lerp_copy(&gl);
                /* Hack to sync state before the next update. */
                game_lerp_apply(&gl, &gd, 1.0f);
                first_update = 0;
                break;
            }

            /* Compute gravity for particle effects. */

            if (status == GAME_GOAL)
                game_tilt_grav(v, GRAVITY_UP, tilt);
            else
                game_tilt_grav(v, GRAVITY_DN, tilt);

            /* Step particle, goal and jump effects. */

            if (ups > 0)
            {
                dt = 1.0f / (float) ups;

                if (gd.goal_e && gl.goal_k[CURR] < 1.0f)
                    gl.goal_k[CURR] += dt;

                if (gd.jump_b)
                {
                    gl.jump_dt[CURR] += dt;

                    if (1.0f < gl.jump_dt[CURR])
                        gd.jump_b = 0;
                }

                part_step(v, dt);
            }

            break;

        case CMD_MAKE_BALL:
            /* Allocate a new ball and mark it as the current ball. */

            if (sol_lerp_cmd(&gl.lerp, cmd))
                curr_ball = gl.lerp.uc - 1;

            break;

        case CMD_MAKE_ITEM:
            /* Allocate and initialise a new item. */

            if ((hp = realloc(vary->hv, sizeof (*hp) * (vary->hc + 1))))
            {
                struct v_item h;

                v_cpy(h.p, cmd->mkitem.p);

                h.t = cmd->mkitem.t;
                h.n = cmd->mkitem.n;

                vary->hv = hp;
                vary->hv[vary->hc] = h;
                vary->hc++;
            }

            break;

        case CMD_PICK_ITEM:
            /* Set up particle effects and discard the item. */

            assert(cmd->pkitem.hi < vary->hc);

            hp = vary->hv + cmd->pkitem.hi;

            item_color(hp, v);
            part_burst(hp->p, v);

            hp->t = ITEM_NONE;

            break;

        case CMD_TILT_ANGLES:
            if (!got_tilt_axes)
                game_tilt_axes(tilt, view->e);

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
            gl.jump_dt[CURR] = 0.0f;
            break;

        case CMD_JUMP_EXIT:
            gd.jump_e = 1;
            break;

        case CMD_BODY_PATH:
            sol_lerp_cmd(&gl.lerp, cmd);
            break;

        case CMD_BODY_TIME:
            sol_lerp_cmd(&gl.lerp, cmd);
            break;

        case CMD_GOAL_OPEN:
            /*
             * Enable the goal and make sure it's fully visible if
             * this is the first update.
             */

            if (!gd.goal_e)
            {
                gd.goal_e = 1;
                gl.goal_k[CURR] = first_update ? 1.0f : 0.0f;
            }
            break;

        case CMD_SWCH_ENTER:
            vary->xv[cmd->swchenter.xi].e = 1;
            break;

        case CMD_SWCH_TOGGLE:
            vary->xv[cmd->swchtoggle.xi].f = !vary->xv[cmd->swchtoggle.xi].f;
            break;

        case CMD_SWCH_EXIT:
            vary->xv[cmd->swchexit.xi].e = 0;
            break;

        case CMD_UPDATES_PER_SECOND:
            ups = cmd->ups.n;
            break;

        case CMD_BALL_RADIUS:
            sol_lerp_cmd(&gl.lerp, cmd);
            break;

        case CMD_CLEAR_ITEMS:
            if (vary->hv)
            {
                free(vary->hv);
                vary->hv = NULL;
            }
            vary->hc = 0;
            break;

        case CMD_CLEAR_BALLS:
            sol_lerp_cmd(&gl.lerp, cmd);
            break;

        case CMD_BALL_POSITION:
            sol_lerp_cmd(&gl.lerp, cmd);
            break;

        case CMD_BALL_BASIS:
            sol_lerp_cmd(&gl.lerp, cmd);
            break;

        case CMD_BALL_PEND_BASIS:
            sol_lerp_cmd(&gl.lerp, cmd);
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
            sol_lerp_cmd(&gl.lerp, cmd);
            curr_ball = cmd->currball.ui;
            break;

        case CMD_PATH_FLAG:
            vary->pv[cmd->pathflag.pi].f = cmd->pathflag.f;
            break;

        case CMD_STEP_SIMULATION:
            sol_lerp_cmd(&gl.lerp, cmd);
            break;

        case CMD_MAP:
            /*
             * Note a version (mis-)match between the loaded map and what
             * the server has. (This doesn't actually load a map.)
             */
            game_compat_map = version.x == cmd->map.version.x;
            break;

        case CMD_TILT_AXES:
            got_tilt_axes = 1;
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

    if (gd.state)
        game_client_free();

    if (!sol_load_full(&gd.file, file_name, config_get_d(CONFIG_SHADOW)))
        return (gd.state = 0);

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

    for (i = 0; i < gd.file.base.dc; i++)
    {
        char *k = gd.file.base.av + gd.file.base.dv[i].ai;
        char *v = gd.file.base.av + gd.file.base.dv[i].aj;

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

    part_reset(GOAL_HEIGHT, JUMP_HEIGHT);

    /* Initialize command state. */

    ups          = 0;
    first_update = 1;

    /* Initialize background. */

    back_init(grad_name);
    sol_load_full(&gd.back, back_name, 0);

    return gd.state;
}

void game_client_free(void)
{
    if (gd.state)
    {
        game_proxy_clr();
        game_lerp_free(&gl);
        sol_free_full(&gd.file);
        sol_free_full(&gd.back);
        back_free();
    }
    gd.state = 0;
}

/*---------------------------------------------------------------------------*/

void game_client_draw(int pose, float t, float a)
{
    game_lerp_apply(&gl, &gd, a);
    game_draw(&gd, pose, t);
}

/*---------------------------------------------------------------------------*/

int curr_clock(void)
{
    return (int) (timer * 100.f);
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
    game_view_fly(&gl.view[CURR], &gd.file.vary, k);
}

/*---------------------------------------------------------------------------*/
