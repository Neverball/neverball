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

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "level.h"
#include "glext.h"
#include "image.h"
#include "game.h"
#include "geom.h"
#include "demo.h"
#include "hud.h"
#include "audio.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

struct score
{
    char time_n[4][MAXNAM];
    int  time_t[4];
    int  time_c[4];

    char coin_n[4][MAXNAM];
    int  coin_t[4];
    int  coin_c[4];
};

struct level
{
    char file[MAXSTR];
    char back[MAXSTR];
    char grad[MAXSTR];
    char shot[MAXSTR];
    char song[MAXSTR];
    int  time;
    int  goal;

    GLuint text;
};

static int score;                       /* Current coin total         */
static int coins;                       /* Current coin count         */
static int balls;                       /* Current life count         */
static int goal;                        /* Current goal count         */

static int level;                       /* Current level number       */
static int count;                       /* Number of levels           */
static int limit;                       /* Last opened (locked) level */
static int status;                      /* Status of current level    */

static int mode;			/* Current play mode          */

static int coins_total;
static int times_total;

static struct level level_v[MAXLVL];
static struct score score_v[MAXLVL];

static char scores_file[MAXSTR];

/*---------------------------------------------------------------------------*/

static void level_store_hs(const char *filename)
{
    FILE *fout;

    if ((fout = fopen(config_user(filename), "w")))
    {
        int i;
        int j;

        for (i = 0; i < limit; i++)
            for (j = 0; j < 3; j++)
            {
                if (strlen(score_v[i].time_n[j]) == 0)
                    strcpy(score_v[i].time_n[j], DEFAULT_PLAYER);
                if (strlen(score_v[i].coin_n[j]) == 0)
                    strcpy(score_v[i].coin_n[j], DEFAULT_PLAYER);

                fprintf(fout, "%d %d %s\n",
                        score_v[i].time_t[j],
                        score_v[i].time_c[j],
                        score_v[i].time_n[j]);
                fprintf(fout, "%d %d %s\n",
                        score_v[i].coin_t[j],
                        score_v[i].coin_c[j],
                        score_v[i].coin_n[j]);
            }
            
        fclose(fout);
    }
}

static void level_load_hs(const char *filename)
{
    FILE *fin;

    limit = 1;

    if ((fin = fopen(config_user(filename), "r")))
    {
        int i;

        for (i = 0; i < count; i++)
        {
            if (fscanf(fin, "%d %d %s",
                       &score_v[i].time_t[0],
                       &score_v[i].time_c[0],
                       score_v[i].time_n[0]) == 3 &&
                fscanf(fin, "%d %d %s",
                       &score_v[i].coin_t[0],
                       &score_v[i].coin_c[0],
                       score_v[i].coin_n[0]) == 3 &&
                fscanf(fin, "%d %d %s",
                       &score_v[i].time_t[1],
                       &score_v[i].time_c[1],
                       score_v[i].time_n[1]) == 3 &&
                fscanf(fin, "%d %d %s",
                       &score_v[i].coin_t[1],
                       &score_v[i].coin_c[1],
                       score_v[i].coin_n[1]) == 3 &&
                fscanf(fin, "%d %d %s",
                       &score_v[i].time_t[2],
                       &score_v[i].time_c[2],
                       score_v[i].time_n[2]) == 3 &&
                fscanf(fin, "%d %d %s",
                       &score_v[i].coin_t[2],
                       &score_v[i].coin_c[2],
                       score_v[i].coin_n[2]) == 3)
                limit = i + 1;
        }

        fclose(fin);
    }
}

/*---------------------------------------------------------------------------*/

static void level_init_rc(const char *filename)
{
    FILE *fin;
    char buf[MAXSTR];

    count = 0;
    level = 0;
    coins = 0;
    score = 0;
    balls = 0;

    /* Load the levels list. */

    if ((fin = fopen(config_data(filename), "r")))
    {
        while (count < MAXLVL && fgets(buf, MAXSTR, fin))
        {
            sscanf(buf, "%s %s %s %s %d %d %s",
                    level_v[count].file,
                    level_v[count].back,
                    level_v[count].shot,
                    level_v[count].grad,
                   &level_v[count].time,
                   &level_v[count].goal,
                    level_v[count].song);
            count++;
        }
        fclose(fin);
    }
}

