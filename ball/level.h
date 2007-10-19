#ifndef LEVEL_H
#define LEVEL_H

#include "base_config.h"
#include "score.h"
#include "levels.h"

/*---------------------------------------------------------------------------*/

struct level
{
    char file[PATHMAX];
    char back[PATHMAX];
    char grad[PATHMAX];
    char shot[PATHMAX];
    char song[PATHMAX];

    char message[MAXSTR];

    char version[MAXSTR];
    char author[MAXSTR];

    int time; /* Time limit   */
    int goal; /* Coins needed */

    struct
    {
        struct score best_times;
        struct score unlock_goal;
        struct score most_coins;
    }
    score;

    /* Set information. */

    struct set *set;

    int  number;

    /* String representation of the number (eg. "IV") */
    char repr[MAXSTR];

    int is_locked;
    int is_bonus;
    int is_completed;
};

int  level_load(const char *, struct level *);
void level_dump(const struct level *);

int  level_replay(const char *);
int  level_play(const struct level *, int);
void level_stat(int, int, int);
void level_stop(void);
int  level_next(void);
int  level_same(void);

/*---------------------------------------------------------------------------*/

int count_extra_balls(int, int);

void level_update_player_name(void);

/*---------------------------------------------------------------------------*/

#define GAME_NONE 0     /* No event (or aborted) */
#define GAME_TIME 1     /* Time's up */
#define GAME_GOAL 2     /* Goal reached */
#define GAME_FALL 3     /* Fall out */

const char *status_to_str(int);

#endif
