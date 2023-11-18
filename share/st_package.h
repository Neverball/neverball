#ifndef ST_PACKAGE
#define ST_PACKAGE 1

#include "state.h"

extern struct state st_package;

void goto_package(int package_id, struct state *back_state);

#endif