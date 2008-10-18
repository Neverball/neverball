
#------------------------------------------------------------------------------

VERSION := $(shell sh scripts/version.sh)
ifeq ($(VERSION),unknown)
    $(warning Failed to obtain sane version for this build.)
endif

# Provide a target system hint for the Makefile.

ifeq ($(shell uname), Darwin)
    DARWIN := 1
endif

#------------------------------------------------------------------------------
# Optional flags (CFLAGS, CPPFLAGS, ...)

ifeq ($(ENABLE_WII),1)
    # libwiimote is NOT ANSI compliant (TODO, check if this is necessary.  GCC
    # is supposed to suppress warnings from system headers.)

    ifneq ($(DEBUG),1)
        CFLAGS   := -g
        CPPFLAGS :=
    else
        CFLAGS   := -O2
        CPPFLAGS := -DNDEBUG
    endif
else
    ifeq ($(DEBUG),1)
        CFLAGS   := -Wall -g -ansi -pedantic
        CPPFLAGS :=
    else
        CFLAGS   := -Wall -O2 -ansi -pedantic
        CPPFLAGS := -DNDEBUG
    endif
endif

#------------------------------------------------------------------------------
# Mandatory flags

# Compiler...

ALL_CFLAGS := $(CFLAGS)

# Preprocessor...

SDL_CPPFLAGS := $(shell sdl-config --cflags)
PNG_CPPFLAGS := $(shell libpng-config --cflags)

ALL_CPPFLAGS := $(SDL_CPPFLAGS) $(PNG_CPPFLAGS) -Ishare \
    -DVERSION=\"$(VERSION)\"

ifeq ($(ENABLE_NLS),0)
    ALL_CPPFLAGS += -DENABLE_NLS=0
else
    ALL_CPPFLAGS += -DENABLE_NLS=1
endif

ifeq ($(ENABLE_WII),1)
    ALL_CPPFLAGS += -DENABLE_WII=1
endif

ifdef DARWIN
    ALL_CPPFLAGS += -I/opt/local/include
endif

ALL_CPPFLAGS += $(CPPFLAGS)

#------------------------------------------------------------------------------
# Libraries

SDL_LIBS := $(shell sdl-config --libs)
PNG_LIBS := $(shell libpng-config --libs)

# The  non-conditionalised values  below  are specific  to the  native
# system. The native system of this Makefile is Linux (or GNU+Linux if
# you prefer). Please be sure to  override ALL of them for each target
# system in the conditional parts below.

INTL_LIBS :=

ifeq ($(ENABLE_WII),1)
    TILT_LIBS := -lcwiimote -lbluetooth
endif

OGL_LIBS := -lGL -lm

ifdef MINGW
    ifneq ($(ENABLE_NLS),0)
        INTL_LIBS := -lintl -liconv
    endif

    TILT_LIBS :=
    OGL_LIBS  := -lopengl32 -lm
endif

ifdef DARWIN
    ifneq ($(ENABLE_NLS),0)
        INTL_LIBS := -lintl -liconv
    endif

    TILT_LIBS :=
    OGL_LIBS  := -framework OpenGL
endif

BASE_LIBS := -ljpeg $(PNG_LIBS)

ifdef DARWIN
    BASE_LIBS += -L/opt/local/lib
endif

ALL_LIBS := $(SDL_LIBS) $(BASE_LIBS) $(TILT_LIBS) $(INTL_LIBS) -lSDL_ttf \
    -lvorbisfile $(OGL_LIBS)

#------------------------------------------------------------------------------

ifdef MINGW
    EXT := .exe
endif

MAPC_TARG := mapc$(EXT)
BALL_TARG := neverball$(EXT)
PUTT_TARG := neverputt$(EXT)

ifdef MINGW
    MAPC := $(WINE) ./$(MAPC_TARG)
else
    MAPC := ./$(MAPC_TARG)
