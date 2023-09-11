# Make a package for a level set. SOLs must already be compiled.
#
# Usage:
#
#   make -f package-levelset.mk PACKAGE_ID=set-easy SET_FILE=./data/set-easy.txt DATA_DIR=./data

ifndef SET_FILE
$(error SET_FILE is missing. ./data/set-easy.txt, ../neverball-packages/set-bud/set-bud.txt, etc.)
endif

ifndef PACKAGE_ID
$(error PACKAGE_ID is missing. Format: set-easy, etc.)
endif

ifndef DATA_DIR
$(error DATA_DIR is missing. Format: ./data, etc.)
endif

ifndef ADDON_SET
$(error ADDON_SET is missing. Format: 0 or 1)
endif

OUTPUT_DIR ?= $(shell pwd)/packages

SET_NAME := $(shell sed -n 's/\r$$//; 1 p' < $(SET_FILE))
SET_DESC := $(shell sed -n 's/\r$$//; 2 s,\\,\\n,g; 2 p' < $(SET_FILE)) # replace backslash with newline due to Makefile limitations
SET_SHOT := $(shell sed -n 's/\r$$//; 4 p' < $(SET_FILE))
SET_SOLS := $(shell sed -n 's/\r$$//; 6,$$ p' < $(SET_FILE))

ifeq ($(ADDON_SET),0)
# This is a Neverball levelset, grab the basics.
SET_SHOT_DIR := $(dir $(SET_SHOT))
SET_SOLS_DIR := $(dir $(firstword $(SET_SOLS)))
ASSET_PATHS := $(notdir $(SET_FILE)) $(SET_SHOT_DIR) $(SET_SOLS_DIR)
else
# This is an addon levelset, grab everything.
ASSET_PATHS := $(notdir $(wildcard $(DATA_DIR)/*))
endif

PACKAGE_MANIFEST := $(OUTPUT_DIR)/$(PACKAGE_ID).manifest.txt
PACKAGE_SCREENSHOT := $(OUTPUT_DIR)/$(PACKAGE_ID)-$(shell md5sum $(DATA_DIR)/$(SET_SHOT) | cut -c -32)$(suffix $(SET_SHOT))

all: package manifest screenshot

package:
	$(MAKE) -f mk/package.mk PACKAGE_ID=$(PACKAGE_ID) DATA_DIR=$(DATA_DIR) ASSET_PATHS="$(ASSET_PATHS)" OUTPUT_DIR=$(OUTPUT_DIR)

# Append stuff to the generic manifest created by package.mk
manifest: package
	printf 'files %s %s\nname %s\ndesc %s\nshot %s\n' \
		$(notdir $(SET_FILE)) "$(SET_SOLS)" "$(SET_NAME)" "$(SET_DESC)" $(notdir $(PACKAGE_SCREENSHOT)) >> $(PACKAGE_MANIFEST)

screenshot: package manifest
	cp $(DATA_DIR)/$(SET_SHOT) $(PACKAGE_SCREENSHOT)

.PHONY: all package screenshot manifest

GNUMAKEFLAGS = --no-print-directory