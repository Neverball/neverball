# Make a minimal Neverball base data package. SOLs must already be compiled.

DATA_DIR := ./data
OUTPUT_DIR ?= $(shell pwd)/packages

ASSET_PATHS := \
	back \
	ball/basic-ball \
	bgm \
	courses.txt \
	geom \
	gui \
	icon \
	item \
	lang \
	lights.txt \
	map-back \
	png \
	sets.txt \
	snd \
	textures \
	ttf

BASE_SCREENSHOT := $(DATA_DIR)/shot-medium/title.jpg # just some file

PACKAGE_ID := base-neverball
PACKAGE_FILES := sets.txt courses.txt # just some files
PACKAGE_NAME := Neverball
PACKAGE_DESC := Neverball base files.
PACKAGE_SCREENSHOT := base-neverball-$(shell md5sum $(BASE_SCREENSHOT) | cut -c -32)$(suffix $(BASE_SCREENSHOT))
PACKAGE_MANIFEST := $(PACKAGE_ID).manifest.txt

all: package manifest screenshot

package:
	$(MAKE) -f mk/package.mk PACKAGE_ID=base-neverball DATA_DIR=$(DATA_DIR) ASSET_PATHS="$(ASSET_PATHS)"

.PHONY: package-only
package-only:
	$(MAKE) -f mk/package.mk PACKAGE_ID=base-neverball DATA_DIR=$(DATA_DIR) ASSET_PATHS="$(ASSET_PATHS)" package

manifest: package
	printf 'files %s\nname %s\ndesc %s\nshot %s\n' \
		"$(PACKAGE_FILES)" \
		"$(PACKAGE_NAME)" \
		"$(PACKAGE_DESC)" \
		$(PACKAGE_SCREENSHOT) >> $(OUTPUT_DIR)/$(PACKAGE_MANIFEST)

screenshot: $(BASE_SCREENSHOT)
	cp $(BASE_SCREENSHOT) $(OUTPUT_DIR)/$(PACKAGE_SCREENSHOT)

.PHONY: all package manifest screenshot

GNUMAKEFLAGS = --no-print-directory