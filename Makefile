
all :
	(cd src;  $(MAKE))
	(cd data; $(MAKE))

clean :
	(cd src;  $(MAKE) -i clean)
	(cd data; $(MAKE) -i clean)
	rm -f neverball mapc

