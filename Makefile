CC=gcc
IDIR=/usr/include/igraph
LDIR=/usr/lib/x86_64-linux-gnu/
LIBS=-ligraph -lm
CFLAGS=-Ofast -Wall

.PHONY: all clean time-evolution

all: graphgen.out agos-common.out agos-common-avr.out time-evolution

graphgen.out: graphgen.c
	$(CC) $(CFLAGS) -o $@ $< -I$(IDIR) -L$(LDIR) $(LIBS)

agos-common.out: agos-common.c
	$(CC) $(CFLAGS) -o $@ $< -I$(IDIR) -L$(LDIR) $(LIBS) -DMEAN_FIELD

agos-common-avr.out: agos-common.c
	$(CC) $(CFLAGS) -o $@ $< -I$(IDIR) -L$(LDIR) $(LIBS) -DMEAN_FIELD -DUSE_AVERAGE_PAYOFF

time-evolution: time-evolution-acc.out time-evolution-avr.out

time-evolution-acc.out: agos-common.c
	$(CC) $(CFLAGS) -o $@ $< -I$(IDIR) -L$(LDIR) $(LIBS) -DTIME_EVOLUTION

time-evolution-avr.out: agos-common.c
	$(CC) $(CFLAGS) -o $@ $< -I$(IDIR) -L$(LDIR) $(LIBS) -DTIME_EVOLUTION -DUSE_AVERAGE_PAYOFF

clean:
	rm -f *.o *.out
