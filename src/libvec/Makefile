OBJS= vec.o
TARG= libvec.a

.c.o :
	$(CC) $(CFLAGS) -c $<

$(TARG) : $(OBJS)
	$(AR) rcs $(TARG) $(OBJS)

clean :
	rm $(TARG) $(OBJS)

install : $(TARG)
	cp $(TARG) ../../lib
	cp vec.h   ../../include
