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

agos-homogeneous-random.out: agos-homogeneous-random.c
	$(CC) $(CFLAGS) -o $@ $< -I$(IDIR) -L$(LDIR) $(LIBS)

agos-time-homogeneous-random.out: agos-time-homogeneous-random.c
	$(CC) $(CFLAGS) -o $@ $< -I$(IDIR) -L$(LDIR) $(LIBS)

clean:
	rm *.o *.out
