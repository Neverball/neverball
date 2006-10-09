
CFLAGS+= -Wall -g -ansi -pedantic $(shell sdl-config --cflags)
SDL_LIBS= $(shell sdl-config --libs)

DEMOCONV= democonv

DEMOCONV_OBJS= \
	../share/binary.o \
	src/democonv.o

%.o : %.c
	$(CC) $(CFLAGS) -I../share -I../ball -o $@ -c $<

all: $(DEMOCONV)

$(DEMOCONV): $(DEMOCONV_OBJS)
	$(CC) $(CFLAGS) -o $(DEMOCONV) $(DEMOCONV_OBJS) $(SDL_LIBS)

clean :
	$(RM) $(DEMOCONV) $(DEMOCONV_OBJS)
