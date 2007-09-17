#-------------------------------------------------------------------------------

VERSION := $(shell sh scripts/version.sh)
ifeq ($(VERSION),)
    $(error Failed to obtain version for this build)
endif

#------------------------------------------------------------------------------

# Optional flags
CFLAGS := -Wall -g -ansi -pedantic

# Mandatory flags
SDL_CPPFLAGS := $(shell sdl-config --cflags)
PNG_CPPFLAGS := $(shell libpng-config --cflags)

ALL_CFLAGS   := $(CFLAGS)
ALL_CPPFLAGS := $(SDL_CPPFLAGS) $(PNG_CPPFLAGS) -Ishare \
    -DVERSION=\"$(VERSION)\"

ifdef DISABLE_NLS
    ALL_CPPFLAGS += -DDISABLE_NLS=1
endif

ALL_CPPFLAGS += $(CPPFLAGS)

#------------------------------------------------------------------------------

SDL_LIBS := $(shell sdl-config --libs)
PNG_LIBS := $(shell libpng-config --libs)

ifdef MINGW
ifndef DISABLE_NLS
    INTL_LIBS := -lintl
endif
endif

ifdef MINGW
    OGL_LIBS  := -lopengl32 -lm
    BASE_LIBS := -lSDL -lSDL_image $(INTL_LIBS)
    ALL_LIBS  := $(SDL_LIBS) -lSDL_image $(INTL_LIBS) \
	$(PNG_LIBS) -lSDL_ttf -lSDL_mixer $(OGL_LIBS)
else
    OGL_LIBS  := -lGL -lm
    BASE_LIBS := $(SDL_LIBS) -lSDL_image
    ALL_LIBS  := $(BASE_LIBS) $(PNG_LIBS) -lSDL_ttf -lSDL_mixer $(OGL_LIBS)
endif

#------------------------------------------------------------------------------

ifdef MINGW
    EXT  := .exe
    WINE := wine
endif

#------------------------------------------------------------------------------

MAPC_TARG := mapc$(EXT)
BALL_TARG := neverball$(EXT)
PUTT_TARG := neverputt$(EXT)

#------------------------------------------------------------------------------

MAPC_OBJS := \
	share/vec3.o        \
	share/base_image.o  \
	share/solid.o       \
	share/binary.o      \
	share/base_config.o \
	share/mapc.o
BALL_OBJS := \
	share/lang.o        \
	share/st_resol.o    \
	share/vec3.o        \
	share/base_image.o  \
	share/image.o       \
	share/solid.o       \
	share/solid_gl.o    \
	share/part.o        \
	share/back.o        \
	share/geom.o        \
	share/gui.o         \
	share/base_config.o \
	share/config.o      \
	share/binary.o      \
	share/state.o       \
	share/audio.o       \
	ball/hud.o          \
	ball/mode.o         \
	ball/game.o         \
	ball/score.o        \
	ball/level.o        \
	ball/levels.o       \
	ball/set.o          \
	ball/demo.o         \
	ball/util.o         \
	ball/st_conf.o      \
	ball/st_demo.o      \
	ball/st_save.o      \
	ball/st_play_end.o  \
	ball/st_done.o      \
	ball/st_level.o     \
	ball/st_over.o      \
	ball/st_play.o      \
	ball/st_set.o       \
	ball/st_start.o     \
	ball/st_title.o     \
	ball/st_help.o      \
	ball/st_name.o      \
	ball/st_shared.o    \
	ball/st_pause.o     \
	ball/main.o
PUTT_OBJS := \
	share/lang.o        \
	share/st_resol.o    \
	share/vec3.o        \
	share/base_image.o  \
	share/image.o       \
	share/solid.o       \
	share/solid_gl.o    \
	share/part.o        \
	share/geom.o        \
	share/back.o        \
	share/base_config.o \
	share/config.o      \
	share/binary.o      \
	share/audio.o       \
	share/state.o       \
	share/gui.o         \
	putt/hud.o          \
	putt/game.o         \
	putt/hole.o         \
	putt/course.o       \
	putt/st_all.o       \
	putt/st_conf.o      \
	putt/main.o

BALL_DEPS := $(BALL_OBJS:.o=.d)
PUTT_DEPS := $(PUTT_OBJS:.o=.d)
MAPC_DEPS := $(MAPC_OBJS:.o=.d)

