#ifndef BINARY_H
#define BINARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

#define FLOAT_BYTES     4
#define INDEX_BYTES     4
#define SHORT_BYTES     2
#define ARRAY_BYTES(n)  (FLOAT_BYTES * (n))
#define STRING_BYTES(s) (strlen(s) + 1)

void put_float(FILE *, const float *);
void put_index(FILE *, const int   *);
void put_short(FILE *, const short *);
void put_array(FILE *, const float *, size_t);

void get_float(FILE *, float *);
void get_index(FILE *, int   *);
void get_short(FILE *, short *);
void get_array(FILE *, float *, size_t);

void put_string(FILE *fout, const char *);
void get_string(FILE *fin, char *, int );

/*---------------------------------------------------------------------------*/

#endif
