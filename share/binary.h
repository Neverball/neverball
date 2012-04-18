#ifndef BINARY_H
#define BINARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"

/*---------------------------------------------------------------------------*/

#define FLOAT_BYTES     4
#define INDEX_BYTES     4
#define SHORT_BYTES     2
#define ARRAY_BYTES(n)  (FLOAT_BYTES * (n))
#define STRING_BYTES(s) (strlen(s) + 1)

void put_float(fs_file, float);
void put_index(fs_file, int);
void put_short(fs_file, short);
void put_array(fs_file, const float *, size_t);

float get_float(fs_file);
int   get_index(fs_file);
short get_short(fs_file);
void  get_array(fs_file, float *, size_t);

void put_string(fs_file fout, const char *);
void get_string(fs_file fin, char *, int);

/*---------------------------------------------------------------------------*/

#endif
