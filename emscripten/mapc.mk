CC = emcc

JSDIR = js

# Emscripten fast linking: https://github.com/emscripten-core/emscripten/issues/17019
BUILD ?= devel

# Generate share/version.h
VERSION := $(shell sh scripts/version.sh)

ifeq ($(BUILD), devel)
CFLAGS := -O1 -fsanitize=undefined -fsanitize=address -std=gnu99 -Wall -Ishare
else
CFLAGS := -O3 -std=gnu99 -Wall -Ishare -DNDEBUG
endif

EM_CFLAGS := \
	-s USE_SDL=2 \
	-s USE_LIBPNG=1 \
	-s USE_LIBJPEG=1

EM_LDFLAGS := \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s LLD_REPORT_UNDEFINED \
	-s NODERAWFS

ifeq ($(BUILD), devel)
EM_LDFLAGS += -s ERROR_ON_WASM_CHANGES_AFTER_LINK -s WASM_BIGINT
endif

MAPC_SRCS := \
	share/mapc.c \
	share/array.c \
	share/base_config.c \
	share/base_image.c \
	share/binary.c \
	share/common.c \
	share/dir.c \
	share/fs_common.c \
	share/fs_jpg.c \
	share/fs_png.c \
	share/fs_stdio.c \
	share/miniz.c \
	share/list.c \
	share/log.c \
	share/queue.c \
	share/solid_base.c \
	share/vec3.c

MAPC_OBJS := $(MAPC_SRCS:.c=.emscripten.o)

%.emscripten.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $(EM_CFLAGS) $<

.PHONY: mapc
mapc: $(JSDIR)/mapc.js

$(JSDIR)/mapc.js: $(MAPC_OBJS)
	$(CC) -o $@ $(MAPC_OBJS) $(CFLAGS) $(EM_CFLAGS) $(LDFLAGS) $(EM_LDFLAGS)

.PHONY: clean
clean:
	$(RM) $(MAPC_OBJS) $(JSDIR)/mapc.js $(JSDIR)/mapc.wasm

.PHONY: sols
sols: mapc
	$(MAKE) -f Makefile sols MAPC="env node $(JSDIR)/mapc.js" MAPC_TARG="$(JSDIR)/mapc.js" -o "$(JSDIR)/mapc.js"