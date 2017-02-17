CC=gcc
IDIR=/usr/include/igraph
LDIR=/usr/lib/x86_64-linux-gnu/
LIBS=-ligraph -lm
CFLAGS=-Ofast -Wall

.PHONY: all clean

graphgen.out: graphgen.c
	$(CC) $(CFLAGS) -o $@ $< -I$(IDIR) -L$(LDIR) $(LIBS)

agos-common.out: agos-common.c
	$(CC) $(CFLAGS) -o $@ $< -I$(IDIR) -L$(LDIR) $(LIBS)

clean:
	rm -f *.o *.out
