OBJS= etc.o
TARG= libetc.a

.c.o :
	$(CC) $(CFLAGS) -c $< $(PNGINCDIR)

all : $(TARG)
	cp $(TARG) ../../lib
	cp etc.h   ../../include

$(TARG) : $(OBJS)
	$(AR) rcs $(TARG) $(OBJS)

clean :
	rm $(TARG) $(OBJS)

