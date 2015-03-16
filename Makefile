CFLAGS = -O
OBJS = lzjb.o comp.o decomp.o inco.o

all : inco

clean :
	rm -f inco *.o

inco : $(OBJS) inco.h
	$(CC) -o inco $(LDFLAGS) $(OBJS)
