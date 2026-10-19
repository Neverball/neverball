#!/usr/bin/env bash
#
# scripts/package-macos.sh - Build and package Neverball for macOS (arm64)
#
# Creates a standalone, self-contained Neverball.app with all dynamic libraries
# bundled, ad-hoc signed, with data packaged in base-neverball.zip, and
# mapc and neverputt bundled inside the application bundle. Optionally creates
# a distributable DMG image.

set -euo pipefail

# -----------------------------------------------------------------------------
# Configuration and Defaults
# -----------------------------------------------------------------------------

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

ARCH="${ARCH:-$(uname -m)}"
BUILD_TYPE="release"
SIGN_IDENTITY="-"
OUT_DIR="build/macos"
CREATE_ZIP=1
CREATE_DMG=0
SKIP_BUILD=0

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Builds and packages Neverball as a self-contained macOS application bundle.

Options:
  --arch <arch>         Target architecture: arm64 or x86_64 (default: host arch)
  --build-type <type>   Build configuration: release (default) or devel
  --identity <id>       Code signing identity (default: "-" for ad-hoc signing)
  --out-dir <dir>       Output directory for .app and archives (default: build/macos)
  --zip                 Create a compressed .zip archive of the .app (default: enabled)
  --no-zip              Skip creating the .zip archive
  --dmg                 Create a compressed .dmg disk image (default: disabled)
  --no-dmg              Skip creating the .dmg disk image
  --skip-build          Skip compilation and assemble bundle from existing binaries
  -h, --help            Show this help message
EOF
    exit 0
}

# Parse command line options
while [[ $# -gt 0 ]]; do
    case "$1" in
        --arch)
            ARCH="$2"
            shift 2
            ;;
        --build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --identity)
            SIGN_IDENTITY="$2"
            shift 2
            ;;
        --out-dir)
            OUT_DIR="$2"
            shift 2
            ;;
        --zip)
            CREATE_ZIP=1
            shift
            ;;
        --no-zip)
            CREATE_ZIP=0
            shift
            ;;
        --dmg)
            CREATE_DMG=1
            shift
            ;;
        --no-dmg)
            CREATE_DMG=0
            shift
            ;;
        --skip-build)
            SKIP_BUILD=1
            shift
            ;;
        -h|--help)
            usage
            ;;
        *)
            echo "Error: Unknown option: $1" >&2
            echo "Run '$(basename "$0") --help' for usage." >&2
            exit 1
            ;;
    esac
done

# Ensure OUT_DIR is an absolute path
mkdir -p "$OUT_DIR"
OUT_DIR="$(cd "$OUT_DIR" && pwd)"

# -----------------------------------------------------------------------------
# Environment & Host Verification
# -----------------------------------------------------------------------------

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "Error: This packaging script must be run on macOS (Darwin)." >&2
    exit 1
fi

if [[ "$ARCH" != "arm64" && "$ARCH" != "x86_64" ]]; then
    echo "Error: Unsupported architecture '$ARCH'. Supported: arm64, x86_64." >&2
    exit 1
fi

for cmd in clang make otool install_name_tool codesign sips iconutil python3 zip; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo "Error: Required command '$cmd' is not found in PATH." >&2
        exit 1
    fi
done

if [[ "$CREATE_ZIP" -eq 1 ]] && ! command -v ditto >/dev/null 2>&1; then
    echo "Error: 'ditto' is required to create a ZIP archive." >&2
    exit 1
fi

if [[ "$CREATE_DMG" -eq 1 ]] && ! command -v hdiutil >/dev/null 2>&1; then
    echo "Error: 'hdiutil' is required to create a DMG disk image." >&2
    exit 1
fi

VERSION="$(sh "$REPO_ROOT/scripts/version.sh")"
echo "=== Packaging Neverball ${VERSION} (macOS ${ARCH}) ==="

# -----------------------------------------------------------------------------
# Build Binaries and Assets
# -----------------------------------------------------------------------------

export MACOSX_DEPLOYMENT_TARGET="11.0"
NCPU="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"

