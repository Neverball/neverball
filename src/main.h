#ifndef MAIN_H
#define MAIN_H

#include <SDL.h>

/*---------------------------------------------------------------------------*/

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xFF000000
#define GMASK 0x00FF0000
#define BMASK 0x0000FF00
#define AMASK 0x000000FF
#else
#define RMASK 0x000000FF
#define GMASK 0x0000FF00
#define BMASK 0x00FF0000
#define AMASK 0xFF000000
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

/*---------------------------------------------------------------------------*/

extern int main_mode;
extern int main_width;
extern int main_height;
extern int main_geom;
extern int main_scale;
extern int main_rate;
extern int main_buff;
extern int main_fps;

#define JOY_MAX 32767
#define JOY_MID 16383

extern int joy_axis_x;
extern int joy_axis_y;
extern int joy_button_r;
extern int joy_button_l;
extern int joy_button_a;
extern int joy_button_b;
extern int joy_button_pause;

void conf_store(void);
int set_mode(int, int, int);

/*---------------------------------------------------------------------------*/

#endif
