
GAME_OBJS= \
	config.o \
	audio.o  \
	game.o   \
	geom.o   \
	back.o   \
	part.o   \
	image.o  \
	level.o  \
	main.o   \
	menu.o   \
	solid.o  \
	state.o  \
	text.o   \
	hud.o    \
	vec3.o
MAPC_OBJS= \
	vec3.o  \
	solid.o \
	mapc.o

CFLAGS= -Wall -O2 -ffast-math -ansi $(shell sdl-config --cflags)

X11_LIBS= -L/usr/X11/lib -lGLU -lGL -lm
SDL_LIBS= $(shell sdl-config --libs)

GAME_LIBS= $(SDL_LIBS) -lSDL_image -lSDL_ttf -lSDL_mixer -lfreetype $(X11_LIBS)
MAPC_LIBS= $(SDL_LIBS) -lSDL_image $(X11_LIBS)

all : ../neverball ../mapc

.c.o :
	$(CC) $(CFLAGS) -c $<

../neverball : neverball
	cp neverball ../neverball

../mapc : mapc
	cp mapc ../mapc

neverball : $(GAME_OBJS)
	$(CC) $(CFLAGS) -o neverball $(GAME_OBJS) $(GAME_LIBS)

mapc : $(MAPC_OBJS)
	$(CC) $(CFLAGS) -o mapc $(MAPC_OBJS) $(MAPC_LIBS)

clean :
	rm -f neverball mapc $(GAME_OBJS) $(MAPC_OBJS)

