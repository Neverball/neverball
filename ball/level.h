#ifndef LEVEL_H
#define LEVEL_H

/*---------------------------------------------------------------------------*/

const char *level_shot(int);
const char *level_coin_n(int, int);
const char *level_time_n(int, int);
int         level_coin_c(int, int);
int         level_time_c(int, int);
int         level_coin_t(int, int);
int         level_time_t(int, int);

void level_init(char *, char *, char *);
void level_free(char *);

int  level_exists(int);
int  level_opened(int);

int  curr_count(void);
int  curr_score(void);
int  curr_coins(void);
int  curr_balls(void);
int  curr_level(void);

int  level_goto(int, int, int, int);
int  level_goal(void);
int  level_pass(void);
int  level_fail(void);

void level_score(int);
void level_snap(int);
void level_song(void);

/*---------------------------------------------------------------------------*/

#endif
