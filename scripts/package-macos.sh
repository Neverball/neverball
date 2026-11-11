#!/usr/bin/env bash
#
# scripts/package-macos.sh - Build and package Neverball for macOS
#
# Creates a standalone, self-contained Neverball.app with statically linked
# dependencies, data packaged in base-neverball.zip, and mapc and neverputt
# bundled inside the application bundle. Packages a distributable DMG disk
# image.

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
CREATE_DMG=1
SKIP_BUILD=0

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Builds and packages Neverball as a self-contained macOS application bundle.

Options:
  --arch <arch>         Target architecture: arm64 or x86_64 (default: host arch)
  --build-type <type>   Build configuration: release (default) or devel
  --identity <id>       Code signing identity (default: "-" for ad-hoc signing)
  --out-dir <dir>       Output directory for .app and .dmg (default: build/macos)
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

for cmd in clang make cmake codesign zip hdiutil; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo "Error: Required command '$cmd' is not found in PATH." >&2
        exit 1
    fi
done

VERSION="$(sh "$REPO_ROOT/scripts/version.sh")"
echo "=== Packaging Neverball ${VERSION} (macOS ${ARCH}) ==="

# -----------------------------------------------------------------------------
# Build Binaries and Assets
# -----------------------------------------------------------------------------

export MACOSX_DEPLOYMENT_TARGET="11.0"
NCPU="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"
BREW_PREFIX="$(brew --prefix 2>/dev/null || echo /opt/homebrew)"
TTF_STATIC_LIB="$OUT_DIR/libSDL2_ttf_${ARCH}.a"

if [[ "$SKIP_BUILD" -eq 0 ]]; then
    SDL_STATIC_LIB_DEFAULT="$BREW_PREFIX/opt/sdl2/lib/libSDL2.a"
    if [[ -z "${SDL_STATIC_LIB:-}" ]]; then
        if [[ -f "$SDL_STATIC_LIB_DEFAULT" ]]; then
            SDL_STATIC_LIB="$SDL_STATIC_LIB_DEFAULT"
        else
            SDL_STATIC_LIB="$OUT_DIR/libSDL2_${ARCH}.a"
        fi
    fi

    if [[ ! -f "$SDL_STATIC_LIB" ]]; then
        echo "==> Compiling SDL2 static library for ${ARCH}..."
        SDL2_VER="2.32.8"
        SDL2_SRC_DIR="$OUT_DIR/SDL2-${SDL2_VER}"
        if [[ ! -d "$SDL2_SRC_DIR" ]]; then
            curl -sSfL "https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VER}/SDL2-${SDL2_VER}.tar.gz" | tar -xz -C "$OUT_DIR"
        fi
        cmake -S "$SDL2_SRC_DIR" -B "$SDL2_SRC_DIR/build-${ARCH}" \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_OSX_ARCHITECTURES="$ARCH" \
            -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACOSX_DEPLOYMENT_TARGET" \
            -DSDL_SHARED=OFF \
            -DSDL_STATIC=ON \
            -DSDL_TEST=OFF
        cmake --build "$SDL2_SRC_DIR/build-${ARCH}" -j"$NCPU"
        cp "$SDL2_SRC_DIR/build-${ARCH}/libSDL2.a" "$SDL_STATIC_LIB"
    fi

    echo "==> Compiling minimal SDL2_ttf for ${ARCH}..."
    SDL_TTF_SRC="$OUT_DIR/SDL_ttf.c"
    if [[ ! -f "$SDL_TTF_SRC" ]]; then
        curl -sSfL "https://raw.githubusercontent.com/libsdl-org/SDL_ttf/release-2.24.0/SDL_ttf.c" -o "$SDL_TTF_SRC"
    fi
    clang -c "$SDL_TTF_SRC" -o "$OUT_DIR/SDL_ttf_${ARCH}.o" \
        -arch "$ARCH" -O2 -DTTF_USE_HARFBUZZ=0 \
        -I"$BREW_PREFIX/include/SDL2" \
        -I"$BREW_PREFIX/include/freetype2"
    ar rcs "$TTF_STATIC_LIB" "$OUT_DIR/SDL_ttf_${ARCH}.o"
    rm -f "$OUT_DIR/SDL_ttf_${ARCH}.o"

    echo "==> Compiling mapc, sols, and locales..."
    make clean-src
    make mapc
    make -j"$NCPU" sols locales

    echo "==> Compiling neverball, neverputt, and mapc for bundle (${ARCH})..."
    make clean-src
    MAKE_ARGS=(
        -j"$NCPU"
        BUILD="$BUILD_TYPE"
        PLATFORM=darwin
        STATIC=1
        SDL_STATIC_LIB="$SDL_STATIC_LIB"
        TTF_STATIC_LIB="$TTF_STATIC_LIB"
        DATADIR="../Resources/data"
        LOCALEDIR="../Resources/locale"
        CFLAGS="-arch $ARCH -O2"
        CXXFLAGS="-arch $ARCH -O2"
        LDFLAGS="-arch $ARCH"
    )
    make "${MAKE_ARGS[@]}" neverball neverputt mapc
