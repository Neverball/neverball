OBJS= main.o ball.o
TARG= ball
LIBS= -lsol -lglw -lvec -ljpeg -lGLU -lGL -lX11 -lm

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
