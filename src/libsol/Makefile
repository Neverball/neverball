OBJS= sol.o
TARG= libsol.a

INCDIR= -I../../include

.c.o :
	$(CC) $(CFLAGS) -c $< $(INCDIR)

all : $(TARG)
	cp $(TARG) ../../lib
	cp sol.h   ../../include

$(TARG) : $(OBJS)
	$(AR) rcs $(TARG) $(OBJS)

clean :
	rm $(TARG) $(OBJS)

