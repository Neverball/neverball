OBJS= aio.o
TARG= libaio.a

.c.o :
	$(CC) $(CFLAGS) $(ALSAOPT) -c $< $(INCDIR)

all : $(TARG)
	cp $(TARG) ../../lib
	cp aio.h   ../../include

$(TARG) : $(OBJS)
	$(AR) rcs $(TARG) $(OBJS)

clean :
	rm $(TARG) $(OBJS)

