
#------------------------------------------------------------------------------

# Maybe you need one of these.  Maybe you don't.

#X11_PATH= -L/usr/X11/lib
#X11_PATH= -L/usr/X11R6/lib

OGL_LIBS= -lGLU -lGL -lm
#OGL_LIBS= -lm                                                # Think Different

#------------------------------------------------------------------------------

CFLAGS= -Wall -O3 -ansi $(shell sdl-config --cflags)
#CFLAGS= -Wall -g -ansi $(shell sdl-config --cflags)
#CFLAGS= -Wall -pg -ansi $(shell sdl-config --cflags)

SDL_LIBS= $(shell sdl-config --libs)
FT2_LIBS= $(shell freetype-config --libs)

MAPC_TARG= mapc
BALL_TARG= neverball
PUTT_TARG= neverputt

MAPC_OBJS= \
	share/vec3.o   \
	share/solid.o  \
	share/glext.o  \
	share/mapc.o
BALL_OBJS= \
	share/vec3.o   \
	share/text.o   \
	share/image.o  \
	share/solid.o  \
	share/part.o   \
	share/geom.o   \
	share/back.o   \
	share/gui.o    \
	share/menu.o   \
	share/glext.o  \
	share/config.o \
	ball/audio.o   \
	ball/hud.o     \
	ball/game.o    \
	ball/level.o   \
	ball/set.o     \
	ball/state.o   \
	ball/main.o
PUTT_OBJS= \
	share/vec3.o   \
	share/text.o   \
	share/image.o  \
	share/solid.o  \
	share/part.o   \
	share/geom.o   \
	share/back.o   \
	share/menu.o   \
	share/glext.o  \
	share/config.o \
	putt/audio.o   \
	putt/hud.o     \
	putt/game.o    \
	putt/hole.o    \
	putt/state.o   \
	putt/main.o

BALL_DEPS= $(BALL_OBJS:.o=.d)
PUTT_DEPS= $(PUTT_OBJS:.o=.d)
MAPC_DEPS= $(MAPC_OBJS:.o=.d)

MAPC_LIBS= $(X11_PATH) $(SDL_LIBS) -lSDL_image $(OGL_LIBS)
BALL_LIBS= $(X11_PATH) $(SDL_LIBS) -lSDL_image -lSDL_ttf -lSDL_mixer $(FT2_LIBS) $(OGL_LIBS)
PUTT_LIBS= $(X11_PATH) $(SDL_LIBS) -lSDL_image -lSDL_ttf -lSDL_mixer $(FT2_LIBS) $(OGL_LIBS)

