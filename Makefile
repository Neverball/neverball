
#------------------------------------------------------------------------------

# Maybe you need one of these.  Maybe you don't.

#X11_PATH= -L/usr/X11/lib
#X11_PATH= -L/usr/X11R6/lib

#------------------------------------------------------------------------------

CFLAGS= -Wall -g -ansi $(shell sdl-config --cflags)

X11_LIBS= $(X11_PATH) -lGLU -lGL -lm
SDL_LIBS= $(shell sdl-config --libs)

MAPC_TARG= mapc
GAME_TARG= neverball

MAPC_OBJS= \
	vec3.o   \
	solid.o  \
	glext.o  \
	mapc.o
GAME_OBJS= \
	vec3.o   \
	text.o   \
	image.o  \
	audio.o  \
	solid.o  \
	hud.o    \
	part.o   \
	geom.o   \
	back.o   \
	game.o   \
	level.o  \
	menu.o   \
	state.o  \
	config.o \
	glext.o  \
	main.o

MAPC_DEPS= $(GAME_SRCS:.c=.d)
GAME_DEPS= $(GAME_OBJS:.o=.d)

MAPC_LIBS= $(SDL_LIBS) -lSDL_image $(X11_LIBS)
GAME_LIBS= $(SDL_LIBS) -lSDL_image -lSDL_ttf -lSDL_mixer -lfreetype $(X11_LIBS)

SOLS= \
	data/sol/00_title.sol   \
	data/sol/01_easy.sol    \
	data/sol/02_coin.sol    \
	data/sol/03_slow.sol    \
	data/sol/04_fence.sol   \
	data/sol/05_fall.sol    \
	data/sol/06_move.sol    \
	data/sol/07_waka.sol    \
	data/sol/08_curve.sol   \
	data/sol/09_maze.sol    \
	data/sol/10_spiral.sol  \
	data/sol/11_ramp.sol    \
	data/sol/12_goal.sol    \
	data/sol/13_pyramid.sol \
	data/sol/14_quads.sol   \
	data/sol/15_frog.sol    \
	data/sol/16_angle.sol   \
	data/sol/17_spread.sol  \
	data/sol/18_four.sol    \
	data/sol/19_hump.sol    \
	data/sol/20_movers.sol  \
	data/sol/21_poke.sol    \
	data/sol/22_ring.sol    \
	data/sol/23_tele.sol    \
	data/sol/24_marble.sol

#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(CFLAGS) -MM -MF $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -c $<

data/sol/%.sol : data/map/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< $@ data

#------------------------------------------------------------------------------

all : $(GAME_TARG) $(MAPC_TARG) $(SOLS)

$(GAME_TARG) : $(GAME_OBJS)
	$(CC) $(CFLAGS) -o $(GAME_TARG) $(GAME_OBJS) $(GAME_LIBS)

$(MAPC_TARG) : $(MAPC_OBJS)
	$(CC) $(CFLAGS) -o $(MAPC_TARG) $(MAPC_OBJS) $(MAPC_LIBS)

clean :
	rm -f $(GAME_TARG) $(GAME_OBJS) $(GAME_DEPS)
	rm -f $(MAPC_TARG) $(MAPC_OBJS) $(MAPC_DEPS)
	rm -f $(SOLS)

#------------------------------------------------------------------------------

include $(GAME_DEPS) $(MAPC_DEPS)
