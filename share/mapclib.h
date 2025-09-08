#ifndef MAPCLIB_H
#define MAPCLIB_H

#include <setjmp.h>

typedef struct mapc_context *mapc_context;

int mapc_init(mapc_context *ctx_ptr);
void mapc_quit(mapc_context *ctx_ptr);

int mapc_opts(mapc_context ctx, int argc, char *argv[]);

void mapc_set_src(mapc_context ctx, const char *src);
void mapc_set_dst(mapc_context ctx, const char *dst);

int mapc_compile(mapc_context ctx);

void mapc_dump(mapc_context ctx);

#endif
