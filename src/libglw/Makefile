OBJS= glw.o
TARG= libglw.so

.c.o :
	$(CC) $(CFLAGS) -c $< 

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -shared -o $(TARG) $(OBJS)

clean :
	rm $(TARG) $(OBJS)

install : $(TARG)
	cp $(TARG) ../../lib
	cp glw.h   ../../include
