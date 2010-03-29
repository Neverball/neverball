#ifndef SOLID_CMD_H
#define SOLID_CMD_H

#include "cmd.h"

extern int sol_cmd_defer;

void sol_cmd_enq(const union cmd *);
void sol_cmd_enq_deferred(void);
void sol_cmd_enq_func(void (*enq_fn) (const union cmd *));

#endif
