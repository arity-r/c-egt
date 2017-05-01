CC=gcc
IDIR=/usr/include/igraph
LDIR=/usr/lib/x86_64-linux-gnu/
LIBS=-ligraph -lm
CFLAGS=-Ofast -Wall

.PHONY: all clean

all: graphgen.out agos-common.out agos-common-avr.out

graphgen.out: graphgen.c
	$(CC) $(CFLAGS) -o $@ $< -I$(IDIR) -L$(LDIR) $(LIBS)

agos-common.out: agos-common.c
	$(CC) $(CFLAGS) -o $@ $< -I$(IDIR) -L$(LDIR) $(LIBS)

agos-common-avr.out: agos-common.c
	$(CC) $(CFLAGS) -o $@ $< -I$(IDIR) -L$(LDIR) $(LIBS) -DUSE_AVERAGE_PAYOFF

clean:
	rm -f *.o *.out
