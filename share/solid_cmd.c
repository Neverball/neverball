#include "solid_cmd.h"
#include "cmd.h"

/*---------------------------------------------------------------------------*/

static void (*cmd_enq_fn)(const union cmd *);

void sol_cmd_enq(const union cmd *new)
{
    if (cmd_enq_fn)
        cmd_enq_fn(new);
}

void sol_cmd_enq_func(void (*enq_fn) (const union cmd *))
{
    cmd_enq_fn = enq_fn;
}

/*---------------------------------------------------------------------------*/
