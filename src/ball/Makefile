OBJS= main.o coin.o ball.o game.o title.o play.o stop.o pause.o
TARG= ball
LIBS= -letc -lsol -lglw -lvec -laio $(ALSALIB) -lpng -ljpeg -lGLU -lGL -lX11 -lm

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

