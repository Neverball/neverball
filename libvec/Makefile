OBJS= vec.o
TARG= libvec.so

.c.o :
	$(CC) $(CFLAGS) -c $<

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -shared -o $(TARG) $(OBJS)

clean :
	rm $(TARG) $(OBJS)

install : $(TARG)
	cp $(TARG) $(HOME)/lib
	cp vec.h   $(HOME)/include
