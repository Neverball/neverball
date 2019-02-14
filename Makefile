
#------------------------------------------------------------------------------

BUILD := $(shell head -n1 .build 2> /dev/null || echo release)

VERSION := 1.6.0
VERSION := $(shell sh scripts/version.sh "$(BUILD)" "$(VERSION)" \
	"share/version.in.h" "share/version.h" ".version")

$(info Will make a "$(BUILD)" build of Neverball $(VERSION).)

#------------------------------------------------------------------------------
# Provide a target system hint for the Makefile.
# Recognized PLATFORM values: darwin, mingw.

ifeq ($(shell uname), Darwin)
	PLATFORM := darwin
endif

ifeq ($(shell uname -o),Msys)
	PLATFORM := mingw
endif

#------------------------------------------------------------------------------
# Paths (packagers might want to set DATADIR and LOCALEDIR)

USERDIR   := .neverball
DATADIR   := ./data
LOCALEDIR := ./locale

ifeq ($(PLATFORM),mingw)
	USERDIR := Neverball
endif

ifneq ($(BUILD),release)
	USERDIR := $(USERDIR)-dev
endif

#------------------------------------------------------------------------------
# Optional flags (CFLAGS, CPPFLAGS, ...)

ifeq ($(DEBUG),1)
	CFLAGS   := -g
	CXXFLAGS := -g
	CPPFLAGS :=
else
	CFLAGS   := -O2
	CXXFLAGS := -O2
	CPPFLAGS := -DNDEBUG
endif

#------------------------------------------------------------------------------
# Mandatory flags

# Compiler...

ifeq ($(ENABLE_TILT),wii)
	# -std=c99 because we need isnormal and -fms-extensions because
	# libwiimote headers make heavy use of the "unnamed fields" GCC
	# extension.

	ALL_CFLAGS := -Wall -Wshadow -std=c99 -pedantic -fms-extensions $(CFLAGS)
else
	ALL_CFLAGS := -Wall -Wshadow -std=c99 -pedantic $(CFLAGS)
endif

ALL_CXXFLAGS := -fno-rtti -fno-exceptions $(CXXFLAGS)

# Preprocessor...

SDL_CPPFLAGS := $(shell sdl2-config --cflags)
PNG_CPPFLAGS := $(shell libpng-config --cflags)

ALL_CPPFLAGS := $(SDL_CPPFLAGS) $(PNG_CPPFLAGS) -Ishare

ALL_CPPFLAGS += \
	-DCONFIG_USER=\"$(USERDIR)\" \
	-DCONFIG_DATA=\"$(DATADIR)\" \
	-DCONFIG_LOCALE=\"$(LOCALEDIR)\"

ifeq ($(ENABLE_OPENGLES),1)
	ALL_CPPFLAGS += -DENABLE_OPENGLES=1
endif

ifeq ($(ENABLE_NLS),0)
	ALL_CPPFLAGS += -DENABLE_NLS=0
else
	ALL_CPPFLAGS += -DENABLE_NLS=1
endif

ifeq ($(ENABLE_HMD),openhmd)
	ALL_CPPFLAGS += -DENABLE_HMD=1
endif
ifeq ($(ENABLE_HMD),libovr)
	ALL_CPPFLAGS += -DENABLE_HMD=1
endif

ifeq ($(ENABLE_RADIANT_CONSOLE),1)
	ALL_CPPFLAGS += -DENABLE_RADIANT_CONSOLE=1
endif

ifeq ($(PLATFORM),darwin)
	ALL_CPPFLAGS += $(patsubst %, -I%, $(wildcard /opt/local/include \
	                                              /usr/local/include))
endif

ALL_CPPFLAGS += $(CPPFLAGS)

#------------------------------------------------------------------------------
# HMD handling is a complicated with 6 platform-backend combinations.

ifeq ($(ENABLE_HMD),openhmd)
	HMD_LIBS := -lopenhmd
endif