fi

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
DATA_DIR="$RESOURCES_DIR/data"
LOCALE_DIR="$RESOURCES_DIR/locale"

echo "==> Assembling bundle structure at ${APP_DIR}..."
rm -rf "$APP_DIR"
mkdir -p "$MACOS_DIR" "$RESOURCES_DIR" "$DATA_DIR" "$LOCALE_DIR"

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

# 4. Copy Application Icon
echo "==> Installing application icon..."
cp "$REPO_ROOT/dist/macos/neverball.icns" "$RESOURCES_DIR/neverball.icns"

# 5. Write Info.plist and PkgInfo
echo "==> Configuring Info.plist and PkgInfo..."
sed "s/@VERSION@/${VERSION}/g" "$REPO_ROOT/dist/macos/Info.plist.in" > "$CONTENTS_DIR/Info.plist"
printf "APPLball" > "$CONTENTS_DIR/PkgInfo"

# 6. Copy level editor gamepacks into Resources
for tool_dir in TrenchBroom Radiant; do
    if [[ -d "$REPO_ROOT/$tool_dir" ]]; then
        echo "==> Copying $tool_dir gamepack into Resources..."
        cp -R "$REPO_ROOT/$tool_dir" "$RESOURCES_DIR/"
        find "$RESOURCES_DIR/$tool_dir" -name ".DS_Store" -delete 2>/dev/null || true
    fi
done

# -----------------------------------------------------------------------------
# Code Signing
# -----------------------------------------------------------------------------

echo "==> Code signing bundle (identity: '${SIGN_IDENTITY}')..."

# 1. Sign auxiliary executables in MacOS (must be signed before the main bundle)
CODESIGN_CMD=(codesign --force --sign "${SIGN_IDENTITY}")
if [[ "$SIGN_IDENTITY" != "-" ]]; then
    CODESIGN_CMD+=(--options runtime)
fi

MAIN_BIN="$MACOS_DIR/neverball"
for bin in "$MACOS_DIR"/*; do
    if [[ -f "$bin" && -x "$bin" && ! -L "$bin" && "$bin" != "$MAIN_BIN" ]]; then
        "${CODESIGN_CMD[@]}" "$bin"
    fi
done

# 3. Sign the application bundle (signs the main executable and seals the bundle)
"${CODESIGN_CMD[@]}" "$APP_DIR"

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
# Create DMG Disk Image
# -----------------------------------------------------------------------------

if [[ "$CREATE_DMG" -eq 1 ]]; then
    DMG_NAME="Neverball-${VERSION}-${ARCH}.dmg"
    DMG_PATH="$OUT_DIR/$DMG_NAME"

    echo "==> Creating compressed DMG image at ${DMG_PATH}..."
    DMG_STAGE="$(mktemp -d -t neverball_dmg_stage)"

    cp -R "$APP_DIR" "$DMG_STAGE/"
    ln -s /Applications "$DMG_STAGE/Applications"
    if [[ -f "$REPO_ROOT/dist/macos/How to Open.html" ]]; then
        cp "$REPO_ROOT/dist/macos/How to Open.html" "$DMG_STAGE/How to Open.html"
    fi
    for tool_dir in TrenchBroom Radiant; do
        if [[ -d "$REPO_ROOT/$tool_dir" ]]; then
            cp -R "$REPO_ROOT/$tool_dir" "$DMG_STAGE/"
            find "$DMG_STAGE/$tool_dir" -name ".DS_Store" -delete 2>/dev/null || true
        fi
    done

    rm -f "$DMG_PATH"
    hdiutil create -volname "Neverball" -srcfolder "$DMG_STAGE" -ov -format UDZO "$DMG_PATH" >/dev/null
    rm -rf "$DMG_STAGE"

    # Only sign the DMG container if an explicit identity other than '-' is provided
    if [[ "$SIGN_IDENTITY" != "-" ]]; then
        codesign --force --sign "${SIGN_IDENTITY}" "$DMG_PATH"
        echo "    DMG created and signed successfully."
    else
        echo "    DMG created successfully (unsigned container)."
    fi
fi

echo "=== Packaging complete! ==="
echo "Application: ${APP_DIR}"
if [[ "$CREATE_DMG" -eq 1 ]]; then
    echo "Disk Image:  ${OUT_DIR}/${DMG_NAME}"
fi