endif


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
	share/item.o        \
	share/ball.o        \
	share/gui.o         \
	share/base_config.o \
	share/config.o      \
	share/binary.o      \
	share/state.o       \
	share/audio.o       \
	share/text.o        \
	share/sync.o        \
	share/tilt.o        \
	share/common.o      \
	share/keynames.o    \
	share/syswm.o       \
	ball/hud.o          \
	ball/game.o         \
	ball/score.o        \
	ball/level.o        \
	ball/progress.o     \
	ball/set.o          \
	ball/demo.o         \
	ball/util.o         \
	ball/st_conf.o      \
	ball/st_demo.o      \
	ball/st_save.o      \
	ball/st_goal.o      \
	ball/st_fall_out.o  \
	ball/st_time_out.o  \
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
	share/ball.o        \
	share/back.o        \
	share/base_config.o \
	share/config.o      \
	share/binary.o      \
	share/audio.o       \
	share/state.o       \
	share/gui.o         \
	share/text.o        \
	share/sync.o        \
	share/common.o      \
	share/syswm.o       \
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

MAPS := $(shell find data -name "*.map" \! -name "*.autosave.map")
SOLS := $(MAPS:%.map=%.sol)

DESKTOPS := $(basename $(wildcard dist/*.desktop.in))

#------------------------------------------------------------------------------

%.o : %.c
	$(CC) $(ALL_CFLAGS) $(ALL_CPPFLAGS) -MM -MP -MF $*.d -MT "$@" $<
	$(CC) $(ALL_CFLAGS) $(ALL_CPPFLAGS) -o $@ -c $<

%.sol : %.map $(MAPC_TARG)
	$(MAPC) $< data

%.desktop : %.desktop.in
	sh scripts/translate-desktop.sh < $< > $@

#------------------------------------------------------------------------------

all : $(BALL_TARG) $(PUTT_TARG) $(MAPC_TARG) sols locales desktops

$(BALL_TARG) : $(BALL_OBJS)
	$(CC) $(ALL_CFLAGS) -o $(BALL_TARG) $(BALL_OBJS) $(LDFLAGS) $(ALL_LIBS)

$(PUTT_TARG) : $(PUTT_OBJS)
	$(CC) $(ALL_CFLAGS) -o $(PUTT_TARG) $(PUTT_OBJS) $(LDFLAGS) $(ALL_LIBS)

$(MAPC_TARG) : $(MAPC_OBJS)
	$(CC) $(ALL_CFLAGS) -o $(MAPC_TARG) $(MAPC_OBJS) $(LDFLAGS) $(BASE_LIBS)

# Work around some extremely helpful sdl-config scripts.

ifdef MINGW
$(MAPC_TARG) : ALL_CPPFLAGS := $(ALL_CPPFLAGS) -Umain
endif

sols : $(SOLS)

locales :
ifneq ($(ENABLE_NLS),0)
	$(MAKE) -C po
endif

desktops : $(DESKTOPS)

clean-src :
	$(RM) $(BALL_TARG) $(BALL_OBJS) $(BALL_DEPS)
	$(RM) $(PUTT_TARG) $(PUTT_OBJS) $(PUTT_DEPS)
	$(RM) $(MAPC_TARG) $(MAPC_OBJS) $(MAPC_DEPS)

clean : clean-src
	$(RM) $(SOLS)
	$(RM) $(DESKTOPS)
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

#------------------------------------------------------------------------------

.PHONY: setup
setup: $(INSTALLER)

$(INSTALLER): install-dlls convert-text-files all contrib
	$(MAKENSIS) $(MAKENSIS_FLAGS) -nocd scripts/neverball.nsi

$(INSTALLER): LDFLAGS := -s $(LDFLAGS)

.PHONY: clean-setup
clean-setup: clean
	$(RM) install-dlls.sh *.dll $(TXT_DOCS)
	find data -name "*.txt" -exec $(FROMDOS) {} \;
	$(MAKE) -C contrib EXT=$(EXT) clean

#------------------------------------------------------------------------------

.PHONY: install-dlls
install-dlls: install-dlls.sh
	sh $<

install-dlls.sh:
	mingw-list-dlls --sh > $@
	@echo --------------------------------------------------------
	@echo Now edit $@ to your needs before restarting make.
	@echo --------------------------------------------------------
	@exit 1

#------------------------------------------------------------------------------

.PHONY: convert-text-files
convert-text-files: $(TXT_DOCS)
	find data -name "*.txt" -exec $(TODOS) {} \;

%.txt: %
	$(CP) $< $@
	$(TODOS) $@

#------------------------------------------------------------------------------

.PHONY: contrib
contrib:
	$(MAKE) -C contrib EXT=$(EXT)

#------------------------------------------------------------------------------

endif

#------------------------------------------------------------------------------