ifeq ($(ENABLE_HMD),libovr)
	HMD_LIBS     := -L/usr/local/OculusSDK/LibOVR/Lib/Linux/Release/x86_64 -lovr -ludev -lX11 -lXinerama
	HMD_CPPFLAGS := -I/usr/local/OculusSDK/LibOVR/Include

	ifeq ($(PLATFORM),mingw)
		HMD_LIBS     := -L/usr/local/OculusSDK/LibOVR/Lib/MinGW/Release/w32 -lovr -lsetupapi -lwinmm
		HMD_CPPFLAGS := -I/usr/local/OculusSDK/LibOVR/Include
	endif
	ifeq ($(PLATFORM),darwin)
		HMD_LIBS     := -L/usr/local/OculusSDK/LibOVR/Lib/MacOS/Release -lovr -framework IOKit -framework CoreFoundation -framework ApplicationServices
		HMD_CPPFLAGS := -I/usr/local/OculusSDK/LibOVR/Include
	endif
endif

ALL_CPPFLAGS += $(HMD_CPPFLAGS)

#------------------------------------------------------------------------------
# Libraries

SDL_LIBS := $(shell sdl2-config --libs)
PNG_LIBS := $(shell libpng-config --libs)

ENABLE_FS := stdio
ifeq ($(ENABLE_FS),stdio)
FS_LIBS :=
endif
ifeq ($(ENABLE_FS),physfs)
FS_LIBS := -lphysfs
endif

# The  non-conditionalised values  below  are specific  to the  native
# system. The native system of this Makefile is Linux (or GNU+Linux if
# you prefer). Please be sure to  override ALL of them for each target
# system in the conditional parts below.

INTL_LIBS :=

ifeq ($(ENABLE_TILT),wii)
	TILT_LIBS := -lcwiimote -lbluetooth
else
ifeq ($(ENABLE_TILT),loop)
	TILT_LIBS := -lusb-1.0 -lfreespace
else
ifeq ($(ENABLE_TILT),leapmotion)
	TILT_LIBS := /usr/lib/Leap/libLeap.so -Wl,-rpath,/usr/lib/Leap
endif
endif
endif

ifeq ($(ENABLE_OPENGLES),1)
	OGL_LIBS := -lGLESv1_CM
else
	OGL_LIBS := -lGL
endif

ifeq ($(PLATFORM),mingw)
	ifneq ($(ENABLE_NLS),0)
		INTL_LIBS := -lintl
	endif

	TILT_LIBS :=
	OGL_LIBS  := -lopengl32
endif

ifeq ($(PLATFORM),darwin)
	ifneq ($(ENABLE_NLS),0)
		INTL_LIBS := -lintl
	endif

	TILT_LIBS :=
	OGL_LIBS  := -framework OpenGL
endif

BASE_LIBS := -ljpeg $(PNG_LIBS) $(FS_LIBS) -lm

ifeq ($(PLATFORM),darwin)
	BASE_LIBS += $(patsubst %, -L%, $(wildcard /opt/local/lib \
	                                           /usr/local/lib))
endif

OGG_LIBS := -lvorbisfile
TTF_LIBS := -lSDL2_ttf

ALL_LIBS := $(HMD_LIBS) $(TILT_LIBS) $(INTL_LIBS) $(TTF_LIBS) \
	$(OGG_LIBS) $(SDL_LIBS) $(OGL_LIBS) $(BASE_LIBS)

MAPC_LIBS := $(BASE_LIBS)

ifeq ($(ENABLE_RADIANT_CONSOLE),1)
	MAPC_LIBS += -lSDL2_net
endif

#------------------------------------------------------------------------------

ifeq ($(PLATFORM),mingw)
X := .exe
endif

MAPC_TARG := mapc$(X)
BALL_TARG := neverball$(X)
PUTT_TARG := neverputt$(X)

ifeq ($(PLATFORM),mingw)
	MAPC := $(WINE) ./$(MAPC_TARG)
else
	MAPC := ./$(MAPC_TARG)
endif

#------------------------------------------------------------------------------

