OBJS= aux.o
TARG= libaux.a

.c.o :
	$(CC) $(CFLAGS) -c $<

$(TARG) : $(OBJS)
	$(AR) rcs $(TARG) $(OBJS)

clean :
	rm $(TARG) $(OBJS)

install : $(TARG)
	cp $(TARG) ../../lib
	cp aux.h   ../../include
