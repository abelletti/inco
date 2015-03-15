all : inline

clean :
	rm -f comp decomp *.o

inline: comp decomp

comp : lzjb.o comp.o
	$(CC) -o comp $(LDFLAGS) lzjb.o comp.o

decomp : lzjb.o decomp.o
	$(CC) -o decomp $(LDFLAGS) lzjb.o decomp.o
