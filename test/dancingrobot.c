#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "matrix.h"
#if defined(__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif
#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

#define I(row,col) ((col)*4 + (row))

void checkOpenGLError(int line) {
  bool wasError = false;
  GLenum error = glGetError();
  while (error != GL_NO_ERROR) {
    printf("GL ERROR: at line %d: %s\n", line, gluErrorString(error));
    wasError = true;
    error = glGetError();
  }
  if (wasError) exit(-1);
}

//
// Client matrices and color.
//

GLfloat ModelViewMatrix[4*4];
GLfloat ModelView[4*4];
GLfloat Projection[4*4];
GLfloat Color[4];

int main(int argc, char *argv[]) {
	matrixIdentity(ModelViewMatrix);
	//matrixOrtho(ModelViewMatrix, 0, 50, 0, 50, -1, 1);
	matrixLookat(ModelViewMatrix, 1, 1, 0, 1, 0, 0, 0, 0, 1);
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			printf("%f", ModelViewMatrix[I(i, j)]);
		}
		puts("");
	}
  return 0;
}
