#ifndef SCORE_H
#define SCORE_H

#include "base_config.h"

/*---------------------------------------------------------------------------*/

enum
{
    RANK_MIN = -1,

    RANK_HARD,
    RANK_MEDM,
    RANK_EASY,
    RANK_LAST,

    RANK_MAX
};

struct score
{
    char player[RANK_MAX][MAXNAM];

    int  timer[RANK_MAX];               /* Time elapsed                      */
    int  coins[RANK_MAX];               /* Coins collected                   */
};

/*---------------------------------------------------------------------------*/

void score_init_hs(struct score *, int, int);

void score_time_insert(struct score *, int *, const char *, int, int);
void score_coin_insert(struct score *, int *, const char *, int, int);

/*---------------------------------------------------------------------------*/

#endif
