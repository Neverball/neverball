
#------------------------------------------------------------------------------

# Maybe you need one of these.  Maybe you don't.

#X11_PATH= -L/usr/X11/lib
#X11_PATH= -L/usr/X11R6/lib

OGL_LIBS= -lGLU -lGL -lm
#OGL_LIBS= -lm                                                # Think Different

#------------------------------------------------------------------------------

#CFLAGS= -Wall -Werror -O3 -ansi $(shell sdl-config --cflags)
CFLAGS= -Wall -Werror -g -ansi $(shell sdl-config --cflags)
#CFLAGS= -Wall -Werror -pg -ansi $(shell sdl-config --cflags)

SDL_LIBS= $(shell sdl-config --libs)
FT2_LIBS= $(shell freetype-config --libs)

MAPC_TARG= mapc
BALL_TARG= neverball
PUTT_TARG= neverputt

MAPC_OBJS= \
	share/vec3.o   \
	share/solid.o  \
	share/glext.o  \
	share/config.o \
	share/mapc.o
BALL_OBJS= \
	share/vec3.o    \
	share/text.o    \
	share/image.o   \
	share/solid.o   \
	share/part.o    \
	share/back.o    \
	share/geom.o    \
	share/gui.o     \
	share/menu.o    \
	share/glext.o   \
	share/config.o  \
	share/state.o   \
	ball/audio.o    \
	ball/hud.o      \
	ball/game.o     \
	ball/level.o    \
	ball/set.o      \
	ball/demo.o     \
	ball/util.o     \
	ball/st_conf.o  \
	ball/st_demo.o  \
	ball/st_fail.o  \
	ball/st_goal.o  \
	ball/st_level.o \
	ball/st_over.o  \
	ball/st_play.o  \
	ball/st_set.o   \
	ball/st_start.o \
	ball/st_title.o \
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

LIBS= $(X11_PATH) $(SDL_LIBS) -lSDL_image -lSDL_ttf -lSDL_mixer $(FT2_LIBS) $(OGL_LIBS)

