
#------------------------------------------------------------------------------

# Maybe you need one of these.  Maybe you don't.

#X11_PATH= -L/usr/X11/lib
#X11_PATH= -L/usr/X11R6/lib

OGL_LIBS= -lGL -lm
#OGL_LIBS= -lm                                                # Think Different

#------------------------------------------------------------------------------

CFLAGS= -Wall -O3 -ansi -pedantic $(shell sdl-config --cflags)
#CFLAGS= -Wall -g -ansi $(shell sdl-config --cflags)
#CFLAGS= -Wall -pg -ansi $(shell sdl-config --cflags)

SDL_LIBS= $(shell sdl-config --libs)
FT2_LIBS= $(shell freetype-config --libs)

MAPC_TARG= mapc
BALL_TARG= neverball
PUTT_TARG= neverputt

LOCALEDIR= locale
LOCALEDOM= neverball

LINGUAS= fr lv # List of locales to generates
POTFILE= po/neverball.pot

MAPC_OBJS= \
	share/vec3.o   \
	share/image.o  \
	share/solid.o  \
	share/binary.o \
	share/config.o \
	share/mapc.o
BALL_OBJS= \
	share/i18n.o    \
	share/st_lang.o \
	share/vec3.o    \
	share/image.o   \
	share/solid.o   \
	share/part.o    \
	share/back.o    \
	share/geom.o    \
	share/gui.o     \
	share/config.o  \
	share/binary.o  \
	share/state.o   \
	share/audio.o   \
	ball/hud.o      \
	ball/game.o     \
	ball/level.o    \
	ball/set.o      \
	ball/demo.o     \
	ball/util.o     \
	ball/st_conf.o  \
	ball/st_demo.o  \
	ball/st_save.o  \
	ball/st_fail.o  \
	ball/st_goal.o  \
	ball/st_done.o  \
	ball/st_level.o \
	ball/st_over.o  \
	ball/st_play.o  \
	ball/st_set.o   \
	ball/st_start.o \
	ball/st_title.o \
	ball/main.o
PUTT_OBJS= \
	share/i18n.o    \
	share/st_lang.o \
	share/vec3.o   \
	share/image.o  \
	share/solid.o  \
	share/part.o   \
	share/geom.o   \
	share/back.o   \
	share/config.o \
	share/binary.o \
	share/audio.o  \
	share/state.o  \
	share/gui.o    \
	putt/hud.o     \
	putt/game.o    \
	putt/hole.o    \
	putt/course.o  \
	putt/st_all.o  \
	putt/st_conf.o \
	putt/main.o

BALL_DEPS= $(BALL_OBJS:.o=.d)
PUTT_DEPS= $(PUTT_OBJS:.o=.d)
MAPC_DEPS= $(MAPC_OBJS:.o=.d)

LIBS= $(X11_PATH) $(SDL_LIBS) -lSDL_image -lSDL_ttf -lSDL_mixer $(FT2_LIBS) $(OGL_LIBS)

