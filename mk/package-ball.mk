# Make a package for a ball skin. SOLs must already be compiled.
#
# Usage:
#
#   make -f package-ball.mk PACKAGE_ID=ball-basic-ball BALL_DIR=ball/basic-ball DATA_DIR=./data

ifndef BALL_DIR
$(error BALL_DIR is missing. ball/basic-ball, etc.)
endif

ifndef PACKAGE_ID
$(error PACKAGE_ID is missing. Format: ball-basic-ball, etc.)
endif

ifndef DATA_DIR
$(error DATA_DIR is missing. Format: ./data, etc.)
endif

OUTPUT_DIR ?= $(shell pwd)/packages

BALL_FILE := $(DATA_DIR)/$(BALL_DIR)/ball.txt
BALL_SHOT := $(DATA_DIR)/$(BALL_DIR)/screenshot.png
BALL_NAME := $(shell sed -n '/^name / s/^name //p' < $(BALL_FILE))

PACKAGE_MANIFEST := $(OUTPUT_DIR)/$(PACKAGE_ID).manifest.txt
PACKAGE_SCREENSHOT := $(OUTPUT_DIR)/$(PACKAGE_ID)-$(shell md5sum $(BALL_SHOT) | cut -c -32)$(suffix $(BALL_SHOT))

all: package manifest screenshot

package:
	$(MAKE) -f mk/package.mk PACKAGE_ID=$(PACKAGE_ID) DATA_DIR=$(DATA_DIR) ASSET_PATHS="$(BALL_DIR)" OUTPUT_DIR=$(OUTPUT_DIR)

# Append stuff to the generic manifest created by package.mk
manifest: package
	printf 'name %s\nshot %s\n' "$(BALL_NAME)" $(notdir $(PACKAGE_SCREENSHOT)) >> $(PACKAGE_MANIFEST)

screenshot: package manifest
	cp $(BALL_SHOT) $(PACKAGE_SCREENSHOT)

.PHONY: all package screenshot manifest

GNUMAKEFLAGS = --no-print-directory