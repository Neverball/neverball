#ifndef BACK_H
#define BACK_H

/*---------------------------------------------------------------------------*/

#define BACK_DIST 100.0

#ifdef SKYBOX
void back_init(const char *);
void back_free(void);
void back_draw(void);
#else
void back_init(const char *, int);
void back_free(void);
void back_draw(void);
#endif

/*---------------------------------------------------------------------------*/

#endif
