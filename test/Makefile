CC=gcc
COPTS=-g -std=c99 -Wall

all: dancingrobot

JUNK=*.o *~ core *.dSYM

clean:
	-rm -rf $(JUNK)

clobber:
	-rm -rf $(JUNK) dancingrobot

ifeq "$(shell uname)" "Darwin"
LIBS=-framework GLUT -framework OpenGL -framework Cocoa
else
LIBS=-L/usr/X11R6/lib -lglut -lGLU -lGL -lXext -lX11 -lm
endif

dancingrobot: dancingrobot.o matrix.o
	$(CC) $(COPTS) $^ -o $@ $(LIBS)

.c.o:
	$(CC) -c $(COPTS) $<

dancingrobot.o: dancingrobot.c matrix.h
matrix.o: matrix.c matrix.h
