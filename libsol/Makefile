OBJS= sol.o cube.o
TARG= libsol.so

INCDIR= -I$(HOME)/include

.c.o :
	$(CC) $(CFLAGS) -c $< $(INCDIR)

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -shared -o $(TARG) $(OBJS)

clean :
	rm $(TARG) $(OBJS)

install : $(TARG)
	cp $(TARG) $(HOME)/lib
	cp sol.h   $(HOME)/include
