OBJS= main.o
TARG= map2sol
LIBS= -lvec -lsol -lGL

INCDIR= -I../../include
LIBDIR= -L../../lib

.c.o :
	$(CC) $(CFLAGS) -c $< $(INCDIR)

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	rm $(TARG) $(OBJS)

install : $(TARG)
	cp $(TARG) ../../bin
