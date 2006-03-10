#ifndef LEVEL_H
#define LEVEL_H

#include "base_config.h"
#define NSCORE  3

/*---------------------------------------------------------------------------*/

/* A score structure */

struct score
{
    char player[NSCORE+1][MAXNAM]; /* player name */
    int  timer[NSCORE+1];          /* time elapsed */
    int  coins[NSCORE+1];          /* coins collected */
};

void score_init_hs(struct score *, int, int);

/*---------------------------------------------------------------------------*/

/* A simple level */

struct level
{
    /* Level identity */
	
    char file[MAXSTR];    /* sol main file */
    char name[MAXSTR];    /* the level name */
    int  version;         /* the level version */
    char author[MAXSTR];  /* the author */
    
    /* Time and goal information */
    
    int time;             /* time limit */
    int goal;             /* coins needed */

    struct score time_score;  /* "best time" score */
    struct score goal_score;  /* "unlock goal" score */
    struct score coin_score;  /* "most coin" score */

    /* Regarding set information */
    
    struct set * set;       /* set (NULL in single mode) */
    int number;             /* level number in the set */
    char numbername[3];     /* string representation of the number (eg. B1) */
    int is_locked;          /* Is the level unplayable */
    int is_bonus;           /* Is the level an extra-bonus level? */ 
    int is_completed;       /* Is the level goal terminated? */ 
    int is_last;            /* Is the level the last of the set? */ 
    
    /* Other metadata (files are relative the data file) */
    
    char message[MAXSTR]; /* intro message */
    char back[MAXSTR];    /* sol background file */
    char grad[MAXSTR];    /* gradiant backgound image */
    char shot[MAXSTR];    /* screenshot image */
    char song[MAXSTR];    /* song file */
};

int level_load(const char *, struct level *);

void level_dump_info(const struct level *);

/*---------------------------------------------------------------------------*/

/* A level for the playing */

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
    int state;         /* state ending */
    int coins;         /* coins collected */
    int timer;         /* time elapsed */
    int state_value;   /* more precision about the state: skip for goal */

    int dead;          /* Is the game over and lost? */
    int win;           /* Is the game over and win? */
    
    /* rank = 3  => unclassed */
    int coin_rank;     /* rank in the level high-scores */
    int goal_rank;     /* rank in the level high-scores */
    int time_rank;     /* rank in the level high-scores */
    int score_rank;    /* rank in the set high-scores */
    int times_rank;    /* rank in the set high-scores */

    const struct level *next_level; /* next level (NULL no next level) */
};

/*---------------------------------------------------------------------------*/

#define MODE_CHALLENGE  1
#define MODE_NORMAL     2
#define MODE_PRACTICE   3
#define MODE_SINGLE     4

const char *mode_to_str(int);

/*---------------------------------------------------------------------------*/

#define GAME_NONE 0     /* No event (or aborted) */
#define GAME_TIME 1     /* Time's up */
#define GAME_GOAL 2	/* Goal reached */
#define GAME_FALL 3	/* Fall out */
#define GAME_SPEC 4	/* Special goal reached */

const char *state_to_str(int);

#endif
