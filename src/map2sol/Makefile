OBJS= main.o
TARG= map2sol
LIBS= -letc -lvec -lsol -lpng -ljpeg -lGL -lm

INCDIR= -I../../include
LIBDIR= -L../../lib $(X11LIBDIR) $(PNGLIBDIR)

.c.o :
	$(CC) $(CFLAGS) -c $< $(INCDIR)

all : $(TARG)
	cp $(TARG) ../../bin

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	rm $(TARG) $(OBJS)

