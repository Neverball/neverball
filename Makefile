
all :
	(cd src;  $(MAKE))
	(cd data; $(MAKE))

install :
	(cd src;  $(MAKE) -i install)
	(cd data; $(MAKE) -i install)

clean :
	(cd src;  $(MAKE) -i clean)
	(cd data; $(MAKE) -i clean)
	(rm -f bin/*)
	(rm -f lib/*)
	(rm -f include/*)

