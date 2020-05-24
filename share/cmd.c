/*
 * Copyright (C) 2009 Neverball authors
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

#include <stdio.h>
#include <assert.h>

#include "cmd.h"
#include "binary.h"
#include "base_config.h"
#include "common.h"

/*---------------------------------------------------------------------------*/

static int cmd_stats = 0;

/*---------------------------------------------------------------------------*/

/*
 * Let's pretend these aren't Ridiculously Convoluted Macros from
 * Hell, and that all this looks pretty straight-forward.  (In all
 * fairness, the macros have paid off.)
 *
 * A command's "write" and "read" functions are defined by calling the
 * PUT_FUNC or the GET_FUNC macro, respectively, with the command type
 * as argument, followed by the body of the function (which has
 * variables "fp" and "cmd" available), and finalised with the
 * END_FUNC macro, which must be terminated with a semi-colon.  Before
 * the function definitions, the BYTES macro must be redefined for
 * each command to an expression evaluating to the number of bytes
 * that the command will occupy in the file.  (See existing commands
 * for examples.)
 */

#define PUT_FUNC(type, bytes)                                           \
    static void cmd_put_ ## type(fs_file fp, const union cmd *cmd) {    \
    const char *cmd_name = #type;                                       \
                                                                        \
    /* This is a write, so BYTES should be safe to eval already. */     \
    short cmd_bytes = (bytes);                                            \
                                                                        \
    /* Write command size info (right after the command type). */       \
    put_short(fp, cmd_bytes);                                           \
                                                                        \
    /* Start the stats output. */                                       \
    if (cmd_stats) printf("put");                                       \

#define GET_FUNC(type, bytes)                                   \
    static void cmd_get_ ## type(fs_file fp, union cmd *cmd) {  \
    const char *cmd_name = #type;                               \
                                                                \
    /* This is a read, so we'll have to eval BYTES later. */    \
    short cmd_bytes = -1;                                       \
                                                                \
    /* Start the stats output. */                               \
    if (cmd_stats) printf("get");

#define END_FUNC(bytes)                                                 \
    if (cmd_bytes < 0) cmd_bytes = (bytes);                             \
                                                                        \
    /* Finish the stats output. */                                      \
    if (cmd_stats) printf("\t%s\t%d\n", cmd_name, cmd_bytes);           \
    } struct dummy              /* Allows a trailing semi-colon. */

