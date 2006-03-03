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
#include "levels.h"
#include "image.h"
#include "game.h"
#include "geom.h"
#include "demo.h"
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


static int count;                       /* Number of levels           */
static int limit;                       /* Last opened (locked) level */

static struct level level_v[MAXLVL];
static struct score score_v[MAXLVL];

static char scores_file[MAXSTR];

static struct level_game current_level_game;

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
    char name[MAXSTR];

    count = 0;

    /* Load the levels list. */

    if ((fin = fopen(config_data(filename), "r")))
    {
        while (count < MAXLVL && fgets(buf, MAXSTR, fin))
        {
            sscanf(buf, "%s %s %s %s %d %d %s",
                    name,
                    level_v[count].back,
                    level_v[count].shot,
                    level_v[count].grad,
                   &level_v[count].time,
                   &level_v[count].goal,
                    level_v[count].song);
	    level_load(config_data(name), &level_v[count]);
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
}

void level_cheat(void)
/* Open each level of the set */
{
    limit = count;
}

void level_free(void)
{
    level_store_hs(scores_file);
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
    return level_opened(i) && (i == limit);
}

int level_extra_bonus(int i)
{
    return level_exists(i) && (i > 20);
}

int level_extra_bonus_opened(void)
{
    return level_opened(21);
}

int level_set_completed(void)
{
    return limit >= count;
}

static const char * names[] = {"1", "2", "3", "4", "5",
       "6", "7", "8", "9", "10",
       "11", "12", "13", "14", "15",
       "16", "17", "18", "19", "20",
       N_("B1"), N_("B2"), N_("B3"), N_("B4"), N_("B5")};

const char * level_number_name(i)
/* Return the number name of the level i */
{
    return names[i-1];
}

/*---------------------------------------------------------------------------*/

int curr_count(void) { return count; }

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
    return demo_replay_init(filename, &current_level_game); 
}


int level_play_go(void)
/* Start to play the current level */
{
    struct level_game *lg = &current_level_game;
    int mode  = lg->mode;
    struct level *l = &level_v[lg->level];
    
    lg->goal = (mode == MODE_PRACTICE) ? 0 : l->goal;
    lg->time = (mode == MODE_PRACTICE) ? 0 : l->time;
    
    return demo_play_init(USER_REPLAY_FILE, l, lg);
}

void level_play_single(const char *filename)
/* Prepare to play a single level */
{
    int level = 0;
    level_init("", "", "");
    count = 1;
    
    current_level_game.mode  = MODE_SINGLE;
    current_level_game.level = level;
    
    strncpy(level_v[0].file, filename, MAXSTR);
    level_v[level].back[0] = '\0';
    level_v[level].grad[0] = '\0';
    level_v[level].song[0] = '\0';
    level_v[level].shot[0] = '\0';
    level_v[level].goal    = 0;
    level_v[level].time    = 0;
}

void level_play(int i, int m)
/* Prepare to play a level sequence from the `i'th level */
{
    current_level_game.mode = m;
    current_level_game.level = i;

    current_level_game.score = 0;
    current_level_game.balls = 3;
    current_level_game.times = 0;
}

/*---------------------------------------------------------------------------*/

const struct level_game * curr_lg(void)
{
    return &current_level_game;
}

int count_extra_balls(int old_score, int coins)
{
    int modulo = old_score % 100;
    int sum    = modulo + coins;
    return sum / 100;
}

void level_stop(int state, int clock, int coins)
/* Stop the current playing level */
{
    struct level_game * lg = &current_level_game;
    int mode = lg->mode;
    int level = lg->level;
    int timer = (mode == MODE_PRACTICE || mode == MODE_SINGLE) ? clock : lg->time - clock;

    lg->state = state;
    lg->coins = coins;
    lg->timer = timer;
   
    /* open next level */
    if (state == GAME_GOAL && mode != MODE_PRACTICE && mode != MODE_SINGLE && limit < level+1)
	if (level_extra_bonus_opened() || !level_extra_bonus(level+1) || mode == MODE_CHALLENGE)
	    limit = level + 1;
    
    if (mode == MODE_CHALLENGE)
    {
	/* sum time */
	lg->times += timer; 
	    
	/* sum coins an earn extra balls */
	if (state == GAME_GOAL)
	{
	    lg->balls += count_extra_balls(lg->score, coins);
	    lg->score += coins;
	}

	/* lose ball */
        if (state == GAME_TIME || state == GAME_FALL)
	    lg->balls--;
    }

    /* stop demo recording */	
    demo_play_stop(lg);
}

int level_dead(void)
{
    int mode = current_level_game.mode;
    int balls = current_level_game.balls;
    return (mode == MODE_CHALLENGE) && (balls <= 0);
}

int level_last(void)
{
    int level = current_level_game.level;
    return (level + 1 == count) || (level_extra_bonus(level + 1));
}

void level_next(void)
{
    current_level_game.level++;
}

int level_sort(int *time_i, int *coin_i)
{
    int i, timer, coins, level;
    char player[MAXNAM];
    
    coins = current_level_game.coins;
    timer = current_level_game.timer;
    level = current_level_game.level;
    
    config_get_s(CONFIG_PLAYER, player, MAXNAM);

    /* Insert the time record into the high score list. */

    strncpy(score_v[level].time_n[3], player, MAXNAM);
    score_v[level].time_c[3] = coins;
    score_v[level].time_t[3] = timer;

    for (i = 2; i >= 0 && score_time_comp(score_v + level, i + 1, i); i--)
    {
        score_time_swap(score_v + level, i + 1, i);
        *time_i = i;
    }

    /* Insert the coin record into the high score list. */

    strncpy(score_v[level].coin_n[3], player, MAXNAM);
    score_v[level].coin_c[3] = coins;
    score_v[level].coin_t[3] = timer;

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
    int score = current_level_game.score;
    int times = current_level_game.times;

    config_get_s(CONFIG_PLAYER, player, MAXNAM);

    /* Note a global high score. */

    strncpy(score_v[0].time_n[3], player, MAXNAM);
    score_v[0].time_c[3] = score;
    score_v[0].time_t[3] = times;

    strncpy(score_v[0].coin_n[3], player, MAXNAM);
    score_v[0].coin_c[3] = score;
    score_v[0].coin_t[3] = times;

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

    return (*time_i < 3 || *coin_i < 3);
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

    if (game_init(&level_v[i], 0, 0))
    {
        /* Render the level and grab the screen. */

        config_clear();
        game_set_fly(1.f);
        game_kill_fade();
        game_draw(1, 0);
        SDL_GL_SwapBuffers();

        image_snap(filename, config_get_d(CONFIG_WIDTH), config_get_d(CONFIG_HEIGHT));
    }
}

/*---------------------------------------------------------------------------*/

