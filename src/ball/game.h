#ifndef GAME_H
#define GAME_H

#include <sol.h>

/*---------------------------------------------------------------------------*/

void game_init(void);
void game_step(double);

void game_paint(void);
int  game_point(int, int);
int  game_click(int);
int  game_keybd(int);

/*---------------------------------------------------------------------------*/

#endif
