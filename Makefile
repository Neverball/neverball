
all :
	(cd src; $(MAKE))

install :
	(cd src; $(MAKE) -i install)

clean :
	(cd src; $(MAKE) -i clean)
	(rm -f bin/*)
	(rm -f lib/*)
	(rm -f include/*)
	(rm -f map/*.sol)
	(rm -f map/*.bak)

