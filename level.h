#ifndef LEVEL_H
#define LEVEL_H

/*---------------------------------------------------------------------------*/

#define LEVEL_FILE "levels.txt"

void level_init(void);
void level_free(void);

int  level_exists(int);
int  level_opened(int);

int  curr_level(void);
int  curr_balls(void);
int  curr_coins(void);

void level_goto(int);
int  level_pass(void);
int  level_fail(void);

void level_score(int);
void level_shot(int);

/*---------------------------------------------------------------------------*/

#endif