if [[ "$SKIP_BUILD" -eq 0 ]]; then
    echo "==> Compiling mapc, sols, and locales..."
    make -j"$NCPU" sols locales

    echo "==> Compiling neverball, neverputt, and mapc for bundle (${ARCH})..."
    # Ensure object files with bundle-relative paths are cleanly recompiled
    make clean-src
    make -j"$NCPU" \
        BUILD="$BUILD_TYPE" \
        PLATFORM=darwin \
        DATADIR="../Resources/data" \
        LOCALEDIR="../Resources/locale" \
        CFLAGS="-arch $ARCH -O2" \
        CXXFLAGS="-arch $ARCH -O2" \
        LDFLAGS="-arch $ARCH" \
        neverball neverputt mapc
fi

# Verify binaries exist
for bin in neverball neverputt mapc; do
    if [[ ! -f "$REPO_ROOT/$bin" ]]; then
        echo "Error: Expected compiled binary '$bin' was not found." >&2
        exit 1
    fi
done

# -----------------------------------------------------------------------------
# Assemble Application Bundle Structure
# -----------------------------------------------------------------------------

APP_NAME="Neverball"
APP_DIR="$OUT_DIR/${APP_NAME}.app"
CONTENTS_DIR="$APP_DIR/Contents"
MACOS_DIR="$CONTENTS_DIR/MacOS"
RESOURCES_DIR="$CONTENTS_DIR/Resources"
FRAMEWORKS_DIR="$CONTENTS_DIR/Frameworks"
DATA_DIR="$RESOURCES_DIR/data"
LOCALE_DIR="$RESOURCES_DIR/locale"

echo "==> Assembling bundle structure at ${APP_DIR}..."
rm -rf "$APP_DIR"
mkdir -p "$MACOS_DIR" "$RESOURCES_DIR" "$FRAMEWORKS_DIR" "$DATA_DIR" "$LOCALE_DIR"

# 1. Copy executables
echo "==> Copying binaries (neverball, neverputt, mapc)..."
cp "$REPO_ROOT/neverball" "$MACOS_DIR/neverball"
cp "$REPO_ROOT/neverputt" "$MACOS_DIR/neverputt"
cp "$REPO_ROOT/mapc" "$MACOS_DIR/mapc"
chmod 755 "$MACOS_DIR"/*

# 2. Package game data into base-neverball.zip
ZIP_TARGET="$DATA_DIR/base-neverball.zip"
echo "==> Packaging data into ${ZIP_TARGET}..."
(
    cd "$REPO_ROOT/data"
    zip -r -q "$ZIP_TARGET" . \
        -x "*.autosave.map" "*.zip" "*.pk3" ".DS_Store" "*/.DS_Store"
)

