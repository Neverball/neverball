#ifndef BACK_H
#define BACK_H

/*---------------------------------------------------------------------------*/

#define BACK_DIST  500.0
#define FAR_DIST  1000.0

void back_init(const char *, int);
void back_free(void);
void back_draw(int, double);

/*---------------------------------------------------------------------------*/

#endif
