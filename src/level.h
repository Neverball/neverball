#ifndef LEVEL_H
#define LEVEL_H

/*---------------------------------------------------------------------------*/

extern int score;
extern int balls;
extern int level;
extern int count;

void level_init(void);
void level_free(void);
void level_goto(int);
int  level_pass(void);
int  level_fail(void);

/*---------------------------------------------------------------------------*/

#endif
