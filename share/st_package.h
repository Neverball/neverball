#ifndef ST_PACKAGE
#define ST_PACKAGE 1

#include "state.h"

extern struct state st_package;

void goto_package(int package_id, struct state *back_state);
void package_set_installed_action(int (*installed_action_fn)(int pi));

#endif