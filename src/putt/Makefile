OBJS= main.o ball.o
TARG= putt
LIBS= -lsol -lglv -lglw -lvec -lGL -lX11 -lm

INCDIR= -I../../include
LIBDIR= -L../../lib -L/usr/X11R6/lib

.c.o :
	$(CC) $(CFLAGS) -c $< $(INCDIR)

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	rm $(TARG) $(OBJS)

install : $(TARG)
	cp $(TARG) ../../bin
