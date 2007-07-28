
# Common compiler flags
CFLAGS += -Wall -g -pedantic

#------------------------------------------------------------------------------

DEMOCONV := democonv$(EXT)
DEMOCONV_OBJS := \
	../share/binary.o \
	src/democonv.o

#------------------------------------------------------------------------------

all: $(DEMOCONV)

$(DEMOCONV): $(DEMOCONV_OBJS)
	$(CC) $(CFLAGS) -o $(DEMOCONV) $(DEMOCONV_OBJS) $(LDFLAGS)

$(DEMOCONV): CPPFLAGS := -I../ball -I../share \
    $(shell sdl-config --cflags) -Umain $(CPPFLAGS)

clean :
	$(RM) $(DEMOCONV) $(DEMOCONV_OBJS)

#------------------------------------------------------------------------------
