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
#include "solid_gl.h"
#include "config.h"
#include "video.h"

#include "game_client.h"
#include "game_common.h"
#include "game_proxy.h"
#include "game_draw.h"

#include "cmd.h"

/*---------------------------------------------------------------------------*/

int game_compat_map;                    /* Client/server map compat flag     */

/*---------------------------------------------------------------------------*/

static struct game_draw dr;

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

/*---------------------------------------------------------------------------*/

static void game_run_cmd(const union cmd *cmd)
{
    if (dr.state)
    {
        s_file *fp = &dr.file;
        s_item *hp;
        s_ball *up;

        float v[3];
        float dt;
        int i;

        switch (cmd->type)
        {
        case CMD_END_OF_UPDATE:

            got_tilt_axes = 0;

            if (first_update)
            {
                first_update = 0;
                break;
            }

            /* Compute gravity for particle effects. */

            if (status == GAME_GOAL)
                game_tilt_grav(v, GRAVITY_UP, &dr.tilt);
            else
                game_tilt_grav(v, GRAVITY_DN, &dr.tilt);

            /* Step particle, goal and jump effects. */

            if (ups > 0)
            {
                dt = 1.0f / (float) ups;

                if (dr.goal_e && dr.goal_k < 1.0f)
                    dr.goal_k += dt;

                if (dr.jump_b)
                {
                    dr.jump_dt += dt;

                    if (1.0f < dr.jump_dt)
                        dr.jump_b = 0;
                }

                part_step(v, dt);
            }

            break;

        case CMD_MAKE_BALL:
            /* Allocate a new ball and mark it as the current ball. */

            if ((up = realloc(fp->uv, sizeof (*up) * (fp->uc + 1))))
            {
                fp->uv = up;
                curr_ball = fp->uc;
                fp->uc++;
            }
            break;

        case CMD_MAKE_ITEM:
            /* Allocate and initialise a new item. */

            if ((hp = realloc(fp->hv, sizeof (*hp) * (fp->hc + 1))))
            {
                s_item h;

                v_cpy(h.p, cmd->mkitem.p);

                h.t = cmd->mkitem.t;
                h.n = cmd->mkitem.n;

                fp->hv = hp;
                fp->hv[fp->hc] = h;
                fp->hc++;
            }

            break;

        case CMD_PICK_ITEM:
            /* Set up particle effects and discard the item. */

            assert(cmd->pkitem.hi < fp->hc);

            hp = fp->hv + cmd->pkitem.hi;

            item_color(hp, v);
            part_burst(hp->p, v);

            hp->t = ITEM_NONE;

            break;

        case CMD_TILT_ANGLES:
            if (!got_tilt_axes)
                game_tilt_axes(&dr.tilt, dr.view.e);

            dr.tilt.rx = cmd->tiltangles.x;
            dr.tilt.rz = cmd->tiltangles.z;
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
            dr.jump_b  = 1;
            dr.jump_e  = 0;
            dr.jump_dt = 0.0f;
            break;

        case CMD_JUMP_EXIT:
            dr.jump_e = 1;
            break;

        case CMD_BODY_PATH:
            fp->bv[cmd->bodypath.bi].pi = cmd->bodypath.pi;
            break;

        case CMD_BODY_TIME:
            fp->bv[cmd->bodytime.bi].t = cmd->bodytime.t;
            break;

        case CMD_GOAL_OPEN:
            /*
             * Enable the goal and make sure it's fully visible if
             * this is the first update.
             */

            if (!dr.goal_e)
            {
                dr.goal_e = 1;
                dr.goal_k = first_update ? 1.0f : 0.0f;
            }
            break;

        case CMD_SWCH_ENTER:
            fp->xv[cmd->swchenter.xi].e = 1;
            break;

        case CMD_SWCH_TOGGLE:
            fp->xv[cmd->swchtoggle.xi].f = !fp->xv[cmd->swchtoggle.xi].f;
            break;

        case CMD_SWCH_EXIT:
            fp->xv[cmd->swchexit.xi].e = 0;
            break;

        case CMD_UPDATES_PER_SECOND:
            ups = cmd->ups.n;
            break;

        case CMD_BALL_RADIUS:
            fp->uv[curr_ball].r = cmd->ballradius.r;
            break;

        case CMD_CLEAR_ITEMS:
            if (fp->hv)
            {
                free(fp->hv);
                fp->hv = NULL;
            }
            fp->hc = 0;
            break;

        case CMD_CLEAR_BALLS:
            if (fp->uv)
            {
                free(fp->uv);
                fp->uv = NULL;
            }
            fp->uc = 0;
            break;

        case CMD_BALL_POSITION:
            up = fp->uv + curr_ball;

            v_cpy(up->p, cmd->ballpos.p);
            break;

        case CMD_BALL_BASIS:
            up = fp->uv + curr_ball;

            v_cpy(up->e[0], cmd->ballbasis.e[0]);
            v_cpy(up->e[1], cmd->ballbasis.e[1]);
            v_crs(up->e[2], up->e[0], up->e[1]);
            break;

        case CMD_BALL_PEND_BASIS:
            up = fp->uv + curr_ball;

            v_cpy(up->E[0], cmd->ballpendbasis.E[0]);
            v_cpy(up->E[1], cmd->ballpendbasis.E[1]);
            v_crs(up->E[2], up->E[0], up->E[1]);
            break;

        case CMD_VIEW_POSITION:
            v_cpy(dr.view.p, cmd->viewpos.p);
            break;

        case CMD_VIEW_CENTER:
            v_cpy(dr.view.c, cmd->viewcenter.c);
            break;

        case CMD_VIEW_BASIS:
            v_cpy(dr.view.e[0], cmd->viewbasis.e[0]);
            v_cpy(dr.view.e[1], cmd->viewbasis.e[1]);
            v_crs(dr.view.e[2], dr.view.e[0], dr.view.e[1]);
            break;

        case CMD_CURRENT_BALL:
            curr_ball = cmd->currball.ui;
            break;

        case CMD_PATH_FLAG:
            fp->pv[cmd->pathflag.pi].f = cmd->pathflag.f;
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

            dt = cmd->stepsim.dt;

            for (i = 0; i < fp->bc; i++)
            {
                s_body *bp = fp->bv + i;
                s_path *pp = fp->pv + bp->pi;

                if (bp->pi >= 0 && pp->f)
                    bp->t += dt;
            }
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
            v_cpy(dr.tilt.x, cmd->tiltaxes.x);
            v_cpy(dr.tilt.z, cmd->tiltaxes.z);
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

    if (dr.state)
        game_client_free();

    if (!sol_load_gl(&dr.file, file_name, config_get_d(CONFIG_SHADOW)))
        return (dr.state = 0);

    dr.reflective = sol_reflective(&dr.file);

    dr.state = 1;

    game_tilt_init(&dr.tilt);

    /* Initialize jump and goal states. */

    dr.jump_e = 1;
    dr.jump_b = 0;

    dr.goal_e = 0;
    dr.goal_k = 0.0f;

    /* Initialise the level, background, particles, fade, and view. */

    dr.fade_k =  1.0f;
    dr.fade_d = -2.0f;


    version.x = 0;
    version.y = 0;

    for (i = 0; i < dr.file.dc; i++)
    {
        char *k = dr.file.av + dr.file.dv[i].ai;
        char *v = dr.file.av + dr.file.dv[i].aj;

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

    part_reset(GOAL_HEIGHT, JUMP_HEIGHT);

    ups          = 0;
    first_update = 1;

    back_init(grad_name);
    sol_load_gl(&dr.back, back_name, 0);

    return dr.state;
}

void game_client_free(void)
{
    if (dr.state)
    {
        game_proxy_clr();
        sol_free_gl(&dr.file);
        sol_free_gl(&dr.back);
        back_free();
    }
    dr.state = 0;
}

/*---------------------------------------------------------------------------*/

void game_client_draw(int pose, float t)
{
    game_draw(&dr, pose, t);
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
    dr.view.c[0] = dr.view.p[0] + fsinf(V_RAD(theta)) * fcosf(V_RAD(phi));
    dr.view.c[1] = dr.view.p[1] +                       fsinf(V_RAD(phi));
    dr.view.c[2] = dr.view.p[2] - fcosf(V_RAD(theta)) * fcosf(V_RAD(phi));
}

/*---------------------------------------------------------------------------*/

void game_kill_fade(void)
{
    dr.fade_k = 0.0f;
    dr.fade_d = 0.0f;
}

void game_step_fade(float dt)
{
    if ((dr.fade_k < 1.0f && dr.fade_d > 0.0f) ||
        (dr.fade_k > 0.0f && dr.fade_d < 0.0f))
        dr.fade_k += dr.fade_d * dt;

    if (dr.fade_k < 0.0f)
    {
        dr.fade_k = 0.0f;
        dr.fade_d = 0.0f;
    }
    if (dr.fade_k > 1.0f)
    {
        dr.fade_k = 1.0f;
        dr.fade_d = 0.0f;
    }
}

void game_fade(float d)
{
    dr.fade_d = d;
}

/*---------------------------------------------------------------------------*/

void game_client_fly(float k)
{
    game_view_fly(&dr.view, &dr.file, k);
}

/*---------------------------------------------------------------------------*/