MESSAGEPART= /LC_MESSAGES/$(LOCALEDOM).mo
MESSAGES= $(LINGUAS:%=$(LOCALEDIR)/%$(MESSAGEPART))

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
	data/map-mym/assault.sol    \
	data/map-mym/bombman.sol    \
	data/map-mym/circuit1.sol   \
	data/map-mym/circuit2.sol   \
	data/map-mym/climb.sol      \
	data/map-mym/comeback.sol   \
	data/map-mym/dance2.sol     \
	data/map-mym/dance1.sol     \
	data/map-mym/descent.sol    \
	data/map-mym/drive1.sol     \
	data/map-mym/drive2.sol     \
	data/map-mym/earthquake.sol \
	data/map-mym/glasstower.sol \
	data/map-mym/hard.sol       \
	data/map-mym/loop1.sol      \
	data/map-mym/loop2.sol      \
	data/map-mym/maze1.sol      \
	data/map-mym/maze2.sol      \
	data/map-mym/narrow.sol     \
	data/map-mym/running.sol    \
	data/map-mym/scrambling.sol \
	data/map-mym/snow.sol       \
	data/map-mym/trust.sol      \
	data/map-mym/turn.sol       \
	data/map-mym/up.sol         \
	data/map-mym2/backforth.sol \
	data/map-mym2/basket.sol \
	data/map-mym2/bigball.sol \
	data/map-mym2/bigcones.sol \
	data/map-mym2/bounces.sol \
	data/map-mym2/fall.sol \
	data/map-mym2/freefall.sol \
	data/map-mym2/ghosts.sol \
	data/map-mym2/grinder.sol \
	data/map-mym2/littlecones.sol \
	data/map-mym2/longpipe.sol \
	data/map-mym2/morenarrow.sol \
	data/map-mym2/movinglumps.sol \
	data/map-mym2/movingpath.sol \
	data/map-mym2/push.sol \
	data/map-mym2/rainbow.sol \
	data/map-mym2/rodeo.sol \
	data/map-mym2/runstop.sol \
	data/map-mym2/shaker.sol \
	data/map-mym2/sonic.sol \
	data/map-mym2/speeddance.sol \
	data/map-mym2/speed.sol \
	data/map-mym2/translation.sol \
	data/map-mym2/updown.sol \
	data/map-mym2/webs.sol \
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
	data/map-back/alien.sol   \
	data/map-back/blank.sol   \
	data/map-back/city.sol    \
	data/map-back/clouds.sol  \
	data/map-back/ocean.sol   \
	data/map-back/jupiter.sol \
	data/map-paxed/01_fastpenta.sol  \
	data/map-paxed/02_bumpy_road.sol \
	data/map-paxed/03_zigzag.sol     \
	data/map-paxed/04_u_twist.sol    \
	data/map-paxed/05_hookey.sol     \
	data/map-paxed/06_partitions.sol \
	data/map-paxed/07_a_danger.sol   \
	data/map-paxed/08_f.sol          \
	data/map-paxed/09_rampedge.sol   \
	data/map-paxed/10_pentomino.sol  \
	data/map-paxed/11_stapler.sol    \
	data/map-paxed/12_helixramp.sol  \
	data/map-paxed/13_loch_ness.sol  \
	data/map-paxed/14_figur8.sol     \
	data/map-paxed/15_upNdown.sol    \
	data/map-paxed/16_pyramidian.sol \
	data/map-paxed/17_oopsydaisy.sol \
	data/map-paxed/18_3rd_floor.sol  \
	data/map-paxed/a.sol \
	data/map-paxed/b.sol \
	data/map-paxed/c.sol \
	data/map-paxed/d.sol \
	data/map-paxed/e.sol \
	data/map-paxed/f.sol \
	data/map-paxed/g.sol \
	data/map-paxed/h.sol \
	data/map-paxed/i.sol \
	data/map-paxed/j.sol \
	data/map-paxed/k.sol \
	data/map-paxed/l.sol \
	data/map-paxed/m.sol \
	data/map-paxed/n.sol \
	data/map-paxed/o.sol \
	data/map-paxed/p.sol \
	data/map-paxed/q.sol \
	data/map-paxed/r.sol \
	data/map-paxed/s.sol \
	data/map-paxed/t.sol \
	data/map-paxed/u.sol \
	data/map-paxed/v.sol \
	data/map-paxed/w.sol \
	data/map-paxed/x.sol \
	data/map-paxed/y.sol \
	data/map-paxed/z.sol

#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(CFLAGS) -Ishare -MM -MF $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -Ishare -o $@ -c $<

$(LOCALEDIR)/%$(MESSAGEPART) : po/%.po
	mkdir -p `dirname $@`
	msgfmt -c -v -o $@ $<
	
data/map-rlk/%.sol : data/map-rlk/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< data

data/map-mym/%.sol : data/map-mym/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< data

data/map-mym2/%.sol : data/map-mym2/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< data

data/map-putt/%.sol : data/map-putt/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< data

data/map-back/%.sol : data/map-back/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< data

data/map-paxed/%.sol : data/map-paxed/%.map $(MAPC_TARG)
	./$(MAPC_TARG) $< data

#------------------------------------------------------------------------------

all : $(BALL_TARG) $(PUTT_TARG) $(MAPC_TARG) $(SOLS) locales

$(BALL_TARG) : $(BALL_OBJS)
	$(CC) $(CFLAGS) -o $(BALL_TARG) $(BALL_OBJS) $(LIBS)

$(PUTT_TARG) : $(PUTT_OBJS)
	$(CC) $(CFLAGS) -o $(PUTT_TARG) $(PUTT_OBJS) $(LIBS)

$(MAPC_TARG) : $(MAPC_OBJS)
	$(CC) $(CFLAGS) -o $(MAPC_TARG) $(MAPC_OBJS) $(LIBS)

locales: $(MESSAGES)

clean-src :
	rm -f $(BALL_TARG) $(BALL_OBJS) $(BALL_DEPS)
	rm -f $(PUTT_TARG) $(PUTT_OBJS) $(PUTT_DEPS)
	rm -f $(MAPC_TARG) $(MAPC_OBJS) $(MAPC_DEPS)

clean : clean-src
	rm -f $(SOLS)
	rm -rf $(LOCALEDIR)

test : all
	./neverball

#------------------------------------------------------------------------------

po/%.po : $(POTFILE)
	msgmerge -U $@ $<
	touch $@
	
po-update-extract :
	bash extractpo.sh $(POTFILE) $(LOCALEDOM)

po-update-merge : $(LINGUAS:%=po/%.po)

po-update : po-update-extract po-update-merge

#------------------------------------------------------------------------------
