CC=gcc
COPTS=-g -std=c99 -Wall

all: toroid

JUNK=*.o *~ core *.dSYM

clean:
	-rm -rf $(JUNK)

clobber:
	-rm -rf $(JUNK) toroid

ifeq "$(shell uname)" "Darwin"
LIBS=-framework GLUT -framework OpenGL -framework Cocoa
else
LIBS=-L/usr/X11R6/lib -lglut -lGLU -lGL -lXext -lX11 -lm
endif

toroid: toroid.o matrix.o
	$(CC) $(COPTS) $^ -o $@ $(LIBS)

.c.o:
	$(CC) -c $(COPTS) $<

toroid.o: toroid.c matrix.h
matrix.o: matrix.c matrix.h