static void level_init_hs(const char *filename)
{
    char buf[MAXSTR];
    FILE *fin;
    int i = 0;

    /* Set some sane values in case the scores file is missing. */

    for (i = 0; i < MAXLVL; i++)
    {
        strcpy(score_v[i].time_n[0], "Hard");
        strcpy(score_v[i].time_n[1], "Medium");
        strcpy(score_v[i].time_n[2], "Easy");

        score_v[i].time_t[0] = i ? 59999 : 359999;
        score_v[i].time_t[1] = i ? 59999 : 359999;
        score_v[i].time_t[2] = i ? 59999 : 359999;

        score_v[i].time_c[0] = 0;
        score_v[i].time_c[1] = 0;
        score_v[i].time_c[2] = 0;

        strcpy(score_v[i].coin_n[0], "Hard");
        strcpy(score_v[i].coin_n[1], "Medium");
        strcpy(score_v[i].coin_n[2], "Easy");

        score_v[i].coin_t[0] = i ? 59999 : 359999;
        score_v[i].coin_t[1] = i ? 59999 : 359999;
        score_v[i].coin_t[2] = i ? 59999 : 359999;

        score_v[i].coin_c[0] = 0;
        score_v[i].coin_c[1] = 0;
        score_v[i].coin_c[2] = 0;
    }

    /* Load the default high scores file. */

    if ((fin = fopen(config_data(filename), "r")))
    {
        for (i = 0; i < MAXLVL && fgets(buf, MAXSTR, fin); i++)
            sscanf(buf, "%d %d %d %d %d %d",
                   &score_v[i].time_t[0], &score_v[i].coin_c[0],
                   &score_v[i].time_t[1], &score_v[i].coin_c[1],
                   &score_v[i].time_t[2], &score_v[i].coin_c[2]);

        fclose(fin);
    }
}

/*---------------------------------------------------------------------------*/

const char *level_shot(int i)
{
    return level_v[i].shot;
}

const char *level_time_n(int i, int j)
{
    return score_v[i].time_n[j];
}

const char *level_coin_n(int i, int j)
{
    return score_v[i].coin_n[j];
}

/*---------------------------------------------------------------------------*/
/* Return the coin count for the Most Coins or Best Time score.              */

int level_coin_c(int i, int j)
{
    if (j < 0)
        return score;
    else
        return score_v[i].coin_c[j];
}

int level_time_c(int i, int j)
{
    return score_v[i].time_c[j];
}

/*---------------------------------------------------------------------------*/
/* Return the time for the Most Coins or Best Time score.                    */

int level_coin_t(int i, int j)
{
    return score_v[i].coin_t[j];
}

int level_time_t(int i, int j)
{
    if (j < 0)
        return level_v[i].time - curr_clock();
    else
        return score_v[i].time_t[j];
}

/*---------------------------------------------------------------------------*/

void level_init(const char *init_levels,
                const char *init_scores,
                const char *user_scores)
{
    memset(level_v, 0, sizeof (struct level) * MAXLVL);
    memset(score_v, 0, sizeof (struct score) * MAXLVL);

    level_init_rc(init_levels);
    level_init_hs(init_scores);
    level_load_hs(user_scores);

    strncpy(scores_file, user_scores, MAXSTR);

    level = 0;

}

void level_cheat(void)
/* Open each level of the set */
{
    limit = count;
}

void level_free(void)
{
    int i;

    level_store_hs(scores_file);

    for (i = 0; i < count; i++)
        if (glIsTexture(level_v[i].text))
            glDeleteTextures(1, &level_v[i].text);

    count = 0;
}

int level_exists(int i)
{
    return (0 < i && i < count);
}

int level_opened(int i)
{
    return level_exists(i) && (0 < i && i < count && i <= limit);
}

int level_locked(int i)
{
    return level_opened(i) && (i == limit) && (level_v[i].goal > 0);
}

/*---------------------------------------------------------------------------*/

int curr_times_total(void) { return times_total; }
int curr_coins_total(void) { return coins_total; }

int curr_count(void) { return count; }
int curr_score(void) { return score; }
int curr_coins(void) { return coins; }
int curr_balls(void) { return balls; }
int curr_level(void) { return level; }
int curr_goal (void) { return goal;  }