MAPC_OBJS := \
	share/vec3.o        \
	share/base_image.o  \
	share/solid_base.o  \
	share/binary.o      \
	share/base_config.o \
	share/common.o      \
	share/fs_common.o   \
	share/fs_png.o      \
	share/fs_jpg.o      \
	share/dir.o         \
	share/array.o       \
	share/list.o        \
	share/mapc.o
BALL_OBJS := \
	share/lang.o        \
	share/st_common.o   \
	share/vec3.o        \
	share/base_image.o  \
	share/image.o       \
	share/solid_base.o  \
	share/solid_vary.o  \
	share/solid_draw.o  \
	share/solid_all.o   \
	share/mtrl.o        \
	share/part.o        \
	share/geom.o        \
	share/ball.o        \
	share/gui.o         \
	share/font.o        \
	share/theme.o       \
	share/base_config.o \
	share/config.o      \
	share/video.o       \
	share/glext.o       \
	share/binary.o      \
	share/state.o       \
	share/audio.o       \
	share/text.o        \
	share/common.o      \
	share/list.o        \
	share/queue.o       \
	share/cmd.o         \
	share/array.o       \
	share/dir.o         \
	share/fbo.o         \
	share/glsl.o        \
	share/fs_common.o   \
	share/fs_png.o      \
	share/fs_jpg.o      \
	share/fs_ov.o       \
	share/log.o         \
	share/joy.o         \
	ball/hud.o          \
	ball/game_common.o  \
	ball/game_client.o  \
	ball/game_server.o  \
	ball/game_proxy.o   \
	ball/game_draw.o    \
	ball/score.o        \
	ball/level.o        \
	ball/progress.o     \
	ball/set.o          \
	ball/demo.o         \
	ball/demo_dir.o     \
	ball/util.o         \
	ball/st_conf.o      \
	ball/st_demo.o      \
	ball/st_save.o      \
	ball/st_goal.o      \
	ball/st_fail.o      \
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
	ball/st_ball.o      \
	ball/main.o
PUTT_OBJS := \
	share/lang.o        \
	share/st_common.o   \
	share/vec3.o        \
	share/base_image.o  \
	share/image.o       \
	share/solid_base.o  \
	share/solid_vary.o  \
	share/solid_draw.o  \
	share/solid_all.o   \
	share/mtrl.o        \
	share/part.o        \
	share/geom.o        \
	share/ball.o        \
	share/base_config.o \
	share/config.o      \
	share/video.o       \
	share/glext.o       \
	share/binary.o      \
	share/audio.o       \
	share/state.o       \
	share/gui.o         \
	share/font.o        \
	share/theme.o       \
	share/text.o        \
	share/common.o      \
	share/list.o        \
	share/fs_common.o   \
	share/fs_png.o      \
	share/fs_jpg.o      \
	share/fs_ov.o       \
	share/dir.o         \
	share/fbo.o         \
	share/glsl.o        \
	share/array.o       \
	share/log.o         \
	share/joy.o         \
	putt/hud.o          \
	putt/game.o         \
	putt/hole.o         \
	putt/course.o       \
	putt/st_all.o       \
	putt/st_conf.o      \
	putt/main.o

BALL_OBJS += share/solid_sim_sol.o
PUTT_OBJS += share/solid_sim_sol.o

ifeq ($(ENABLE_FS),stdio)
BALL_OBJS += share/fs_stdio.o
PUTT_OBJS += share/fs_stdio.o
MAPC_OBJS += share/fs_stdio.o
endif
ifeq ($(ENABLE_FS),physfs)
BALL_OBJS += share/fs_physfs.o
PUTT_OBJS += share/fs_physfs.o
MAPC_OBJS += share/fs_physfs.o
endif

ifeq ($(ENABLE_TILT),wii)
BALL_OBJS += share/tilt_wii.o
else
ifeq ($(ENABLE_TILT),loop)
BALL_OBJS += share/tilt_loop.o
else
ifeq ($(ENABLE_TILT),leapmotion)
BALL_OBJS += share/tilt_leapmotion.o
else
BALL_OBJS += share/tilt_null.o
endif
endif
endif

