#ifndef TEXT_H
#define TEXT_H

/*---------------------------------------------------------------------------*/

#define TXT_COINS 0
#define TXT_BALLS 1

void text_init(void);
void text_free(void);

void text_digit(int, double, double, double, double);
void text_label(int, double, double, double, double);

/*---------------------------------------------------------------------------*/

#endif
