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
#include "hud.h"
#include "back.h"
#include "audio.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

struct score
{
    char   time_n[4][MAXNAM];
    double time_s[4];
    int    time_c[4];

    char   coin_n[4][MAXNAM];
    double coin_s[4];
    int    coin_c[4];
};

struct level
{
    char   file[MAXSTR];
    char   back[MAXSTR];
    char   shot[MAXSTR];
    char   song[MAXSTR];
    int    time;

    GLuint text;
};

static int score;                       /* Current level score       */
static int coins;                       /* Current coin count        */
static int balls;                       /* Current life count        */
static int level;                       /* Current level number      */
static int count;                       /* Number of levels          */
static int limit;                       /* First unopened level      */

static struct level level_v[MAXLVL];
static struct score score_v[MAXLVL];

char player[MAXNAM] = DEFAULT_NAME;

/*---------------------------------------------------------------------------*/

static void level_store_hs(const char *filename)
{
    char  file[MAXSTR];
    FILE *fout;

    if (config_home(file, filename, MAXSTR))
        if ((fout = fopen(file, "w")))
        {
            int i;

            for (i = 1; i < limit; i++)
            {
                fprintf(fout, "%5.2f %d %s\n",
                        score_v[i].time_s[0],
                        score_v[i].time_c[0],
                        score_v[i].time_n[0]);
                fprintf(fout, "%5.2f %d %s\n",
                        score_v[i].coin_s[0],
                        score_v[i].coin_c[0],
                        score_v[i].coin_n[0]);
                fprintf(fout, "%5.2f %d %s\n",
                        score_v[i].time_s[1],
                        score_v[i].time_c[1],
                        score_v[i].time_n[1]);
                fprintf(fout, "%5.2f %d %s\n",
                        score_v[i].coin_s[1],
                        score_v[i].coin_c[1],
                        score_v[i].coin_n[1]);
                fprintf(fout, "%5.2f %d %s\n",
                        score_v[i].time_s[2],
                        score_v[i].time_c[2],
                        score_v[i].time_n[2]);
                fprintf(fout, "%5.2f %d %s\n",
                        score_v[i].coin_s[2],
                        score_v[i].coin_c[2],
                        score_v[i].coin_n[2]);
            }
            
            fclose(fout);
        }
 }

static void level_load_hs(const char *filename)
{
    char  file[MAXSTR];
    FILE *fin;

    limit = 2;

    if (config_home(file, filename, MAXSTR))
        if ((fin = fopen(file, "r")))
        {
            int i;

            for (i = 1; i < count; i++)
                if (fscanf(fin, "%lf %d %s",
                           &score_v[i].time_s[0],
                           &score_v[i].time_c[0],
                            score_v[i].time_n[0]) == 3 &&
                    fscanf(fin, "%lf %d %s",
                           &score_v[i].coin_s[0],
                           &score_v[i].coin_c[0],
                            score_v[i].coin_n[0]) == 3 &&
                    fscanf(fin, "%lf %d %s",
                           &score_v[i].time_s[1],
                           &score_v[i].time_c[1],
                            score_v[i].time_n[1]) == 3 &&
                    fscanf(fin, "%lf %d %s",
                           &score_v[i].coin_s[1],
                           &score_v[i].coin_c[1],
                            score_v[i].coin_n[1]) == 3 &&
                    fscanf(fin, "%lf %d %s",
                           &score_v[i].time_s[2],
                           &score_v[i].time_c[2],
                            score_v[i].time_n[2]) == 3 &&
                    fscanf(fin, "%lf %d %s",
                           &score_v[i].coin_s[2],
                           &score_v[i].coin_c[2],
                            score_v[i].coin_n[2]) == 3)
                    limit = i + 1;

            fclose(fin);
        }
}

/*---------------------------------------------------------------------------*/

static void level_init_rc(const char *filename)
{
    FILE *fin = fopen(filename, "r");
    char buf[MAXSTR];

    count = 0;
    level = 0;
    coins = 0;
    score = 0;
    balls = 0;

    /* Load the levels list. */

    if (fin)
    {
        while (count < MAXLVL && fgets(buf, MAXSTR, fin))
        {
            sscanf(buf, "%s %s %s %d %s",
                    level_v[count].file,
                    level_v[count].shot,
                    level_v[count].back,
                   &level_v[count].time,
                    level_v[count].song);
            count++;
        }
        fclose(fin);
    }
}

