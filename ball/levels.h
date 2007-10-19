#ifndef LEVELS_H
#define LEVELS_H

/*---------------------------------------------------------------------------*/

struct level_game
{
    int mode;          /* game mode */
    const struct level *level; /* the level played */

    int goal;          /* coins needed */
    int time;          /* time limit */

    /* MODE_CHALLENGE only */
    int score;         /* coin total */
    int balls;         /* live count */
    int times;         /* time total */

    /* Once a level is finished */
    int status;        /* status ending */
    int coins;         /* coins collected */
    int timer;         /* time elapsed */

    /* rank = 3  => unclassed */
    int coin_rank;     /* rank in the level high-scores */
    int goal_rank;     /* rank in the level high-scores */
    int time_rank;     /* rank in the level high-scores */
    int score_rank;    /* rank in the set high-scores */
    int times_rank;    /* rank in the set high-scores */

    /* What about the game and the set? */
    int dead;          /* Is the game over and lost? */
    int win;           /* Is the game over and win? */
    int unlock;        /* Is the next level newly unlocked */
    const struct level *next_level; /* next level (NULL no next level) */
};

struct level_game *curr_lg(void);

/*---------------------------------------------------------------------------*/

#endif
