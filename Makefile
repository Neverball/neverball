
linux :
	(cd src;  $(MAKE) X11LIBDIR="-L/usr/X11R6/lib")
	(cd data; $(MAKE))

alsa :
	(cd src;  $(MAKE) X11LIBDIR="-L/usr/X11R6/lib" \
	                  ALSALIB="-lasound"           \
	                  ALSAOPT="-DALSA")
	(cd data; $(MAKE))

irix :
	(cd src;  $(MAKE) PNGLIBDIR="-I/usr/freeware/lib32" \
	                  PNGINCDIR="-L/usr/freeware/include")
	(cd data; $(MAKE))

clean :
	(cd src;  $(MAKE) -i clean)
	(cd data; $(MAKE) -i clean)
	(rm -f bin/*)
	(rm -f lib/*)
	(rm -f include/*)

