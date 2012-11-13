CC=gcc
COPTS=-g -std=c99 -Wall

all: toroid_shaded toroid_mesh toroid_spline

JUNK=*.o *~ core *.dSYM

clean:
	-rm -rf $(JUNK)

clobber:
	-rm -rf $(JUNK) toroid_shaded toroid_mesh toroid_spline

ifeq "$(shell uname)" "Darwin"
LIBS=-framework GLUT -framework OpenGL -framework Cocoa
else
LIBS=-L/usr/X11R6/lib -lglut -lGLU -lGL -lXext -lX11 -lm
endif

toroid_shaded: toroid_shaded.o matrix.o
	$(CC) $(COPTS) $^ -o $@ $(LIBS)

toroid_mesh: toroid_mesh.o matrix.o
	$(CC) $(COPTS) $^ -o $@ $(LIBS)

toroid_spline: toroid_spline.o matrix.o
	$(CC) $(COPTS) $^ -o $@ $(LIBS)

.c.o:
	$(CC) -c $(COPTS) $<

toroid_shaded.o: toroid_shaded.c matrix.h
toroid_mesh.o: toroid_mesh.c matrix.h
toroid_spline.o: toroid_spline.c matrix.h
matrix.o: matrix.c matrix.h
