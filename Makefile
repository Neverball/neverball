
#------------------------------------------------------------------------------

# Maybe you need one of these.  Maybe you don't.

#X11_PATH= -L/usr/X11/lib
#X11_PATH= -L/usr/X11R6/lib

OGL_LIBS= -lGL -lm
#OGL_LIBS= -lm                                                # Think Different

#------------------------------------------------------------------------------
# Configuration constants
#------------------------------------------------------------------------------

CFLAGS= -Wall -g -O3 -ansi -pedantic $(shell sdl-config --cflags)
#CFLAGS= -Wall -g -O1 -ansi -pedantic $(shell sdl-config --cflags)
#CFLAGS= -Wall -pg -ansi $(shell sdl-config --cflags)

CPPFLAGS := -Ishare $(CPPFLAGS)

SDL_LIBS= $(shell sdl-config --libs)
PNG_LIBS= $(shell libpng-config --libs)

MAPC_TARG= mapc
BALL_TARG= neverball
PUTT_TARG= neverputt

MAPC_EXEC= ./$(MAPC_TARG)

LOCALEDIR= locale
LOCALEDOM= neverball

POTFILE= po/neverball.pot

#-------------------------------------------------------------------------------

MAPC_OBJS= \
	share/vec3.o   \
	share/base_image.o  \
	share/solid.o  \
	share/binary.o \
	share/base_config.o \
	share/mapc.o
BALL_OBJS= \
	share/i18n.o    \
	share/st_lang.o \
	share/st_resol.o \
	share/vec3.o    \
	share/base_image.o   \
	share/image.o   \
	share/solid.o   \
	share/solid_gl.o\
	share/part.o    \
	share/back.o    \
	share/geom.o    \
	share/gui.o     \
	share/base_config.o  \
	share/config.o  \
	share/binary.o  \
	share/state.o   \
	share/audio.o   \
	ball/hud.o      \
	ball/game.o     \
	ball/level.o    \
	ball/levels.o   \
	ball/set.o      \
	ball/demo.o     \
	ball/util.o     \
	ball/st_conf.o  \
	ball/st_demo.o  \
	ball/st_save.o  \
	ball/st_play_end.o  \
	ball/st_done.o  \
	ball/st_level.o \
	ball/st_over.o  \
	ball/st_play.o  \
	ball/st_set.o   \
	ball/st_start.o \
	ball/st_title.o \
	ball/st_help.o  \
	ball/st_name.o  \
	ball/st_shared.o  \
	ball/main.o
PUTT_OBJS= \
	share/i18n.o    \
	share/st_lang.o \
	share/st_resol.o \
	share/vec3.o   \
	share/base_image.o  \
	share/image.o  \
	share/solid.o  \
	share/solid_gl.o  \
	share/part.o   \
	share/geom.o   \
	share/back.o   \
	share/base_config.o  \
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

BASE_LIBS= $(SDL_LIBS) -lSDL_image
LIBS= $(X11_PATH) $(BASE_LIBS) $(PNG_LIBS) -lSDL_ttf -lSDL_mixer $(OGL_LIBS)

MESSAGEPART= /LC_MESSAGES/$(LOCALEDOM).mo
MESSAGES= $(LINGUAS:%=$(LOCALEDIR)/%$(MESSAGEPART))

MAPS= $(shell find data/ -name '*.map')
SOLS= $(MAPS:%.map=%.sol)

POS= $(shell echo po/*.po)
LINGUAS= $(POS:po/%.po=%)

#------------------------------------------------------------------------------
# Implicit rules
#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM -MF $@ -MT '$*.o $@' $<

%.o : %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

%.sol : %.map $(MAPC_TARG)
	$(MAPC_EXEC) $< data

$(LOCALEDIR)/%$(MESSAGEPART) : po/%.po
	mkdir -p `dirname $@`
	msgfmt -c -v -o $@ $<

#------------------------------------------------------------------------------
# Main rules
#------------------------------------------------------------------------------

all : $(BALL_TARG) $(PUTT_TARG) $(MAPC_TARG) sols locales

$(BALL_TARG) : $(BALL_OBJS)
	$(CC) $(CFLAGS) -o $(BALL_TARG) $(BALL_OBJS) $(LIBS)

$(PUTT_TARG) : $(PUTT_OBJS)
	$(CC) $(CFLAGS) -o $(PUTT_TARG) $(PUTT_OBJS) $(LIBS)

$(MAPC_TARG) : $(MAPC_OBJS)
	$(CC) $(CFLAGS) -o $(MAPC_TARG) $(MAPC_OBJS) $(BASE_LIBS)

sols : $(SOLS)

locales : $(MESSAGES)

clean-src :
	rm -f $(BALL_TARG) $(BALL_OBJS) $(BALL_DEPS)
	rm -f $(PUTT_TARG) $(PUTT_OBJS) $(PUTT_DEPS)
	rm -f $(MAPC_TARG) $(MAPC_OBJS) $(MAPC_DEPS)

clean : clean-src
	rm -f $(SOLS)
	rm -rf $(LOCALEDIR)

test : all
	./neverball

tools :
	cd tools && $(MAKE)

#------------------------------------------------------------------------------
# PO update rules
#------------------------------------------------------------------------------

po/%.po : $(POTFILE)
	msgmerge -U $@ $<
	touch $@
	
po-update-extract :
	sh scripts/extractpo.sh $(POTFILE) $(LOCALEDOM)

po-update-merge : $(POS)

po-update : po-update-extract po-update-merge

#------------------------------------------------------------------------------

.PHONY : all sols locales clean-src clean test \
	po-update-extract po-update-merge po-update \
	tools

-include $(BALL_DEPS) $(PUTT_DEPS) $(MAPC_DEPS)