SOLS= \
	data/map-rlk/easy.sol     \
	data/map-rlk/peasy.sol    \
	data/map-rlk/coins.sol    \
	data/map-rlk/goslow.sol   \
	data/map-rlk/fence.sol    \
	data/map-rlk/bumper.sol   \
	data/map-rlk/maze.sol     \
	data/map-rlk/goals.sol    \
	data/map-rlk/hole.sol     \
	data/map-rlk/bumps.sol    \
	data/map-rlk/corners.sol  \
	data/map-rlk/easytele.sol \
	data/map-rlk/zigzag.sol   \
	data/map-rlk/greed.sol    \
	data/map-rlk/mover.sol    \
	data/map-rlk/wakka.sol    \
	data/map-rlk/curbs.sol    \
	data/map-rlk/curved.sol   \
	data/map-rlk/stairs.sol   \
	data/map-rlk/rampdn.sol   \
	data/map-rlk/sync.sol     \
	data/map-rlk/spiralin.sol \
	data/map-rlk/plinko.sol   \
	data/map-rlk/drops.sol    \
	data/map-rlk/locks.sol    \
	data/map-rlk/grid.sol     \
	data/map-rlk/four.sol     \
	data/map-rlk/telemaze.sol \
	data/map-rlk/spiraldn.sol \
	data/map-rlk/islands.sol  \
	data/map-rlk/angle.sol    \
	data/map-rlk/spiralup.sol \
	data/map-rlk/rampup.sol   \
	data/map-rlk/check.sol    \
	data/map-rlk/risers.sol   \
	data/map-rlk/tilt.sol     \
	data/map-rlk/gaps.sol     \
	data/map-rlk/pyramid.sol  \
	data/map-rlk/quads.sol    \
	data/map-rlk/frogger.sol  \
	data/map-rlk/timer.sol    \
	data/map-rlk/spread.sol   \
	data/map-rlk/hump.sol     \
	data/map-rlk/movers.sol   \
	data/map-rlk/teleport.sol \
	data/map-rlk/poker.sol    \
	data/map-rlk/invis.sol    \
	data/map-rlk/ring.sol     \
	data/map-rlk/pipe.sol     \
	data/map-rlk/title.sol    \
	data/map-mym/descent.sol        \
	data/map-mym/glass_tower.sol    \
	data/map-mym/scrambling.sol     \
	data/map-mym/trust.sol          \
	data/map-mym/maze1.sol          \
	data/map-mym/turn.sol           \
	data/map-mym/loop1.sol          \
	data/map-mym/free_fall.sol      \
	data/map-mym/earth_quake.sol    \
	data/map-mym/drive1.sol         \
	data/map-mym/maze2.sol          \
	data/map-mym/up.sol             \
	data/map-mym/circuit2.sol       \
	data/map-mym/comeback.sol       \
	data/map-mym/maze3.sol          \
	data/map-mym/assault_course.sol \
	data/map-mym/narrow.sol         \
	data/map-mym/loop2.sol          \
	data/map-mym/drive2.sol         \
	data/map-mym/running.sol        \
	data/map-mym/circuit1.sol       \
	data/map-mym/bombman.sol        \
	data/map-mym/climb.sol          \
	data/map-mym/dance.sol          \
	data/map-mym/hard.sol           \
	data/map-putt/01_easy.sol    \
	data/map-putt/02_uturn.sol   \
	data/map-putt/03_kink.sol    \
	data/map-putt/04_aych.sol    \
	data/map-putt/05_ramp.sol    \
	data/map-putt/06_cone.sol    \
	data/map-putt/07_tele.sol    \
	data/map-putt/08_zig.sol     \
	data/map-putt/09_vturn.sol   \
	data/map-putt/10_diamond.sol \
	data/map-putt/11_mover.sol   \
	data/map-putt/12_bumpy.sol   \
	data/map-putt/13_choose.sol  \
	data/map-putt/14_walls.sol   \
	data/map-putt/15_hole.sol    \
	data/map-putt/16_pipe.sol    \
	data/map-putt/17_timing.sol  \
	data/map-putt/18_hill.sol    \
	data/map-back/alien.sol  \
	data/map-back/blank.sol  \
	data/map-back/city.sol   \
	data/map-back/clouds.sol \
	data/map-back/ocean.sol  \
	data/map-back/jupiter.sol

#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(CFLAGS) -Ishare -MM -MF $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -Ishare -o $@ -c $<

data/map-rlk/%.sol : data/map-rlk/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< data

data/map-mym/%.sol : data/map-mym/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< data

data/map-putt/%.sol : data/map-putt/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< data

data/map-back/%.sol : data/map-back/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< data

data/map-misc/%.sol : data/map-misc/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< data

#------------------------------------------------------------------------------

all : $(BALL_TARG) $(PUTT_TARG) $(MAPC_TARG) $(SOLS)

$(BALL_TARG) : $(BALL_OBJS)
	$(CC) $(CFLAGS) -o $(BALL_TARG) $(BALL_OBJS) $(LIBS)

$(PUTT_TARG) : $(PUTT_OBJS)
	$(CC) $(CFLAGS) -o $(PUTT_TARG) $(PUTT_OBJS) $(LIBS)

$(MAPC_TARG) : $(MAPC_OBJS)
	$(CC) $(CFLAGS) -o $(MAPC_TARG) $(MAPC_OBJS) $(LIBS)

clean-src :
	rm -f $(BALL_TARG) $(BALL_OBJS) $(BALL_DEPS)
	rm -f $(PUTT_TARG) $(PUTT_OBJS) $(PUTT_DEPS)
	rm -f $(MAPC_TARG) $(MAPC_OBJS) $(MAPC_DEPS)

clean : clean-src
	rm -f $(SOLS)

test : all
	./neverball

#------------------------------------------------------------------------------