ifeq ($(ENABLE_HMD),openhmd)
BALL_OBJS += share/hmd_openhmd.o share/hmd_common.o
PUTT_OBJS += share/hmd_openhmd.o share/hmd_common.o
else
ifeq ($(ENABLE_HMD),libovr)
BALL_OBJS += share/hmd_libovr.o share/hmd_common.o
PUTT_OBJS += share/hmd_libovr.o share/hmd_common.o
else
BALL_OBJS += share/hmd_null.o
PUTT_OBJS += share/hmd_null.o
endif
endif

ifeq ($(PLATFORM),mingw)
BALL_OBJS += neverball.ico.o
PUTT_OBJS += neverputt.ico.o
endif

BALL_DEPS := $(BALL_OBJS:.o=.d)
PUTT_DEPS := $(PUTT_OBJS:.o=.d)
MAPC_DEPS := $(MAPC_OBJS:.o=.d)

MAPS := $(shell find data -name "*.map" \! -name "*.autosave.map")
SOLS := $(MAPS:%.map=%.sol)

DESKTOPS := $(basename $(wildcard dist/*.desktop.in))

# The build environment defines this (or should).
# This is a fallback that likely only works on a Windows host.
WINDRES ?= windres

#------------------------------------------------------------------------------

%.o : %.c
	$(CC) $(ALL_CFLAGS) $(ALL_CPPFLAGS) -MM -MP -MF $*.d -MT "$@" $<
	$(CC) $(ALL_CFLAGS) $(ALL_CPPFLAGS) -o $@ -c $<

%.o : %.cpp
	$(CXX) $(ALL_CXXFLAGS) $(ALL_CPPFLAGS) -MM -MP -MF $*.d -MT "$@" $<
	$(CXX) $(ALL_CXXFLAGS) $(ALL_CPPFLAGS) -o $@ -c $<

%.sol : %.map $(MAPC_TARG)
	$(MAPC) $< data

%.desktop : %.desktop.in
	sh scripts/translate-desktop.sh < $< > $@

%.ico.o: dist/ico/%.ico
	echo "1 ICON \"$<\"" | $(WINDRES) -o $@

#------------------------------------------------------------------------------

all : $(BALL_TARG) $(PUTT_TARG) $(MAPC_TARG) sols locales desktops

ifeq ($(ENABLE_HMD),libovr)
LINK := $(CXX) $(ALL_CXXFLAGS)
else
ifeq ($(ENABLE_TILT),leapmotion)
LINK := $(CXX) $(ALL_CXXFLAGS)
else
LINK := $(CC) $(ALL_CFLAGS)
endif
endif

$(BALL_TARG) : $(BALL_OBJS)
	$(LINK) -o $(BALL_TARG) $(BALL_OBJS) $(LDFLAGS) $(ALL_LIBS)

$(PUTT_TARG) : $(PUTT_OBJS)
	$(LINK) -o $(PUTT_TARG) $(PUTT_OBJS) $(LDFLAGS) $(ALL_LIBS)

$(MAPC_TARG) : $(MAPC_OBJS)
	$(CC) $(ALL_CFLAGS) -o $(MAPC_TARG) $(MAPC_OBJS) $(LDFLAGS) $(MAPC_LIBS)

# Work around some extremely helpful sdl-config scripts.

ifeq ($(PLATFORM),mingw)
$(MAPC_TARG) : ALL_CPPFLAGS := $(ALL_CPPFLAGS) -Umain
endif

sols : $(SOLS)

locales :
ifneq ($(ENABLE_NLS),0)
	$(MAKE) -C po
endif

desktops : $(DESKTOPS)

clean-src :
	$(RM) $(BALL_TARG) $(PUTT_TARG) $(MAPC_TARG)
	find . \( -name '*.o' -o -name '*.d' \) -delete

clean : clean-src
	$(RM) $(SOLS)
	$(RM) $(DESKTOPS)
	$(MAKE) -C po clean

#------------------------------------------------------------------------------

.PHONY : all sols locales desktops clean-src clean

-include $(BALL_DEPS) $(PUTT_DEPS) $(MAPC_DEPS)

#------------------------------------------------------------------------------
