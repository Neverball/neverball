/*
 * Copyright (C) 2007 Robert Kooima
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

#include <string.h>
#include "score.h"
#include "common.h"

/*---------------------------------------------------------------------------*/

static int score_time_comp(const struct score *S, int i, int j)
{
    if (S->timer[i] < S->timer[j])
        return 1;

    if (S->timer[i] == S->timer[j] && S->coins[i] > S->coins[j])
        return 1;

    return 0;
}

static int score_coin_comp(const struct score *S, int i, int j)
{
    if (S->coins[i] > S->coins[j])
        return 1;

    if (S->coins[i] == S->coins[j] && S->timer[i] < S->timer[j])
        return 1;

    return 0;
}

static void score_swap(struct score *S, int i, int j)
{
    char player[MAXNAM];
    int  tmp;

    SAFECPY(player,       S->player[i]);
    SAFECPY(S->player[i], S->player[j]);
    SAFECPY(S->player[j], player);

    tmp         = S->timer[i];
    S->timer[i] = S->timer[j];
    S->timer[j] = tmp;

    tmp         = S->coins[i];
    S->coins[i] = S->coins[j];
    S->coins[j] = tmp;
}

/*---------------------------------------------------------------------------*/

static void score_insert(struct score *s, int i,
                         const char *player, int timer, int coins)
{
    SAFECPY(s->player[i], player);

    s->timer[i] = timer;
    s->coins[i] = coins;
}

void score_init_hs(struct score *s, int timer, int coins)
{
    score_insert(s, RANK_HARD, "Hard",   timer, coins);
    score_insert(s, RANK_MEDM, "Medium", timer, coins);
    score_insert(s, RANK_EASY, "Easy",   timer, coins);
    score_insert(s, RANK_LAST, "",       timer, coins);
}

void score_time_insert(struct score *s, int *rank,
                       const char *player, int timer, int coins)
{
    int i;

    score_insert(s, RANK_LAST, player, timer, coins);

    if (rank)
    {
        for (i = RANK_EASY; i >= RANK_HARD && score_time_comp(s, i + 1, i); i--)
            score_swap(s, i + 1, i);

        *rank = i + 1;
    }
}

void score_coin_insert(struct score *s, int *rank,
                       const char *player, int timer, int coins)
{
    int i;

    score_insert(s, RANK_LAST, player, timer, coins);

    if (rank)
    {
        for (i = RANK_EASY; i >= RANK_HARD && score_coin_comp(s, i + 1, i); i--)
            score_swap(s, i + 1, i);

        *rank = i + 1;
    }
}

/*---------------------------------------------------------------------------*/