SOLS= \
	data/sol-rlk/easy.sol     \
	data/sol-rlk/peasy.sol    \
	data/sol-rlk/coins.sol    \
	data/sol-rlk/goslow.sol   \
	data/sol-rlk/fence.sol    \
	data/sol-rlk/bumper.sol   \
	data/sol-rlk/maze.sol     \
	data/sol-rlk/goals.sol    \
	data/sol-rlk/hole.sol     \
	data/sol-rlk/bumps.sol    \
	data/sol-rlk/corners.sol  \
	data/sol-rlk/easytele.sol \
	data/sol-rlk/zigzag.sol   \
	data/sol-rlk/greed.sol    \
	data/sol-rlk/mover.sol    \
	data/sol-rlk/wakka.sol    \
	data/sol-rlk/curbs.sol    \
	data/sol-rlk/curved.sol   \
	data/sol-rlk/stairs.sol   \
	data/sol-rlk/rampdn.sol   \
	data/sol-rlk/sync.sol     \
	data/sol-rlk/spiralin.sol \
	data/sol-rlk/plinko.sol   \
	data/sol-rlk/drops.sol    \
	data/sol-rlk/locks.sol    \
	data/sol-rlk/grid.sol     \
	data/sol-rlk/four.sol     \
	data/sol-rlk/telemaze.sol \
	data/sol-rlk/spiraldn.sol \
	data/sol-rlk/islands.sol  \
	data/sol-rlk/angle.sol    \
	data/sol-rlk/spiralup.sol \
	data/sol-rlk/rampup.sol   \
	data/sol-rlk/check.sol    \
	data/sol-rlk/risers.sol   \
	data/sol-rlk/tilt.sol     \
	data/sol-rlk/gaps.sol     \
	data/sol-rlk/pyramid.sol  \
	data/sol-rlk/quads.sol    \
	data/sol-rlk/frogger.sol  \
	data/sol-rlk/timer.sol    \
	data/sol-rlk/spread.sol   \
	data/sol-rlk/hump.sol     \
	data/sol-rlk/movers.sol   \
	data/sol-rlk/teleport.sol \
	data/sol-rlk/poker.sol    \
	data/sol-rlk/invis.sol    \
	data/sol-rlk/ring.sol     \
	data/sol-rlk/pipe.sol     \
	data/sol-rlk/title.sol    \
	data/sol-mym/descent.sol        \
	data/sol-mym/glass_tower.sol    \
	data/sol-mym/scrambling.sol     \
	data/sol-mym/trust.sol          \
	data/sol-mym/maze1.sol          \
	data/sol-mym/turn.sol           \
	data/sol-mym/loop1.sol          \
	data/sol-mym/free_fall.sol      \
	data/sol-mym/earth_quake.sol    \
	data/sol-mym/drive1.sol         \
	data/sol-mym/maze2.sol          \
	data/sol-mym/up.sol             \
	data/sol-mym/circuit2.sol       \
	data/sol-mym/comeback.sol       \
	data/sol-mym/maze3.sol          \
	data/sol-mym/assault_course.sol \
	data/sol-mym/narrow.sol         \
	data/sol-mym/loop2.sol          \
	data/sol-mym/drive2.sol         \
	data/sol-mym/running.sol        \
	data/sol-mym/circuit1.sol       \
	data/sol-mym/bombman.sol        \
	data/sol-mym/climb.sol          \
	data/sol-mym/dance.sol          \
	data/sol-mym/hard.sol           \
	data/sol-putt/01_easy.sol    \
	data/sol-putt/02_uturn.sol   \
	data/sol-putt/03_kink.sol    \
	data/sol-putt/04_aych.sol    \
	data/sol-putt/05_ramp.sol    \
	data/sol-putt/06_cone.sol    \
	data/sol-putt/07_tele.sol    \
	data/sol-putt/08_zig.sol     \
	data/sol-putt/09_vturn.sol   \
	data/sol-putt/10_diamond.sol \
	data/sol-putt/11_mover.sol   \
	data/sol-putt/12_bumpy.sol   \
	data/sol-putt/13_choose.sol  \
	data/sol-putt/14_walls.sol   \
	data/sol-putt/15_hole.sol    \
	data/sol-putt/16_pipe.sol    \
	data/sol-putt/17_timing.sol  \
	data/sol-putt/18_hill.sol



#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(CFLAGS) -Ishare -MM -MF $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -Ishare -o $@ -c $<

data/sol-rlk/%.sol : data/map-rlk/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< $@ data

data/sol-mym/%.sol : data/map-mym/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< $@ data

data/sol-putt/%.sol : data/map-putt/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< $@ data

data/sol-misc/%.sol : data/map-misc/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< $@ data

#------------------------------------------------------------------------------

#all : $(BALL_TARG) $(PUTT_TARG) $(MAPC_TARG) $(SOLS)
all : $(BALL_TARG) $(MAPC_TARG) $(SOLS)

$(BALL_TARG) : $(BALL_OBJS)
	$(CC) $(CFLAGS) -o $(BALL_TARG) $(BALL_OBJS) $(BALL_LIBS)

$(PUTT_TARG) : $(PUTT_OBJS)
	$(CC) $(CFLAGS) -o $(PUTT_TARG) $(PUTT_OBJS) $(PUTT_LIBS)

$(MAPC_TARG) : $(MAPC_OBJS)
	$(CC) $(CFLAGS) -o $(MAPC_TARG) $(MAPC_OBJS) $(MAPC_LIBS)

clean-src :
	rm -f $(BALL_TARG) $(BALL_OBJS) $(BALL_DEPS)
	rm -f $(PUTT_TARG) $(PUTT_OBJS) $(PUTT_DEPS)
	rm -f $(MAPC_TARG) $(MAPC_OBJS) $(MAPC_DEPS)

clean : clean-src
	rm -f $(SOLS)

#------------------------------------------------------------------------------