# 3. Copy locale catalogs
if [[ -d "$REPO_ROOT/locale" ]]; then
    echo "==> Copying localization catalogs..."
    cp -R "$REPO_ROOT/locale"/* "$LOCALE_DIR/" 2>/dev/null || true
fi

# 4. Generate App Icon
ICNS_CACHE="$OUT_DIR/neverball.icns"
ICON_SRC="$REPO_ROOT/dist/modern-icons/png/neverball-modern-512.png"
if [[ ! -f "$ICON_SRC" ]]; then
    ICON_SRC="$REPO_ROOT/dist/neverball_512.png"
fi

if [[ (! -f "$ICNS_CACHE" || "$ICON_SRC" -nt "$ICNS_CACHE") && -f "$ICON_SRC" ]]; then
    echo "==> Generating application icon from ${ICON_SRC}..."
    ICONSET_TMP="$(mktemp -d -t neverball_iconset)"
    mkdir -p "$ICONSET_TMP/neverball.iconset"

    sips -z 16 16     -s dpiWidth 72 -s dpiHeight 72 "$ICON_SRC" --out "$ICONSET_TMP/neverball.iconset/icon_16x16.png" >/dev/null
    sips -z 32 32     -s dpiWidth 72 -s dpiHeight 72 "$ICON_SRC" --out "$ICONSET_TMP/neverball.iconset/icon_16x16@2x.png" >/dev/null
    sips -z 32 32     -s dpiWidth 72 -s dpiHeight 72 "$ICON_SRC" --out "$ICONSET_TMP/neverball.iconset/icon_32x32.png" >/dev/null
    sips -z 64 64     -s dpiWidth 72 -s dpiHeight 72 "$ICON_SRC" --out "$ICONSET_TMP/neverball.iconset/icon_32x32@2x.png" >/dev/null
    sips -z 128 128   -s dpiWidth 72 -s dpiHeight 72 "$ICON_SRC" --out "$ICONSET_TMP/neverball.iconset/icon_128x128.png" >/dev/null
    sips -z 256 256   -s dpiWidth 72 -s dpiHeight 72 "$ICON_SRC" --out "$ICONSET_TMP/neverball.iconset/icon_128x128@2x.png" >/dev/null
    sips -z 256 256   -s dpiWidth 72 -s dpiHeight 72 "$ICON_SRC" --out "$ICONSET_TMP/neverball.iconset/icon_256x256.png" >/dev/null
    sips -z 512 512   -s dpiWidth 72 -s dpiHeight 72 "$ICON_SRC" --out "$ICONSET_TMP/neverball.iconset/icon_256x256@2x.png" >/dev/null
    sips -z 512 512   -s dpiWidth 72 -s dpiHeight 72 "$ICON_SRC" --out "$ICONSET_TMP/neverball.iconset/icon_512x512.png" >/dev/null
    sips -z 1024 1024 -s dpiWidth 72 -s dpiHeight 72 "$ICON_SRC" --out "$ICONSET_TMP/neverball.iconset/icon_512x512@2x.png" >/dev/null

    iconutil -c icns "$ICONSET_TMP/neverball.iconset" -o "$ICNS_CACHE"
    rm -rf "$ICONSET_TMP"
fi

if [[ -f "$ICNS_CACHE" ]]; then
    cp "$ICNS_CACHE" "$RESOURCES_DIR/neverball.icns"
fi

# 5. Write Info.plist and PkgInfo
echo "==> Writing Info.plist and PkgInfo..."
cat <<EOF > "$CONTENTS_DIR/Info.plist"
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>en</string>
	<key>CFBundleDisplayName</key>
	<string>Neverball</string>
	<key>CFBundleExecutable</key>
	<string>neverball</string>
	<key>CFBundleIconFile</key>
	<string>neverball.icns</string>
	<key>CFBundleIconName</key>
	<string>neverball</string>
	<key>CFBundleIdentifier</key>
	<string>org.neverball.neverball</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>Neverball</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleShortVersionString</key>
	<string>${VERSION}</string>
	<key>CFBundleVersion</key>
	<string>${VERSION}</string>
	<key>LSMinimumSystemVersion</key>
	<string>11.0</string>
	<key>NSHighResolutionCapable</key>
	<true/>
	<key>NSSupportsAutomaticGraphicsSwitching</key>
	<true/>
</dict>
</plist>
EOF

printf "APPLball" > "$CONTENTS_DIR/PkgInfo"

# -----------------------------------------------------------------------------
# Bundle & Relink Dynamic Libraries
# -----------------------------------------------------------------------------

echo "==> Resolving, copying, and relinking dynamic libraries into Frameworks..."
python3 - <<PYEOF
import os, sys, shutil, subprocess, re

macos_dir = "$MACOS_DIR"
fw_dir = "$FRAMEWORKS_DIR"

def get_deps(path):
    res = subprocess.check_output(["otool", "-L", path]).decode()
    deps = []
    for line in res.splitlines()[1:]:
        m = re.match(r"\s+([^\s]+)", line)
        if m:
            dep = m.group(1)
            # Skip system libraries and frameworks
            if not (dep.startswith("/usr/lib/") or dep.startswith("/System/Library/")):
                deps.append(dep)
    return deps

binaries = [
    os.path.join(macos_dir, "neverball"),
    os.path.join(macos_dir, "neverputt"),
    os.path.join(macos_dir, "mapc")
]

copied_dylibs = {}
queue = []

for b in binaries:
    if os.path.exists(b):
        queue.append(b)

while queue:
    curr = queue.pop(0)
    deps = get_deps(curr)
    for dep in deps:
        if dep.startswith("@rpath/"):
            continue
        base = os.path.basename(dep)
        dest_dylib = os.path.join(fw_dir, base)
        if base not in copied_dylibs:
            real_dep = os.path.realpath(dep)
            shutil.copy2(real_dep, dest_dylib)
            os.chmod(dest_dylib, 0o755)
            copied_dylibs[base] = dest_dylib
            queue.append(dest_dylib)
        # Update load path in curr
        subprocess.check_call(["install_name_tool", "-change", dep, f"@rpath/{base}", curr])

# Update library IDs for copied dylibs
for base, dylib in copied_dylibs.items():
    subprocess.check_call(["install_name_tool", "-id", f"@rpath/{base}", dylib])

# Add @executable_path/../Frameworks to rpath of each binary
for b in binaries:
    if os.path.exists(b):
        # Check existing rpaths to avoid duplicate rpath warnings
        load_cmds = subprocess.check_output(["otool", "-l", b]).decode()
        if "@executable_path/../Frameworks" not in load_cmds:
            subprocess.check_call(["install_name_tool", "-add_rpath", "@executable_path/../Frameworks", b])

print(f"    Relinked {len(copied_dylibs)} dynamic libraries.")
PYEOF

# -----------------------------------------------------------------------------
# Inside-Out Code Signing
# -----------------------------------------------------------------------------

echo "==> Ad-hoc code signing bundle (identity: '${SIGN_IDENTITY}')..."

# 1. Sign all dylibs in Frameworks
if compgen -G "$FRAMEWORKS_DIR/*.dylib" >/dev/null; then
    for dylib in "$FRAMEWORKS_DIR"/*.dylib; do
        codesign --force --sign "${SIGN_IDENTITY}" "$dylib"
    done
fi

# 2. Sign all executables in MacOS
SIGN_OPTS=()
if [[ "$SIGN_IDENTITY" != "-" ]]; then
    SIGN_OPTS+=(--options runtime)
fi

for bin in "$MACOS_DIR"/*; do
    if [[ -f "$bin" && -x "$bin" ]]; then
        codesign --force --sign "${SIGN_IDENTITY}" "${SIGN_OPTS[@]}" "$bin"
    fi
done

# 3. Sign the application bundle
codesign --force --sign "${SIGN_IDENTITY}" "${SIGN_OPTS[@]}" "$APP_DIR"

# 4. Strictly verify bundle signature
echo "==> Verifying code signature..."
codesign --verify --deep --strict --verbose=2 "$APP_DIR"
echo "    Code signature verified successfully."

# Invalidate LaunchServices icon cache for local interactive builds
if [[ -z "${CI:-}" ]]; then
    touch "$APP_DIR"
    LSREGISTER="/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister"
    if [[ -x "$LSREGISTER" ]]; then
        "$LSREGISTER" -f "$APP_DIR" >/dev/null 2>&1 || true
    fi
fi

# -----------------------------------------------------------------------------
# Create Archives (ZIP / DMG)
# -----------------------------------------------------------------------------

if [[ "$CREATE_ZIP" -eq 1 ]]; then
    ZIP_NAME="Neverball-${VERSION}-${ARCH}.zip"
    ZIP_PATH="$OUT_DIR/$ZIP_NAME"

    echo "==> Creating compressed ZIP archive at ${ZIP_PATH}..."
    rm -f "$ZIP_PATH"
    (cd "$OUT_DIR" && ditto -c -k --keepParent "${APP_NAME}.app" "$ZIP_NAME")
    echo "    ZIP archive created successfully."
fi

if [[ "$CREATE_DMG" -eq 1 ]]; then
    DMG_NAME="Neverball-${VERSION}-${ARCH}.dmg"
    DMG_PATH="$OUT_DIR/$DMG_NAME"

    echo "==> Creating compressed DMG image at ${DMG_PATH}..."
    DMG_STAGE="$(mktemp -d -t neverball_dmg_stage)"

    cp -R "$APP_DIR" "$DMG_STAGE/"
    ln -s /Applications "$DMG_STAGE/Applications"

    rm -f "$DMG_PATH"
    hdiutil create -volname "Neverball" -srcfolder "$DMG_STAGE" -ov -format UDZO "$DMG_PATH" >/dev/null
    rm -rf "$DMG_STAGE"

    # Sign the DMG file
    codesign --force --sign "${SIGN_IDENTITY}" "$DMG_PATH"
    echo "    DMG created and signed successfully."
fi

echo "=== Packaging complete! ==="
echo "Application: ${APP_DIR}"
if [[ "$CREATE_ZIP" -eq 1 ]]; then
    echo "ZIP Archive: ${OUT_DIR}/${ZIP_NAME}"
fi
if [[ "$CREATE_DMG" -eq 1 ]]; then
    echo "Disk Image:  ${OUT_DIR}/${DMG_NAME}"
fi
