CFLAGS = -O
OBJS = lzjb.o comp.o decomp.o inco.o

all : inco

clean :
	rm -f inco comp decomp *.o

inco : $(OBJS)
	$(CC) -o inco $(LDFLAGS) $(OBJS)
	ln -fs inco comp
	ln -fs inco decomp

comp.o : comp.c defs.h comp.h

decomp.o : decomp.c defs.h decomp.h

inco.o : inco.c defs.h inco.h
