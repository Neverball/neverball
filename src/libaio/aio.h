#ifndef AIO_H
#define AIO_H

int    aio_init(void);
void   aio_fini(void);

short *aio_load(const char *, int *);
int    aio_play(short *, int, float);
int    aio_loop(short *, int, float);
void   aio_freq(int, float);
void   aio_ampl(int, float);

#endif
