OBJS= aio.o
TARG= libaio.a

.c.o :
	$(CC) $(CFLAGS) -c $< $(INCDIR)

$(TARG) : $(OBJS)
	$(AR) rcs $(TARG) $(OBJS)

clean :
	rm $(TARG) $(OBJS)

install : $(TARG)
	cp $(TARG) ../../lib
	cp aio.h   ../../include
