CC=gcc
CFLAGS=--std=c99 -g -O3 -Wall --pedantic `sdl-config --cflags`
LDFLAGS=
LIBS=-lm -lpng -lcairo `sdl-config --libs`

PROGRAM=guilloche

all: ${PROGRAM}

${PROGRAM}: ${PROGRAM}.c savepng.c
	$(CC) ${PROGRAM}.c savepng.c $(CFLAGS) -o $@ $(LDFLAGS) $(LIBS)

clean:
	rm -f ${PROGRAM}
