
BALL_OBJS= \
	game.o  \
	geom.o  \
	image.o \
	level.o \
	main.o  \
	solid.o \
	state.o \
	vec3.o
MAPC_OBJS= \
	vec3.o  \
	solid.o \
	mapc.o

CFLAGS= -Wall -g -ffast-math -ansi $(shell sdl-config --cflags)

BALL_LIBS= $(shell sdl-config --libs) -lSDL_image -lSDL_mixer -lGLU -lGL
MAPC_LIBS= $(shell sdl-config --libs) -lSDL_image -lGL

all : ../ball ../mapc

.c.o :
	$(CC) $(CFLAGS) -c $<

../ball : ball
	cp ball ../ball

../mapc : mapc
	cp mapc ../mapc

ball : $(BALL_OBJS)
	$(CC) $(CFLAGS) -o ball $(BALL_OBJS) $(BALL_LIBS)

mapc : $(MAPC_OBJS)
	$(CC) $(CFLAGS) -o mapc $(MAPC_OBJS) $(MAPC_LIBS)

clean :
	rm -f ball mapc $(BALL_OBJS) $(MAPC_OBJS)

