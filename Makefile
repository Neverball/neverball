
all :
	(cd libsol; $(MAKE))
	(cd libglw; $(MAKE))
	(cd libglv; $(MAKE))
	(cd libvec; $(MAKE))
	(cd putt;   $(MAKE))

install :
	(cd libsol; $(MAKE) install)
	(cd libglw; $(MAKE) install)
	(cd libglv; $(MAKE) install)
	(cd libvec; $(MAKE) install)
	(cd putt;   $(MAKE) install)

clean :
	(cd libsol; $(MAKE) clean)
	(cd libglw; $(MAKE) clean)
	(cd libglv; $(MAKE) clean)
	(cd libvec; $(MAKE) clean)
	(cd putt;   $(MAKE) clean)

