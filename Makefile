
#------------------------------------------------------------------------------

# Maybe you need one of these.  Maybe you don't.

#X11_PATH= -L/usr/X11/lib
#X11_PATH= -L/usr/X11R6/lib

OGL_LIBS= -lGLU -lGL -lm
#OGL_LIBS= -lm                                                # Think Different

#------------------------------------------------------------------------------

#CFLAGS= -Wall -pg -ansi $(shell sdl-config --cflags)
#CFLAGS= -Wall -O3 -ansi $(shell sdl-config --cflags)
CFLAGS= -Wall -g -ansi $(shell sdl-config --cflags)

SDL_LIBS= $(shell sdl-config --libs)
FT2_LIBS= $(shell freetype-config --libs)

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
	set.o    \
	menu.o   \
	state.o  \
	config.o \
	glext.o  \
	main.o

GAME_DEPS= $(GAME_OBJS:.o=.d)
MAPC_DEPS= $(MAPC_OBJS:.o=.d)

MAPC_LIBS= $(X11_PATH) $(SDL_LIBS) -lSDL_image $(OGL_LIBS)
GAME_LIBS= $(X11_PATH) $(SDL_LIBS) -lSDL_image -lSDL_ttf -lSDL_mixer $(FT2_LIBS) $(OGL_LIBS)

SOLS= \
	data/sol-rlk/title.sol    \
	data/sol-rlk/pipe.sol     \
	data/sol-rlk/pyramid.sol  \
	data/sol-rlk/greed.sol    \
	data/sol-rlk/easytele.sol \
	data/sol-rlk/drops.sol    \
	data/sol-rlk/curbs.sol    \
	data/sol-rlk/rampdn.sol   \
	data/sol-rlk/invis.sol    \
	data/sol-rlk/plinko.sol   \
	data/sol-rlk/telemaze.sol \
	data/sol-rlk/islands.sol  \
	data/sol-rlk/check.sol    \
	data/sol-rlk/bumps.sol    \
	data/sol-rlk/corners.sol  \
	data/sol-rlk/peasy.sol    \
	data/sol-rlk/timer.sol    \
	data/sol-rlk/bumper.sol   \
	data/sol-rlk/stairs.sol   \
	data/sol-rlk/tilt.sol     \
	data/sol-rlk/gaps.sol     \
	data/sol-rlk/spiralin.sol \
	data/sol-rlk/locks.sol    \
	data/sol-rlk/easy.sol     \
	data/sol-rlk/coins.sol    \
	data/sol-rlk/goslow.sol   \
	data/sol-rlk/fence.sol    \
	data/sol-rlk/hole.sol     \
	data/sol-rlk/zigzag.sol   \
	data/sol-rlk/mover.sol    \
	data/sol-rlk/wakka.sol    \
	data/sol-rlk/curved.sol   \
	data/sol-rlk/sync.sol     \
	data/sol-rlk/grid.sol     \
	data/sol-rlk/maze.sol     \
	data/sol-rlk/spiraldn.sol \
	data/sol-rlk/spiralup.sol \
	data/sol-rlk/rampup.sol   \
	data/sol-rlk/goals.sol    \
	data/sol-rlk/risers.sol   \
	data/sol-rlk/quads.sol    \
	data/sol-rlk/frogger.sol  \
	data/sol-rlk/angle.sol    \
	data/sol-rlk/spread.sol   \
	data/sol-rlk/four.sol     \
	data/sol-rlk/hump.sol     \
	data/sol-rlk/movers.sol   \
	data/sol-rlk/poker.sol    \
	data/sol-rlk/ring.sol     \
	data/sol-rlk/teleport.sol \
	data/sol-mym/assault_course.sol \
	data/sol-mym/autosave.sol \
	data/sol-mym/bigdipper.sol \
	data/sol-mym/bombman.sol \
	data/sol-mym/circuit1.sol \
	data/sol-mym/circuit2.sol \
	data/sol-mym/climb.sol \
	data/sol-mym/comeback.sol \
	data/sol-mym/dance.sol \
	data/sol-mym/descent.sol \
	data/sol-mym/drive1.sol \
	data/sol-mym/drive2.sol \
	data/sol-mym/earth_quake.sol \
	data/sol-mym/free_fall.sol \
	data/sol-mym/glass_tower.sol \
	data/sol-mym/hard.sol \
	data/sol-mym/loop1.sol \
	data/sol-mym/loop2.sol \
	data/sol-mym/maze1.sol \
	data/sol-mym/maze2.sol \
	data/sol-mym/maze3.sol \
	data/sol-mym/narrow.sol \
	data/sol-mym/running.sol \
	data/sol-mym/scrambling.sol \
	data/sol-mym/trust.sol \
	data/sol-mym/turn.sol \
	data/sol-mym/up.sol

#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(CFLAGS) -MM -MF $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -c $<

data/sol-rlk/%.sol : data/map-rlk/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< $@ data

data/sol-mym/%.sol : data/map-mym/%.map $(MAPC_TARG)
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
