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

    strncpy(player,       S->player[i], MAXNAM);
    strncpy(S->player[i], S->player[j], MAXNAM);
    strncpy(S->player[j], player,       MAXNAM);

    tmp         = S->timer[i];
    S->timer[i] = S->timer[j];
    S->timer[j] = tmp;

    tmp         = S->coins[i];
    S->coins[i] = S->coins[j];
    S->coins[j] = tmp;
}

/*---------------------------------------------------------------------------*/

void score_init_hs(struct score *s, int timer, int coins)
{
    int i;

    strcpy(s->player[0], "Hard");
    strcpy(s->player[1], "Medium");
    strcpy(s->player[2], "Easy");
    strcpy(s->player[3], "");

    for (i = 0; i < NSCORE + 1; i++)
    {
        s->timer[i] = timer;
        s->coins[i] = coins;
    }
}

int score_time_insert(struct score *s, const char *player, int timer, int coins)
{
    int i;

    strncpy(s->player[3], player, MAXNAM);
    s->timer[3] = timer;
    s->coins[3] = coins;

    for (i = 2; i >= 0 && score_time_comp(s, i + 1, i); i--)
        score_swap(s, i + 1, i);

    return i + 1;
}

int score_coin_insert(struct score *s, const char *player, int timer, int coins)
{
    int i;

    strncpy(s->player[3], player, MAXNAM);
    s->timer[3] = timer;
    s->coins[3] = coins;

    for (i = 2; i >= 0 && score_coin_comp(s, i + 1, i); i--)
        score_swap(s, i + 1, i);

    return i + 1;
}

/*---------------------------------------------------------------------------*/
