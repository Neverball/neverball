#ifndef BACK_H
#define BACK_H

/*---------------------------------------------------------------------------*/

#define BACK_DIST  500.f
#define FAR_DIST  1000.f

void back_init(const char *, int);
void back_free(void);
void back_draw(int, float);

/*---------------------------------------------------------------------------*/

#endif
