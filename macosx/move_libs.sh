#!/bin/sh
# This shell script moves all files needed to the right directories EXCEPT SDLMain.h and SDLMain.m
# Get these from the /SDLMain/NIBless folder of the SDL devel extras found at http://www.libsdl.org/download-1.2.php.
# It will work only with MacPorts layout.
cp /opt/local/lib/libpng12.a /opt/local/lib/libintl.a /opt/local/lib/libiconv.a external_libs/.
cp /opt/local/lib/libvorbis.a /opt/local/lib/libvorbisenc.a /opt/local/lib/libvorbisfile.a external_libs/.
cp /opt/local/lib/libogg.a
cp /opt/local/include/libintl.h /opt/local/include/iconv.h external_libs/.
cp /opt/local/include/libpng12/png.h /opt/local/include/libpng12/pngconf.h external_libs/.
cp /opt/local/include/vorbis/codec.h /opt/local/include/vorbis/vorbisenc.h /opt/local/include/vorbis/vorbisfile.h external_libs/.
cp /opt/local/include/ogg/config_types.h /opt/local/include/ogg/ogg.h /opt/local/include/ogg/os_types.h external_libs/.
cp /opt/local/bin/msgfmt external_tools/.