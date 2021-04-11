#ifndef PACKAGE_H
#define PACKAGE_H 1

#include "fetch.h"

enum package_status
{
    PACKAGE_NONE = 0,
    PACKAGE_AVAILABLE,
    PACKAGE_DOWNLOADING,
    PACKAGE_PARTIAL,
    PACKAGE_INSTALLED,
    PACKAGE_ERROR
};

void package_init(void);
void package_quit(void);

int package_search(const char *file);
int package_next(const char *type, int start);

unsigned int package_fetch(int, struct fetch_callback);

enum package_status package_get_status(int);

const char *package_get_name(int);
const char *package_get_desc(int);
const char *package_get_shot(int);
const char *package_get_files(int);

const char *package_get_shot_filename(int);

#endif