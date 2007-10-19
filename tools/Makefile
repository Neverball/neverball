#------------------------------------------------------------------------------

CFLAGS := -Wall -g -pedantic

#------------------------------------------------------------------------------

DEMOCONV := democonv$(EXT)
DEMOCONV_CPPFLAGS := \
    -I../ball -I../share $(shell sdl-config --cflags) -Umain
DEMOCONV_OBJS := ../share/binary.o src/democonv.o

#------------------------------------------------------------------------------

all: $(DEMOCONV)

$(DEMOCONV): $(DEMOCONV_OBJS)
	$(CC) $(CFLAGS) $(DEMOCONV_OBJS) $(LDFLAGS) -o $(DEMOCONV)

$(DEMOCONV): CPPFLAGS := $(DEMOCONV_CPPFLAGS) $(CPPFLAGS)

clean:
	$(RM) $(DEMOCONV) $(DEMOCONV_OBJS)

#------------------------------------------------------------------------------

