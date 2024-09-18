# Build official and addon packages of all types.

DATA_DIR ?= ./data
PACKAGES_DIR ?= ../neverball-packages
OUTPUT_DIR ?= $(shell pwd)/packages

SET_FILES := $(filter-out $(DATA_DIR)/set-misc.txt, $(wildcard $(DATA_DIR)/set-*.txt))
SET_PACKAGES := $(basename $(notdir $(SET_FILES)))

BALL_DIRS := $(filter-out $(DATA_DIR)/ball/basic-ball, $(wildcard $(DATA_DIR)/ball/*))
BALL_PACKAGES := $(addprefix ball-,$(notdir $(BALL_DIRS)))

ADDON_SET_DIRS := $(wildcard $(PACKAGES_DIR)/set-*)
ADDON_SET_PACKAGES := $(basename $(notdir $(ADDON_SET_DIRS)))

MANIFEST := $(OUTPUT_DIR)/available-packages.txt

all: base $(SET_PACKAGES) $(BALL_PACKAGES) $(ADDON_SET_PACKAGES)
	find $(OUTPUT_DIR) -name '*.manifest.txt' | LC_ALL=C sort | xargs cat > $(MANIFEST)

base:
	$(MAKE) -f mk/package-base.mk OUTPUT_DIR=$(OUTPUT_DIR)

$(SET_PACKAGES): %: $(DATA_DIR)/%.txt
	$(MAKE) -f mk/package-levelset.mk ADDON_SET=0 PACKAGE_ID=$@ SET_FILE=$< DATA_DIR=$(DATA_DIR) OUTPUT_DIR=$(OUTPUT_DIR)

$(BALL_PACKAGES): ball-%: $(DATA_DIR)/ball/%
	$(MAKE) -f mk/package-ball.mk PACKAGE_ID=$@ BALL_DIR="ball/$*" DATA_DIR=$(DATA_DIR) OUTPUT_DIR=$(OUTPUT_DIR)

$(ADDON_SET_PACKAGES): %: $(PACKAGES_DIR)/%
	$(MAKE) -f mk/package-levelset.mk ADDON_SET=1 PACKAGE_ID=$@ SET_FILE=$</$(notdir $<).txt DATA_DIR=$< OUTPUT_DIR=$(OUTPUT_DIR)

mp3s:
	find $(DATA_DIR) $(PACKAGES_DIR) -name '*.ogg' | while read ogg; do \
		mp3=$(OUTPUT_DIR)/mp3/$$(echo $$ogg | sed -E 's,^\$(DATA_DIR)/,,' | sed -E 's,^\$(PACKAGES_DIR)/[^/]+/,,' | sed 's/\.ogg/.mp3/'); \
		mkdir -p $$(dirname $$mp3); \
		ffmpeg -nostdin -y -loglevel error -i $$ogg -acodec libmp3lame $$mp3; \
	done

.PHONY: all base $(SET_PACKAGES) $(BALL_PACKAGES) $(ADDON_SET_PACKAGES) mp3s

GNUMAKEFLAGS = --no-print-directory