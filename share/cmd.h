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

#ifndef CMD_H
#define CMD_H

/*
 * In an attempt to improve replay compatibility, a few guidelines
 * apply to command addition, removal, and modification:
 *
 * - New commands are added at the bottom of the list.
 *
 * - Existing commands are never modified nor removed.
 *
 * - The list is never reordered.  (It's tempting...)
 *
 * However, commands can be renamed (e.g., to add a "deprecated" tag,
 * because it's superseded by another command).
 */

enum cmd_type
{
    CMD_NONE = 0,

    CMD_END_OF_UPDATE,
    CMD_MAKE_BALL,
    CMD_MAKE_ITEM,
    CMD_PICK_ITEM,
    CMD_TILT_ANGLES,
    CMD_SOUND,
    CMD_TIMER,
    CMD_STATUS,
    CMD_COINS,
    CMD_JUMP_ENTER,
    CMD_JUMP_EXIT,
    CMD_BODY_PATH,
    CMD_BODY_TIME,
    CMD_GOAL_OPEN,
    CMD_SWCH_ENTER,
    CMD_SWCH_TOGGLE,
    CMD_SWCH_EXIT,
    CMD_UPDATES_PER_SECOND,
    CMD_BALL_RADIUS,
    CMD_CLEAR_ITEMS,
    CMD_CLEAR_BALLS,
    CMD_BALL_POSITION,
    CMD_BALL_BASIS,
    CMD_BALL_PEND_BASIS,
    CMD_VIEW_POSITION,
    CMD_VIEW_CENTER,
    CMD_VIEW_BASIS,
    CMD_CURRENT_BALL,
    CMD_PATH_FLAG,
    CMD_STEP_SIMULATION,
    CMD_MAP,
    CMD_TILT_AXES,

    CMD_MAX
};

/*
 * Here are the members common to all structures.  Note that it
 * explicitly says "enum cmd_type", not "int".  This allows GCC to
 * catch and warn about unhandled command types in switch constructs
 * (handy when adding new commands).
 */

#define CMD_HEADER \
    enum cmd_type type

struct cmd_end_of_update
{
    CMD_HEADER;
};

struct cmd_make_ball
{
    CMD_HEADER;
};

struct cmd_make_item
{
    CMD_HEADER;
    float p[3];
    int   t;
    int   n;
};

struct cmd_pick_item
{
    CMD_HEADER;
    int   hi;
};

struct cmd_tilt_angles
{
    CMD_HEADER;
    float x;
    float z;
};

struct cmd_sound
{
    CMD_HEADER;
    char  *n;
    float  a;
};

struct cmd_timer
{
    CMD_HEADER;
    float t;
};

struct cmd_status
{
    CMD_HEADER;
    int t;
};

struct cmd_coins
{
    CMD_HEADER;
    int n;
};

struct cmd_jump_enter
{
    CMD_HEADER;
};

struct cmd_jump_exit
{
    CMD_HEADER;
};

struct cmd_body_path
{
    CMD_HEADER;
    int bi;
    int pi;
};

struct cmd_body_time
{
    CMD_HEADER;
    int   bi;
    float t;
};

struct cmd_goal_open
{
    CMD_HEADER;
};

struct cmd_swch_enter
{
    CMD_HEADER;
    int xi;
};

struct cmd_swch_toggle
{
    CMD_HEADER;
    int xi;
};

struct cmd_swch_exit
{
    CMD_HEADER;
    int xi;
};

struct cmd_updates_per_second
{
    CMD_HEADER;
    int n;
};

struct cmd_ball_radius
{
    CMD_HEADER;
    float r;
};

struct cmd_clear_items
{
    CMD_HEADER;
};

struct cmd_clear_balls
{
    CMD_HEADER;
};

struct cmd_ball_position
{
    CMD_HEADER;
    float p[3];
};

struct cmd_ball_basis
{
    CMD_HEADER;
    float e[2][3];
};

struct cmd_ball_pend_basis
{
    CMD_HEADER;
    float E[2][3];
};

struct cmd_view_position
{
    CMD_HEADER;
    float p[3];
};

struct cmd_view_center
{
    CMD_HEADER;
    float c[3];
};

struct cmd_view_basis
{
    CMD_HEADER;
    float e[2][3];
};

struct cmd_current_ball
{
    CMD_HEADER;
    int ui;
};

struct cmd_path_flag
{
    CMD_HEADER;
    int pi;
    int f;
};

struct cmd_step_simulation
{
    CMD_HEADER;
    float dt;
};

struct cmd_map
{
    CMD_HEADER;
    char *name;
    struct
    {
        int x, y;
    } version;
};

struct cmd_tilt_axes
{
    CMD_HEADER;
    float x[3], z[3];
};

union cmd
{
    CMD_HEADER;
    struct cmd_end_of_update      eou;
    struct cmd_make_ball          mkball;
    struct cmd_make_item          mkitem;
    struct cmd_pick_item          pkitem;
    struct cmd_tilt_angles        tiltangles;
    struct cmd_sound              sound;
    struct cmd_timer              timer;
    struct cmd_status             status;
    struct cmd_coins              coins;
    struct cmd_jump_enter         jumpenter;
    struct cmd_jump_exit          jumpexit;
    struct cmd_body_path          bodypath;
    struct cmd_body_time          bodytime;
    struct cmd_goal_open          goalopen;
    struct cmd_swch_enter         swchenter;
    struct cmd_swch_toggle        swchtoggle;
    struct cmd_swch_exit          swchexit;
    struct cmd_updates_per_second ups;
    struct cmd_ball_radius        ballradius;
    struct cmd_clear_items        clritems;
    struct cmd_clear_balls        clrballs;
    struct cmd_ball_position      ballpos;
    struct cmd_ball_basis         ballbasis;
    struct cmd_ball_pend_basis    ballpendbasis;
    struct cmd_view_position      viewpos;
    struct cmd_view_center        viewcenter;
    struct cmd_view_basis         viewbasis;
    struct cmd_current_ball       currball;
    struct cmd_path_flag          pathflag;
    struct cmd_step_simulation    stepsim;
    struct cmd_map                map;
    struct cmd_tilt_axes          tiltaxes;
};

#undef CMD_HEADER

#include "fs.h"

int cmd_put(fs_file, const union cmd *);
int cmd_get(fs_file, union cmd *);

void cmd_free(union cmd *);

#endif