/*---------------------------------------------------------------------------*/

static int score_time_comp(const struct score *S, int i, int j)
{
    if (S->time_t[i] <  S->time_t[j])
        return 1;

    if (S->time_t[i] == S->time_t[j] &&
        S->time_c[i] >  S->time_c[j])
        return 1;

    return 0;
}

static int score_coin_comp(const struct score *S, int i, int j)
{
    if (S->coin_c[i] >  S->coin_c[j])
        return 1;

    if (S->coin_c[i] == S->coin_c[j] &&
        S->coin_t[i] <  S->coin_t[j])
        return 1;

    return 0;
}

/*---------------------------------------------------------------------------*/

static void score_time_swap(struct score *S, int i, int j)
{
    char n[MAXNAM];
    int  t;
    int  c;

    strncpy(n,            S->time_n[i], MAXNAM);
    strncpy(S->time_n[i], S->time_n[j], MAXNAM);
    strncpy(S->time_n[j], n,            MAXNAM);

    t            = S->time_t[i];
    S->time_t[i] = S->time_t[j];
    S->time_t[j] = t;

    c            = S->time_c[i];
    S->time_c[i] = S->time_c[j];
    S->time_c[j] = c;
}

static void score_coin_swap(struct score *S, int i, int j)
{
    char n[MAXNAM];
    int  t;
    int  c;

    strncpy(n,            S->coin_n[i], MAXNAM);
    strncpy(S->coin_n[i], S->coin_n[j], MAXNAM);
    strncpy(S->coin_n[j], n,            MAXNAM);

    t            = S->coin_t[i];
    S->coin_t[i] = S->coin_t[j];
    S->coin_t[j] = t;

    c            = S->coin_c[i];
    S->coin_c[i] = S->coin_c[j];
    S->coin_c[j] = c;
}

/*---------------------------------------------------------------------------*/

int level_replay(const char *filename)
{
    int r, time;
    status = GAME_NONE;

    r = demo_replay_init(filename, &score, &coins, &balls, &goal, &time);

    mode = (time == 0) ? MODE_PRACTICE : MODE_NORMAL;

    return r;
}

static int level_play_go(void)
/* Start to play the current level */
{
    int time;
    
    status = GAME_NONE;
    coins  = 0;
    goal   = (mode == MODE_PRACTICE) ? 0 : level_v[level].goal;
    time   = (mode == MODE_PRACTICE) ? 0 : level_v[level].time;
    
    return demo_play_init(USER_REPLAY_FILE,
                          level_v[level].file,
                          level_v[level].back,
                          level_v[level].grad,
                          level_v[level].song,
                          level_v[level].shot,
                          time,
                          goal, score, coins, balls);
}

int level_play(const char *filename, int i, int m)
/* Start to play a level sequence from the `i'th level */
{
    mode = m;
    level = i;

    score = 0;
    balls = 3;
    coins_total = 0;
    times_total = 0;

    return level_play_go();
}

/*---------------------------------------------------------------------------*/

void level_stat(int s)
{
    int time;
    if ((status = s) == GAME_GOAL)
    {
        coins_total += coins;
    }

    time = (mode == MODE_PRACTICE) ? curr_clock() : level_v[level].time - curr_clock();
    demo_play_stat(curr_coins(), time);
}

int level_dead(void)
{
    return (balls <= 0);
}

int level_last(void)
{
    return (level + 1 == count);
}

int level_exit(const char *filename, int next)
{
    times_total += level_v[level].time - curr_clock();

    demo_play_stop(filename);

    switch (status)
    {
    case GAME_GOAL:
        level++;
        if (limit < level)
            limit = level;

        level_store_hs(scores_file);
	if (!next) level--;
        break;

    case GAME_TIME:
    case GAME_FALL:
        if (mode == MODE_CHALLENGE)
	    balls--;
        break;
    }
    
    /* Load the next level. */

    if (status && level < count && balls >= 0)
	return level_play_go();

    return 0;
}

