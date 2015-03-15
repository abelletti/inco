all : inline

clean :
	rm -f comp decomp *.o

inline: comp decomp

comp : compress.o comp.o
	$(CC) -o comp $(LDFLAGS) compress.o comp.o

decomp : compress.o decomp.o
	$(CC) -o decomp $(LDFLAGS) compress.o decomp.o
