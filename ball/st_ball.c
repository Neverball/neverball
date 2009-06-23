#include "gui.h"
#include "state.h"
#include "array.h"
#include "dir.h"
#include "config.h"
#include "fs.h"
#include "common.h"
#include "ball.h"
#include "cmd.h"

#include "game_server.h"
#include "game_proxy.h"
#include "game_client.h"

#include "st_ball.h"
#include "st_shared.h"
#include "st_conf.h"

static Array balls;
static int   curr_ball;
static char  ball_file[64];

void scan_balls(void)
{
    int i;

    config_get_s(CONFIG_BALL_FILE, ball_file, sizeof (ball_file) - 1);

    if ((balls = fs_dir_scan("ball", NULL)))
    {
        for (i = 0; i < array_len(balls); i++)
        {
            const char *path = DIR_ITEM_GET(balls, i)->path;

            if (strncmp(ball_file, path, strlen(path)) == 0)
            {
                curr_ball = i;
                break;
            }
        }
    }
}

void free_balls(void)
{
    fs_dir_free(balls);
    balls = NULL;
}

static void set_curr_ball(void)
{
    sprintf(ball_file, "%s/%s",
            DIR_ITEM_GET(balls, curr_ball)->path,
            base_name(DIR_ITEM_GET(balls, curr_ball)->path, NULL));

    config_set_s(CONFIG_BALL_FILE, ball_file);

    ball_free();
    ball_init();
}

static void next_ball(void)
{
    if (++curr_ball == array_len(balls))
        curr_ball = 0;

    set_curr_ball();
}

static void prev_ball(void)
{
    if (--curr_ball == -1)
        curr_ball = array_len(balls) - 1;

    set_curr_ball();
}

static int ball_enter(void)
{
    scan_balls();

    game_client_init("map-medium/title.sol");
    game_set_fly(0, game_client_file());
    game_client_step(NULL);

    return 0;
}

static void ball_leave(int id)
{
    free_balls();
}

static void ball_timer(int id, float dt)
{
    game_step_fade(dt);
}

static void ball_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
    {
        if (v > 0 && v != 1)
            next_ball();

        if (v < 0 && v != 1)
            prev_ball();
    }
}

static int ball_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b) ||
            config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_conf);
    }
    return 1;
}

struct state st_ball = {
    ball_enter,
    ball_leave,
    shared_paint,
    ball_timer,
    NULL,
    ball_stick,
    NULL,
    NULL,
    NULL,
    ball_buttn,
    1, 0
};