int level_sort(int *time_i, int *coin_i)
{
    int i, clock;
    char player[MAXNAM];
     
    if (mode == MODE_PRACTICE)
	clock = curr_clock();
    else
        clock = level_v[level].time - curr_clock();

    config_get_s(CONFIG_PLAYER, player, MAXNAM);

    /* Insert the time record into the high score list. */

    strncpy(score_v[level].time_n[3], player, MAXNAM);
    score_v[level].time_c[3] = coins;
    score_v[level].time_t[3] = clock;

    for (i = 2; i >= 0 && score_time_comp(score_v + level, i + 1, i); i--)
    {
        score_time_swap(score_v + level, i + 1, i);
        *time_i = i;
    }

    /* Insert the coin record into the high score list. */

    strncpy(score_v[level].coin_n[3], player, MAXNAM);
    score_v[level].coin_c[3] = coins;
    score_v[level].coin_t[3] = clock;

    for (i = 2; i >= 0 && score_coin_comp(score_v + level, i + 1, i); i--)
    {
        score_coin_swap(score_v + level, i + 1, i);
        *coin_i = i;
    }

    return (*time_i < 3 || *coin_i < 3);
}

int level_done(int *time_i, int *coin_i)
{
    int i;
    char player[MAXNAM];

    config_get_s(CONFIG_PLAYER, player, MAXNAM);

    /* Note a global high score. */

    strncpy(score_v[0].time_n[3], player, MAXNAM);
    score_v[0].time_c[3] = coins_total;
    score_v[0].time_t[3] = times_total;

    strncpy(score_v[0].coin_n[3], player, MAXNAM);
    score_v[0].coin_c[3] = coins_total;
    score_v[0].coin_t[3] = times_total;

    if (level == count)
    {
        /* Insert the time record into the global high score list. */

        for (i = 2; i >= 0 && score_time_comp(score_v, i + 1, i); i--)
        {
            score_time_swap(score_v, i + 1, i);
            *time_i = i;
        }

        /* Insert the coin record into the global high score list. */

        for (i = 2; i >= 0 && score_coin_comp(score_v, i + 1, i); i--)
        {
            score_coin_swap(score_v, i + 1, i);
            *coin_i = i;
        }
    }

    return (*time_i < 3 || *coin_i < 3);
}

int level_score(int n)
{
    int sound = AUD_COIN;
    int value = 0;

    coins += n;

    /* Pulse the coin counter based on the value of the grabbed coin. */

    if      (n >= 10) hud_coin_pulse(2.00f);
    else if (n >=  5) hud_coin_pulse(1.50f);
    else              hud_coin_pulse(1.25f);

    /* Check for goal open. */

    if (goal > 0)
    {
        if      (n >= 10) hud_goal_pulse(2.00f);
        else if (n >=  5) hud_goal_pulse(1.50f);
        else              hud_goal_pulse(1.25f);

        if (goal - n <= 0)
        {
            sound = AUD_SWITCH;
            value = 1;
            hud_goal_pulse(2.0f);
        }

        goal = (goal > n) ? (goal - n) : 0;
    }

    audio_play(sound, 1.f);
    return value;
}

int level_count(void)
{
    if (mode != MODE_CHALLENGE)
        return 0;
    if (coins > 0)
    {
        score++;
        coins--;

        if (score % 100 == 0)
        {
            balls += 1;
            audio_play(AUD_BALL, 1.0f);
        }
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void level_name(int i, const char *name, int time_i, int coin_i)
{
    strncpy(score_v[i].time_n[time_i], name, MAXNAM);
    strncpy(score_v[i].coin_n[coin_i], name, MAXNAM);
}

void level_snap(int i)
{
    char filename[MAXSTR];

    /* Convert the level name to a BMP filename. */

    memset(filename, 0, MAXSTR);
    strncpy(filename, level_v[i].file, strcspn(level_v[i].file, "."));
    strcat(filename, ".bmp");

    /* Initialize the game for a snapshot. */

    if (game_init(level_v[i].file, level_v[i].back, level_v[i].grad, 0, 1))
    {
        /* Render the level and grab the screen. */

        config_clear();
        game_set_fly(1.f);
        game_kill_fade();
        game_draw(1, 0);
        SDL_GL_SwapBuffers();

        image_snap(filename);
    }
}

/*---------------------------------------------------------------------------*/

int level_mode(void)
{
    return mode;
}