MAPS := $(shell find data -name "*.map")
SOLS := $(MAPS:%.map=%.sol)

#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(ALL_CFLAGS) $(ALL_CPPFLAGS) -MM -MF $@ -MT '$*.o $@' $<

%.o : %.c
	$(CC) $(ALL_CFLAGS) $(ALL_CPPFLAGS) -o $@ -c $<

%.sol : %.map $(MAPC_TARG)
	$(WINE) ./$(MAPC_TARG) $< data

#------------------------------------------------------------------------------

all : $(BALL_TARG) $(PUTT_TARG) $(MAPC_TARG) sols locales

$(BALL_TARG) : $(BALL_OBJS)
	$(CC) $(ALL_CFLAGS) -o $(BALL_TARG) $(BALL_OBJS) $(LDFLAGS) $(ALL_LIBS)

$(PUTT_TARG) : $(PUTT_OBJS)
	$(CC) $(ALL_CFLAGS) -o $(PUTT_TARG) $(PUTT_OBJS) $(LDFLAGS) $(ALL_LIBS)

$(MAPC_TARG) : $(MAPC_OBJS)
	$(CC) $(ALL_CFLAGS) -o $(MAPC_TARG) $(MAPC_OBJS) $(LDFLAGS) $(BASE_LIBS)

sols : $(SOLS)

locales :
ifndef DISABLE_NLS
	$(MAKE) -C po
endif

clean-src :
	$(RM) $(BALL_TARG) $(BALL_OBJS) $(BALL_DEPS)
	$(RM) $(PUTT_TARG) $(PUTT_OBJS) $(PUTT_DEPS)
	$(RM) $(MAPC_TARG) $(MAPC_OBJS) $(MAPC_DEPS)

clean : clean-src
	$(RM) $(SOLS)
	$(MAKE) -C po clean

test : all
	./neverball

#------------------------------------------------------------------------------

.PHONY : all sols locales clean-src clean test

-include $(BALL_DEPS) $(PUTT_DEPS) $(MAPC_DEPS)

#------------------------------------------------------------------------------

ifdef MINGW

#------------------------------------------------------------------------------

INSTALLER := ../neverball-$(VERSION)-setup.exe

MAKENSIS := makensis
MAKENSIS_FLAGS := -DVERSION=$(VERSION) -DOUTFILE=$(INSTALLER)

TODOS   := todos
FROMDOS := fromdos

CP := cp

TEXT_DOCS := \
	doc/AUTHORS \
	doc/MANUAL  \
	CHANGES     \
	COPYING     \
	README

TXT_DOCS := $(TEXT_DOCS:%=%.txt)

#-----------------------------------------------------------------------------

.PHONY: setup
setup: $(INSTALLER)

$(INSTALLER): install-dlls convert-text-files all tools
	$(MAKENSIS) $(MAKENSIS_FLAGS) -nocd scripts/neverball.nsi

$(INSTALLER): LDFLAGS := -s $(LDFLAGS)

.PHONY: clean-setup
clean-setup: clean
	$(RM) install-dlls.sh *.dll $(TXT_DOCS)
	find data -name "*.txt" -exec $(FROMDOS) {} \;
	$(MAKE) -C tools EXT=$(EXT) clean

#-----------------------------------------------------------------------------

.PHONY: install-dlls
install-dlls: install-dlls.sh
	sh $<

install-dlls.sh:
	if ! sh scripts/gen-install-dlls.sh > $@; then \
	    $(RM) $@; \
	    exit 1; \
	fi
	@echo --------------------------------------------------------
	@echo You can probably ignore any file-not-found errors above.
	@echo Now edit $@ to your needs before restarting make.
	@echo --------------------------------------------------------
	@exit 1

#-----------------------------------------------------------------------------

.PHONY: convert-text-files
convert-text-files: $(TXT_DOCS)
	find data -name "*.txt" -exec $(TODOS) {} \;

%.txt: %
	$(CP) $< $@
	$(TODOS) $@

#-----------------------------------------------------------------------------

.PHONY: tools
tools:
	$(MAKE) -C tools EXT=$(EXT)

#------------------------------------------------------------------------------

endif

#------------------------------------------------------------------------------
