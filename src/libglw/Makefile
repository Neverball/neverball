OBJS= glw.o
TARG= libglw.a

INCDIR= -I/usr/X11R6/include

.c.o :
	$(CC) $(CFLAGS) -c $< $(INCDIR)

$(TARG) : $(OBJS)
	$(AR) rcs $(TARG) $(OBJS)

clean :
	rm $(TARG) $(OBJS)

install : $(TARG)
	cp $(TARG) ../../lib
	cp glw.h   ../../include
