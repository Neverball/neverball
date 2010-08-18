#ifndef LEVEL_H
#define LEVEL_H

#include "base_config.h"
#include "score.h"
#include "progress.h"

/*---------------------------------------------------------------------------*/

enum
{
    SCORE_BEST_TIMES = 0,
    SCORE_FAST_UNLOCK,
    SCORE_MOST_COINS
};

struct level
{
    /* TODO: turn into an internal structure. */

    char file[PATHMAX];
    char shot[PATHMAX];
    char song[PATHMAX];

    char message[MAXSTR];

    char version[MAXSTR];
    char author[MAXSTR];

    int time; /* Time limit   */
    int goal; /* Coins needed */

    struct score scores[3];

    /* Set information. */

    int  number;

    /* String representation of the number (eg. "IV") */
    char name[MAXSTR];

    int is_locked;
    int is_bonus;
    int is_completed;
};

int  level_load(const char *, struct level *);

/*---------------------------------------------------------------------------*/

int  level_exists(int);

void level_open  (int);
int  level_opened(int);

void level_complete (int);
int  level_completed(int);

int  level_time(int);
int  level_goal(int);
int  level_bonus(int);

const char *level_shot(int);
const char *level_file(int);
const char *level_name(int);
const char *level_msg (int);

const struct score *level_score(int, int);

/*---------------------------------------------------------------------------*/

int  level_score_update (int, int, int, int *, int *, int *);
void level_rename_player(int, int, int, int, const char *);

/*---------------------------------------------------------------------------*/

#endif
