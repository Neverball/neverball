
CFLAGS+= -Wall -g -ansi -pedantic $(shell sdl-config --cflags)

DEMOCONV= democonv

DEMOCONV_OBJS= \
	../share/binary.o \
	src/democonv.o

%.o : %.c
	$(CC) $(CFLAGS) -I../share -I../ball -o $@ -c $<

all: $(DEMOCONV)

$(DEMOCONV): $(DEMOCONV_OBJS)
	$(CC) $(CFLAGS) -o $(DEMOCONV) $(DEMOCONV_OBJS) $(LIBS)

clean :
	$(RM) $(DEMOCONV) $(DEMOCONV_OBJS)
