OBJS= main.o
TARG= map2sol
LIBS= -laux -lvec -lsol -lpng -ljpeg -lGL

INCDIR= -I../../include -I/usr/X11R6/include
LIBDIR= -L../../lib     -L/usr/X11R6/lib

.c.o :
	$(CC) $(CFLAGS) -c $< $(INCDIR)

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	rm $(TARG) $(OBJS)

install : $(TARG)
	cp $(TARG) ../../bin