static void level_init_hs(const char *filename)
{
    FILE *fin = fopen(filename, "r");
    char buf[MAXSTR];
    int i = 0;

    /* Load the default high scores list. */

    if (fin)
    {
        while (i < MAXLVL && fgets(buf, MAXSTR, fin))
        {
            sscanf(buf,
                   "%lf %s %d %s"
                   "%lf %s %d %s"
                   "%lf %s %d %s",
                   &score_v[i].time_s[0], score_v[i].time_n[0],
                   &score_v[i].coin_c[0], score_v[i].coin_n[0],
                   &score_v[i].time_s[1], score_v[i].time_n[1],
                   &score_v[i].coin_c[1], score_v[i].coin_n[1],
                   &score_v[i].time_s[2], score_v[i].time_n[2],
                   &score_v[i].coin_c[2], score_v[i].coin_n[2]);

            score_v[i].time_c[0] = 1;
            score_v[i].time_c[1] = 2;
            score_v[i].time_c[2] = 3;

            score_v[i].coin_s[0] = 599.99;
            score_v[i].coin_s[1] = 599.99;
            score_v[i].coin_s[2] = 599.99;;

            i++;
        }

        fclose(fin);
    }
}

/*---------------------------------------------------------------------------*/

static void sprint_time(char *buf, double t)
{
    int m = (int) floor(t) / 60;
    int s = (int) floor(t) % 60;
    int h = (int) (100.0 * (t - m * 60 - s));

    sprintf(buf, "%d:%02d.%02d", m, s, h);
}

/*---------------------------------------------------------------------------*/

const char *level_coin_n(int i, int j)
{
    return score_v[i].coin_n[j];
}

const char *level_coin_c(int i, int j)
{
    static char buf[MAXSTR];

    if (j < 0)
        sprintf(buf, "%02d", score);
    else
        sprintf(buf, "%02d", score_v[i].coin_c[j]);

    return buf;
}

const char *level_coin_s(int i, int j)
{
    static char buf[MAXSTR];

    sprint_time(buf, score_v[i].coin_s[j]);

    return buf;
}

/*---------------------------------------------------------------------------*/

const char *level_time_n(int i, int j)
{
    return score_v[i].time_n[j];
}

const char *level_time_c(int i, int j)
{
    static char buf[MAXSTR];

    sprintf(buf, "%02d", score_v[i].time_c[j]);
    return buf;
}

const char *level_time_s(int i, int j)
{
    static char buf[MAXSTR];

    if (j < 0)
        sprint_time(buf, level_v[i].time - curr_clock());
    else
        sprint_time(buf, score_v[i].time_s[j]);

    return buf;
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
}

void level_free(const char *user_scores)
{
    int i;

    level_store_hs(user_scores);

    for (i = 0; i < count; i++)
        if (glIsTexture(level_v[i].text))
            glDeleteTextures(1, &level_v[i].text);

    game_free();
    back_free();

    count = 0;
}

int level_exists(int i)
{
    return (0 < i && i < count);
}

int level_opened(int i)
{
    return (0 < i && i < count && i < limit);
}

/*---------------------------------------------------------------------------*/

int curr_count(void) { return count; }
int curr_score(void) { return score; }
int curr_coins(void) { return coins; }
int curr_balls(void) { return balls; }
int curr_level(void) { return level; }

/*---------------------------------------------------------------------------*/

static int time_comp(double t1, double t2)
{
    int h1 = (int) floor(100.0 * t1);
    int h2 = (int) floor(100.0 * t2);

    if (h1 < h2) return -1;
    if (h1 > h2) return +1;

    return 0;
}

static int score_time_comp(const struct score *S, int i, int j)
{
    if (time_comp(S->time_s[i], S->time_s[j]) <  0)
        return 1;

    if (time_comp(S->time_s[i], S->time_s[j]) == 0 &&
        S->time_c[i] >  S->time_c[j])
        return 1;

    return 0;
}

static int score_coin_comp(const struct score *S, int i, int j)
{
    if (S->coin_c[i] >  S->coin_c[j])
        return 1;

    if (S->coin_c[i] == S->coin_c[j] &&
        time_comp(S->coin_s[i], S->coin_s[j]) < 0)
        return 1;

    return 0;
}

