#ifndef MAIN_H
#define MAIN_H

#include <SDL/SDL.h>

/*---------------------------------------------------------------------------*/

#define MIN(a, b) ((a) < (b) ? (a) : (b))

extern int main_mode;
extern int main_width;
extern int main_height;
extern int main_geom;
extern int main_scale;
extern int main_rate;
extern int main_buff;

int set_mode(int, int, int);

/*---------------------------------------------------------------------------*/

#endif
