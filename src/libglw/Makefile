OBJS= glw.o
TARG= libglw.a

.c.o :
	$(CC) $(CFLAGS) -c $<

all : $(TARG)
	cp $(TARG) ../../lib
	cp glw.h   ../../include

$(TARG) : $(OBJS)
	$(AR) rcs $(TARG) $(OBJS)

clean :
	rm $(TARG) $(OBJS)

