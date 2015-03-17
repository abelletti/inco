CFLAGS = -O
OBJS = lzjb.o comp.o decomp.o inco.o

all : inco

clean :
	rm -f inco comp decomp *.o

inco : $(OBJS) inco.h
	$(CC) -o inco $(LDFLAGS) $(OBJS)
	ln -s inco comp
	ln -s inco decomp
