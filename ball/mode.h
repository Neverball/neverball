#ifndef MODE_H
#define MODE_H

/*---------------------------------------------------------------------------*/

#define MODE_CHALLENGE  1
#define MODE_NORMAL     2
#define MODE_PRACTICE   3

void mode_set(int);
int  curr_mode(void);

const char *mode_to_str(int, int);

/*---------------------------------------------------------------------------*/

#endif
