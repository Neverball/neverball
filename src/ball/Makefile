OBJS= main.o coin.o ball.o game.o play.o title.o time.o fall.o goal.o pause.o
TARG= ball
LIBS= -laux -lsol -lglw -lvec -laio $(ALSALIB) -lpng -ljpeg -lGLU -lGL -lX11 -lm

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
