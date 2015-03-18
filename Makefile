CFLAGS = -O -Wall
OBJS = lzjb.o comp.o decomp.o inco.o

UNAME := $(shell uname)
ifeq ($(UNAME),Linux)
	CFLAGS += -DLINUX
endif
ifeq ($(UNAME),SunOS)
	CFLAGS += -DSOLARIS
endif
ifeq ($(UNAME),Darwin)
	CFLAGS += -DOSX
endif

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
