# Neverball

## Requirements

| Name      |                                        |
| --------- | -------------------------------------- |
| SDL 2.0   | https://github.com/libsdl-org/SDL      |
| SDL2_ttf  | https://github.com/libsdl-org/SDL_ttf  |
| libvorbis | https://xiph.org/vorbis                |
| libpng    | https://libpng.org/pub/png/libpng.html |
| libjpeg   | https://ijg.org                        |
| libcurl   | optional, see below                    |
| libintl   | optional, see below                    |

### Ubuntu / Debian

```shell
sudo apt install build-essential gettext libsdl2-dev libsdl2-ttf-dev libcurl4-openssl-dev libjpeg-dev libpng-dev libvorbis-dev
```

### Fedora

```shell
sudo dnf install @development-tools gettext-devel SDL2-devel SDL2_ttf-devel libcurl-devel libjpeg-turbo-devel libpng-devel libvorbis-devel
```

## Compilation

Under Unix and Linux, simply run:

```shell
make
```

### Windows (MSYS2)

Under Windows, install the MSYS2 environment and run:

```shell
make
```

### Cross-compilation for Windows (Fedora example)

Cross-compilation for Windows is supported. On Fedora Linux:

```shell
make sols clean-src
mingw32-make -o sols PLATFORM=mingw
```

## Optional features

Optional features can be enabled at compile time by passing one or more additional arguments to `make`. Most of these features require additional libraries to be installed.

### Native language support

Disable NLS:

```shell
make ENABLE_NLS=0
```

- Enabled by default.
- Requires an additional library on non-GLIBC systems.
- Dependency: libintl (GNU gettext): https://www.gnu.org/software/gettext/

### Package downloads

Use libcurl for package downloads (enabled by default). Set this to any other value (e.g. `ENABLE_FETCH=0`) to disable downloads:

```shell
make ENABLE_FETCH=curl
```

- Dependency: libcurl: https://curl.se/libcurl

### Tilt input devices

Nintendo Wii Remote support on Linux:

```shell
make ENABLE_TILT=wii
```

- Dependencies: BlueZ (https://www.bluez.org), libwiimote (https://libwiimote.sourceforge.net)

Hillcrest Labs Loop support:

```shell
make ENABLE_TILT=loop
```

- Dependencies: libusb-1.0 (http://libusb.org/wiki/Libusb1.0), libfreespace (https://github.com/hcrest/libfreespace)

Leap Motion support on Linux:

```shell
make ENABLE_TILT=leapmotion
```

- Dependency: Leap Motion V2 SDK: https://developer.leapmotion.com

### Head-mounted display (HMD)

OpenHMD support (including the Oculus Rift):

```shell
make ENABLE_HMD=openhmd
```

- Dependency: OpenHMD: http://openhmd.net

Oculus Rift support:

```shell
make ENABLE_HMD=libovr
```

- Dependency: Oculus SDK: https://developer.oculusvr.com

### Radiant console output

Map compiler output to Radiant console:

```shell
make ENABLE_RADIANT_CONSOLE=1
```

- Dependency: SDL2_net: https://github.com/libsdl-org/SDL_net

## Installation

By default, an uninstalled build may be executed in place. A system-wide installation on Linux would probably copy the game to `/opt/neverball`.

To be able to use the NetRadiant level editor, the game data and binaries must be installed in the same location. Distributions that wish to package Neverball, Neverputt, and their shared data separately should take care to use symlinks and launcher scripts to support this.

## Distribution

The `dist` subdirectory contains some miscellaneous files:

- `.desktop` files
- high resolution icons in PNG, SVG and ICO formats

Web: https://neverball.org

