#ifndef PACKAGE_H
#define PACKAGE_H 1

#include "fetch.h"

enum package_status
{
    PACKAGE_NONE = 0,
    PACKAGE_AVAILABLE,
    PACKAGE_UPDATE,
    PACKAGE_DOWNLOADING,
    PACKAGE_INSTALLED,
    PACKAGE_ERROR
};

void package_init(void);
void package_quit(void);

int package_count(void);

int package_search(const char *file);
int package_search_id(const char *package_id);
int package_next(const char *type, int start);

unsigned int package_fetch(int, struct fetch_callback);

enum package_status package_get_status(int);

const char *package_get_id(int);
const char *package_get_type(int);
const char *package_get_name(int);
const char *package_get_desc(int);
const char *package_get_shot(int);
const char *package_get_files(int);

const char *package_get_shot_filename(int);
const char *package_get_formatted_type(int);

#endif