OBJS= sol.o
TARG= libsol.a

INCDIR= -I../../include

.c.o :
	$(CC) $(CFLAGS) -c $< $(INCDIR)

$(TARG) : $(OBJS)
	$(AR) rcs $(TARG) $(OBJS)

clean :
	rm $(TARG) $(OBJS)

install : $(TARG)
	cp $(TARG) ../../lib
	cp sol.h   ../../include