/*---------------------------------------------------------------------------*/

static void score_time_swap(struct score *S, int i, int j)
{
    char   n[MAXNAM];
    double s;
    int    c;

    strncpy(n,            S->time_n[i], MAXNAM);
    strncpy(S->time_n[i], S->time_n[j], MAXNAM);
    strncpy(S->time_n[j], n,            MAXNAM);

    s            = S->time_s[i];
    S->time_s[i] = S->time_s[j];
    S->time_s[j] = s;

    c            = S->time_c[i];
    S->time_c[i] = S->time_c[j];
    S->time_c[j] = c;
}

static void score_coin_swap(struct score *S, int i, int j)
{
    char   n[MAXNAM];
    double s;
    int    c;

    strncpy(n,            S->coin_n[i], MAXNAM);
    strncpy(S->coin_n[i], S->coin_n[j], MAXNAM);
    strncpy(S->coin_n[j], n,            MAXNAM);

    s            = S->coin_s[i];
    S->coin_s[i] = S->coin_s[j];
    S->coin_s[j] = s;

    c            = S->coin_c[i];
    S->coin_c[i] = S->coin_c[j];
    S->coin_c[j] = c;
}

/*---------------------------------------------------------------------------*/

void level_goto(int s, int c, int b, int l)
{
    if (s >= 0) score = s;
    if (c >= 0) coins = c;
    if (b >= 0) balls = b;
    if (l >= 0) level = l;

    back_init(level_v[level].back, config_geom());
    game_init(level_v[level].file,
              level_v[level].time);
}

int level_goal(void)
{
    score_v[level].time_c[3] = curr_score();
    score_v[level].coin_c[3] = curr_score();

    score_v[level].time_s[3] = level_v[level].time - curr_clock();
    score_v[level].coin_s[3] = level_v[level].time - curr_clock();

    if (score_time_comp(score_v + level, 3, 2)) return 1;
    if (score_coin_comp(score_v + level, 3, 2)) return 1;

    return 0;
}

int level_pass(void)
{
    double clock = level_v[level].time - curr_clock();
    int i;
    
    if (strlen(player) == 0)
        strcpy(player, DEFAULT_NAME);

    /* Insert the time record into the high score list. */

    strncpy(score_v[level].time_n[3], player, MAXNAM);
    score_v[level].time_c[3] = score;
    score_v[level].time_s[3] = clock;

    for (i = 2; i >= 0 && score_time_comp(score_v + level, i + 1, i); i--)
        score_time_swap(score_v + level, i + 1, i);

    /* Insert the coin record into the high score list. */

    strncpy(score_v[level].coin_n[3], player, MAXNAM);
    score_v[level].coin_c[3] = score;
    score_v[level].coin_s[3] = clock;

    for (i = 2; i >= 0 && score_coin_comp(score_v + level, i + 1, i); i--)
        score_coin_swap(score_v + level, i + 1, i);

    /* Load the next level. */

    if (++level < count)
    {
        game_free();
        back_free();
        score = 0;

        back_init(level_v[level].back, config_geom());
        game_init(level_v[level].file,
                  level_v[level].time);

        return 1;
    }
    else
    {
        limit = count;

        ball_free();
        ball_init(config_geom());
    }

    return 0;
}

int level_fail(void)
{
    if (--balls >= 0)
    {
        game_free();
        back_free();
        score = 0;

        back_init(level_v[level].back, config_geom());
        game_init(level_v[level].file,
                  level_v[level].time);

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void level_score(int n)
{
    double k = 1.25;

    coins += n;
    score += n;

    if (n >=  5) k = 1.5;
    if (n >= 10) k = 2.0;

    hud_coin_pulse(k);

    if (coins >= 100)
    {
        coins -= 100;
        balls += 1;
        audio_play(AUD_BALL, 1.0f);
        hud_ball_pulse(2.0);

        limit = (level + 1 > limit) ? level + 1 : limit;
    }
    else
        audio_play(AUD_COIN, 1.0f);
}

void level_shot(int i)
{
    if (0 <= i && i < count)
    {
        if (!glIsTexture(level_v[i].text))
            level_v[i].text = make_image_from_file(0, 0, level_v[i].shot);

        glBindTexture(GL_TEXTURE_2D, level_v[i].text);
    }
}

void level_song(void)
{
    audio_music_play(level_v[level].song);
}

