#ifndef LEVEL_H
#define LEVEL_H

#include "base_config.h"

/*---------------------------------------------------------------------------*/

/* A simple level */

struct level
{
    /* (data) means that the file is relative from the data file */
    char file[MAXSTR];    /* sol main file */
    char back[MAXSTR];    /* sol background file (data) */
    char grad[MAXSTR];    /* gradiant backgound image (data) */
    char shot[MAXSTR];    /* screenshot image (data)*/
    char song[MAXSTR];    /* song file (data) */
    int  time;            /* time limit */
    int  goal;            /* coins needed */
};

int level_load(const char *, struct level *);

void level_dump_info(const struct level *);

/*---------------------------------------------------------------------------*/

/* A level for the playing */

struct level_game
{
    int mode;          /* game mode */
    int level;         /* level id of the set */

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
};

/*---------------------------------------------------------------------------*/

#define MODE_CHALLENGE  1
#define MODE_NORMAL     2
#define MODE_PRACTICE   3
#define MODE_SINGLE     4

const char *mode_to_str(int);

/*---------------------------------------------------------------------------*/

#define GAME_NONE 0
#define GAME_TIME 1
#define GAME_GOAL 2
#define GAME_FALL 3

const char *state_to_str(int);

#endif
