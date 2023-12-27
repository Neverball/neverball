# Make a Neverball package. SOLs must already be compiled.
#
# This also creates a baseline manifest with package name, filename and size.
#
# Usage:
#
#   make -f package.mk PACKAGE_ID=set-easy DATA_DIR=./data ASSET_PATHS="set-easy.txt map-easy shot-easy"

ifndef PACKAGE_ID
$(error PACKAGE_ID is missing. Example: [type]-[name], e.g., set-easy, ball-ufo, base-neverball, etc.)
endif

ifndef DATA_DIR
$(error DATA_DIR is missing. Example: ./data or ../neverball-packages/set-bud)
endif

ifndef ASSET_PATHS
$(error ASSET_PATHS is missing. Files and folders are OK; must be relative to DATA_DIR)
endif

OUTPUT_DIR ?= $(shell pwd)/packages

# TODO: build separate source packages
PACKAGE_EXCLUDE := \
	'*.xcf' \
	'*.wings' \
	'screenshot.png'

LOCAL_PATHS := $(ASSET_PATHS:%=$(DATA_DIR)/%) # prefix with ./data/
LOCAL_FILES := $(shell find $(LOCAL_PATHS) -type f) # find files to hash
ASSET_HASH := $(shell sh scripts/hash-assets.sh $(LOCAL_PATHS))
PACKAGE_ZIP := $(PACKAGE_ID)-$(ASSET_HASH).zip
PACKAGE_TMP := /tmp/$(PACKAGE_ID)
PACKAGE_MANIFEST := $(PACKAGE_ID).manifest.txt

all: output-dir package manifest

output-dir:
	mkdir -p $(OUTPUT_DIR)

package: output-dir
	cd $(DATA_DIR) && \
	zip -X -q -r $(OUTPUT_DIR)/$(PACKAGE_ZIP) $(ASSET_PATHS) -x $(PACKAGE_EXCLUDE) && \
	rm -rf "$(PACKAGE_TMP)" && \
	mkdir "$(PACKAGE_TMP)" && \
	( \
		cd "$(PACKAGE_TMP)" && \
		unzip -q $(OUTPUT_DIR)/$(PACKAGE_ZIP) && \
		find . -execdir touch -t 200305010000 {} \; && \
		rm $(OUTPUT_DIR)/$(PACKAGE_ZIP) && \
		zip -X -q -r $(OUTPUT_DIR)/$(PACKAGE_ZIP) . \
	) && \
	rm -rf "$(PACKAGE_TMP)"

manifest: output-dir package
	printf 'package %s\nfilename %s\nsize %s\n' $(PACKAGE_ID) $(PACKAGE_ZIP) $(shell du -b $(OUTPUT_DIR)/$(PACKAGE_ZIP) | cut -f1) > $(OUTPUT_DIR)/$(PACKAGE_MANIFEST)

.PHONY: all output-dir package manifest