#define DEFINE_CMD(type, bytes, put_func_body, get_func_body) \
    PUT_FUNC(type, bytes) \
    put_func_body \
    END_FUNC(bytes); \
    GET_FUNC(type, bytes) \
    get_func_body \
    END_FUNC(bytes)

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_END_OF_UPDATE, 0, {}, {});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_MAKE_BALL, 0, {}, {});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_MAKE_ITEM, ARRAY_BYTES(3) + INDEX_BYTES + INDEX_BYTES, {
    put_array(fp, cmd->mkitem.p, 3);
    put_index(fp, cmd->mkitem.t);
    put_index(fp, cmd->mkitem.n);
}, {
    get_array(fp, cmd->mkitem.p, 3);

    cmd->mkitem.t = get_index(fp);
    cmd->mkitem.n = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_PICK_ITEM, INDEX_BYTES, {
    put_index(fp, cmd->pkitem.hi);
}, {
    cmd->pkitem.hi = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_TILT_ANGLES, FLOAT_BYTES + FLOAT_BYTES, {
    put_float(fp, cmd->tiltangles.x);
    put_float(fp, cmd->tiltangles.z);
}, {
    cmd->tiltangles.x = get_float(fp);
    cmd->tiltangles.z = get_float(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_SOUND, STRING_BYTES(cmd->sound.n) + FLOAT_BYTES, {
    put_string(fp, cmd->sound.n);
    put_float(fp, cmd->sound.a);
}, {
    static char buff[MAXSTR];

    get_string(fp, buff, sizeof (buff));

    cmd->sound.a = get_float(fp);
    cmd->sound.n = strdup(buff);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_TIMER, FLOAT_BYTES, {
    put_float(fp, cmd->timer.t);
}, {
    cmd->timer.t = get_float(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_STATUS, INDEX_BYTES, {
    put_index(fp, cmd->status.t);
}, {
    cmd->status.t = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_COINS, INDEX_BYTES, {
    put_index(fp, cmd->coins.n);
}, {
    cmd->coins.n = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_JUMP_ENTER, 0, {}, {});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_JUMP_EXIT, 0, {}, {});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_BODY_PATH, INDEX_BYTES + INDEX_BYTES, {
    put_index(fp, cmd->bodypath.bi);
    put_index(fp, cmd->bodypath.pi);
}, {
    cmd->bodypath.bi = get_index(fp);
    cmd->bodypath.pi = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_BODY_TIME, INDEX_BYTES + FLOAT_BYTES, {
    put_index(fp, cmd->bodytime.bi);
    put_float(fp, cmd->bodytime.t);
}, {
    cmd->bodytime.bi = get_index(fp);
    cmd->bodytime.t  = get_float(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_GOAL_OPEN, 0, {}, {});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_SWCH_ENTER, INDEX_BYTES, {
    put_index(fp, cmd->swchenter.xi);
}, {
    cmd->swchenter.xi = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_SWCH_TOGGLE, INDEX_BYTES, {
    put_index(fp, cmd->swchenter.xi);
}, {
    cmd->swchenter.xi = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_SWCH_EXIT, INDEX_BYTES, {
    put_index(fp, cmd->swchenter.xi);
}, {
    cmd->swchenter.xi = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_UPDATES_PER_SECOND, INDEX_BYTES, {
    put_index(fp, cmd->ups.n);
}, {
    cmd->ups.n = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_BALL_RADIUS, FLOAT_BYTES, {
    put_float(fp, cmd->ballradius.r);
}, {
    cmd->ballradius.r = get_float(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_CLEAR_ITEMS, 0, {}, {});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_CLEAR_BALLS, 0, {}, {});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_BALL_POSITION, ARRAY_BYTES(3), {
    put_array(fp, cmd->ballpos.p, 3);
}, {
    get_array(fp, cmd->ballpos.p, 3);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_BALL_BASIS, ARRAY_BYTES(3) + ARRAY_BYTES(3), {
    put_array(fp, cmd->ballbasis.e[0], 3);
    put_array(fp, cmd->ballbasis.e[1], 3);
}, {
    get_array(fp, cmd->ballbasis.e[0], 3);
    get_array(fp, cmd->ballbasis.e[1], 3);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_BALL_PEND_BASIS, ARRAY_BYTES(3) + ARRAY_BYTES(3), {
    put_array(fp, cmd->ballpendbasis.E[0], 3);
    put_array(fp, cmd->ballpendbasis.E[1], 3);
}, {
    get_array(fp, cmd->ballpendbasis.E[0], 3);
    get_array(fp, cmd->ballpendbasis.E[1], 3);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_VIEW_POSITION, ARRAY_BYTES(3), {
    put_array(fp, cmd->viewpos.p, 3);
}, {
    get_array(fp, cmd->viewpos.p, 3);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_VIEW_CENTER, ARRAY_BYTES(3), {
    put_array(fp, cmd->viewcenter.c, 3);
}, {
    get_array(fp, cmd->viewcenter.c, 3);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_VIEW_BASIS, ARRAY_BYTES(3) + ARRAY_BYTES(3), {
    put_array(fp, cmd->viewbasis.e[0], 3);
    put_array(fp, cmd->viewbasis.e[1], 3);
}, {
    get_array(fp, cmd->viewbasis.e[0], 3);
    get_array(fp, cmd->viewbasis.e[1], 3);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_CURRENT_BALL, INDEX_BYTES, {
    put_index(fp, cmd->currball.ui);
}, {
    cmd->currball.ui = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_PATH_FLAG, INDEX_BYTES + INDEX_BYTES, {
    put_index(fp, cmd->pathflag.pi);
    put_index(fp, cmd->pathflag.f);
}, {
    cmd->pathflag.pi = get_index(fp);
    cmd->pathflag.f  = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_STEP_SIMULATION, FLOAT_BYTES, {
    put_float(fp, cmd->stepsim.dt);
}, {
    cmd->stepsim.dt = get_float(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_MAP, STRING_BYTES(cmd->map.name) + INDEX_BYTES * 2, {
    put_string(fp, cmd->map.name);

    put_index(fp, cmd->map.version.x);
    put_index(fp, cmd->map.version.y);
}, {
    char buff[MAXSTR];

    get_string(fp, buff, sizeof (buff));

    cmd->map.name = strdup(buff);

    cmd->map.version.x = get_index(fp);
    cmd->map.version.y = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_TILT_AXES, ARRAY_BYTES(3) * 2, {
    put_array(fp, cmd->tiltaxes.x, 3);
    put_array(fp, cmd->tiltaxes.z, 3);
}, {
    get_array(fp, cmd->tiltaxes.x, 3);
    get_array(fp, cmd->tiltaxes.z, 3);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_MOVE_PATH, INDEX_BYTES + INDEX_BYTES, {
    put_index(fp, cmd->movepath.mi);
    put_index(fp, cmd->movepath.pi);
}, {
    cmd->movepath.mi = get_index(fp);
    cmd->movepath.pi = get_index(fp);
});

/*---------------------------------------------------------------------------*/

DEFINE_CMD(CMD_MOVE_TIME, INDEX_BYTES + FLOAT_BYTES, {
    put_index(fp, cmd->movetime.mi);
    put_float(fp, cmd->movetime.t);
}, {
    cmd->movetime.mi = get_index(fp);
    cmd->movetime.t  = get_float(fp);
});

/*---------------------------------------------------------------------------*/

#define PUT_CASE(t) case t: cmd_put_ ## t(fp, cmd); break
#define GET_CASE(t) case t: cmd_get_ ## t(fp, cmd); break

int cmd_put(fs_file fp, const union cmd *cmd)
{
    if (!fp || !cmd)
        return 0;

    assert(cmd->type > CMD_NONE && cmd->type < CMD_MAX);

    fs_putc(cmd->type, fp);

    switch (cmd->type)
    {
        PUT_CASE(CMD_END_OF_UPDATE);
        PUT_CASE(CMD_MAKE_BALL);
        PUT_CASE(CMD_MAKE_ITEM);
        PUT_CASE(CMD_PICK_ITEM);
        PUT_CASE(CMD_TILT_ANGLES);
        PUT_CASE(CMD_SOUND);
        PUT_CASE(CMD_TIMER);
        PUT_CASE(CMD_STATUS);
        PUT_CASE(CMD_COINS);
        PUT_CASE(CMD_JUMP_ENTER);
        PUT_CASE(CMD_JUMP_EXIT);
        PUT_CASE(CMD_BODY_PATH);
        PUT_CASE(CMD_BODY_TIME);
        PUT_CASE(CMD_GOAL_OPEN);
        PUT_CASE(CMD_SWCH_ENTER);
        PUT_CASE(CMD_SWCH_TOGGLE);
        PUT_CASE(CMD_SWCH_EXIT);
        PUT_CASE(CMD_UPDATES_PER_SECOND);
        PUT_CASE(CMD_BALL_RADIUS);
        PUT_CASE(CMD_CLEAR_ITEMS);
        PUT_CASE(CMD_CLEAR_BALLS);
        PUT_CASE(CMD_BALL_POSITION);
        PUT_CASE(CMD_BALL_BASIS);
        PUT_CASE(CMD_BALL_PEND_BASIS);
        PUT_CASE(CMD_VIEW_POSITION);
        PUT_CASE(CMD_VIEW_CENTER);
        PUT_CASE(CMD_VIEW_BASIS);
        PUT_CASE(CMD_CURRENT_BALL);
        PUT_CASE(CMD_PATH_FLAG);
        PUT_CASE(CMD_STEP_SIMULATION);
        PUT_CASE(CMD_MAP);
        PUT_CASE(CMD_TILT_AXES);
        PUT_CASE(CMD_MOVE_PATH);
        PUT_CASE(CMD_MOVE_TIME);

    case CMD_NONE:
    case CMD_MAX:
        break;
    }

    return !fs_eof(fp);
}

int cmd_get(fs_file fp, union cmd *cmd)
{
    int type;
    short size;

    if (!fp || !cmd)
        return 0;

    if ((type = fs_getc(fp)) >= 0)
    {
        size = get_short(fp);

        /* Discard unrecognised commands. */

        if (type >= CMD_MAX)
        {
            fs_seek(fp, size, SEEK_CUR);
            type = CMD_NONE;
        }

        cmd->type = type;

        switch (cmd->type)
        {
            GET_CASE(CMD_END_OF_UPDATE);
            GET_CASE(CMD_MAKE_BALL);
            GET_CASE(CMD_MAKE_ITEM);
            GET_CASE(CMD_PICK_ITEM);
            GET_CASE(CMD_TILT_ANGLES);
            GET_CASE(CMD_SOUND);
            GET_CASE(CMD_TIMER);
            GET_CASE(CMD_STATUS);
            GET_CASE(CMD_COINS);
            GET_CASE(CMD_JUMP_ENTER);
            GET_CASE(CMD_JUMP_EXIT);
            GET_CASE(CMD_BODY_PATH);
            GET_CASE(CMD_BODY_TIME);
            GET_CASE(CMD_GOAL_OPEN);
            GET_CASE(CMD_SWCH_ENTER);
            GET_CASE(CMD_SWCH_TOGGLE);
            GET_CASE(CMD_SWCH_EXIT);
            GET_CASE(CMD_UPDATES_PER_SECOND);
            GET_CASE(CMD_BALL_RADIUS);
            GET_CASE(CMD_CLEAR_ITEMS);
            GET_CASE(CMD_CLEAR_BALLS);
            GET_CASE(CMD_BALL_POSITION);
            GET_CASE(CMD_BALL_BASIS);
            GET_CASE(CMD_BALL_PEND_BASIS);
            GET_CASE(CMD_VIEW_POSITION);
            GET_CASE(CMD_VIEW_CENTER);
            GET_CASE(CMD_VIEW_BASIS);
            GET_CASE(CMD_CURRENT_BALL);
            GET_CASE(CMD_PATH_FLAG);
            GET_CASE(CMD_STEP_SIMULATION);
            GET_CASE(CMD_MAP);
            GET_CASE(CMD_TILT_AXES);
            GET_CASE(CMD_MOVE_PATH);
            GET_CASE(CMD_MOVE_TIME);

        case CMD_NONE:
        case CMD_MAX:
            break;
        }

        return !fs_eof(fp);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void cmd_free(union cmd *cmd)
{
    if (cmd)
    {
        switch (cmd->type)
        {
        case CMD_SOUND:
            free(cmd->sound.n);
            break;

        case CMD_MAP:
            free(cmd->map.name);
            break;

        default:
            break;
        }

        free(cmd);
    }
}

/*---------------------------------------------------------------------------*/
