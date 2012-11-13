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

#define RADIANS_PER_PIXEL M_PI/(2 * 90.0)
#define N 200
#define M 1
#define a 100
#define b 40
#define R 20
#define p 3
#define q 8

GLfloat fovy = 40.0;
GLdouble eyeRho = 600;
GLdouble eyeTheta = 0;
GLdouble eyePhi = M_PI/2;

void sphericalToCartesian(double r, double theta, double phi,
													double *x, double *y, double *z){
	double sin_phi = sin(phi);
	*x = r * cos(theta) * sin_phi;
	*y = r * sin(theta) * sin_phi;
	*z = r * cos(phi);
}

void setView(){
	GLdouble eyex, eyey, eyez;
	sphericalToCartesian(eyeRho, eyeTheta, eyePhi,
												&eyex, &eyey, &eyez);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyex, eyey, eyez, 0, 0, 0, 0, 0, 1);
}

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
GLfloat ModelView[4*4];
GLfloat Projection[4*4];
GLfloat Color[4];

//
// Vertex Posistion Attribute
//
GLint vertexPositionAttr;

//
// GL uniform handles
//
GLint ModelViewProjectionUniform;
GLint ColorUniform;

void loadUniforms() {
  GLfloat ModelViewProjection[4*4];
	glGetFloatv(GL_MODELVIEW_MATRIX, ModelView);
	glGetFloatv(GL_PROJECTION_MATRIX, Projection);
  matrixMultiply(ModelViewProjection, Projection, ModelView);
  glUniformMatrix4fv(ModelViewProjectionUniform, 1, GL_FALSE,
                     ModelViewProjection);
  glUniform3fv(ColorUniform, 1, Color);
}

void spline(void){
	static GLboolean first = GL_TRUE;
	static GLuint vertBuffer;
	if(first){
		static GLfloat verts[N][3];

		for(int i = 0; i < N; i++){
			GLfloat t = 2 * M_PI * i / N;
			verts[i][0] = (a + b * cos(q * t)) * cos(p * t);
			verts[i][1] = (a + b * cos(q * t)) * sin(p * t);
			verts[i][2] = b * sin(q * t);
		}
		glGenBuffers(1, &vertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat) * N, verts, GL_STATIC_DRAW);
		first = GL_FALSE;
	}
	loadUniforms();
	glEnableVertexAttribArray(vertexPositionAttr);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glVertexAttribPointer(vertexPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glDrawArrays(GL_LINE_LOOP, 0, N);
}

void reshape(int w, int h) {
  glViewport(0,0, w,h);
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);
	spline();
  glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
#define ESC 27
  if (key == ESC) exit(0);
#undef ESC
}

//
// GLSL source code for our vertex shader.
//
const GLchar *vertexShaderSource = {
  "attribute vec3 vertexPosition;"
  "uniform mat4 ModelViewProjection;\n" 
  "void main() {\n"
  "  gl_Position = ModelViewProjection*vec4(vertexPosition, 1.0);\n"
  "}\n"
};

//
// GLSL source code for our fragment shader.
//
const GLchar *fragmentShaderSource = {
  "uniform vec3 Color;\n"
  "void main() {\n"
  "  gl_FragColor = vec4(Color, 1.0);\n"
  "}\n"
};

GLuint vertexShader;
GLuint fragmentShader;
GLuint program;

//
// Install our shader programs and tell GL to use them.
// We also initialize the uniform variables.
//
void installShaders(void) {
  //
  // (1) Create shader objects
  //
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

  //
  // (2) Load source code into shader objects.
  //
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

  //
  // (3) Compile shaders.
  //
  glCompileShader(vertexShader);
  GLint success;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar infoLog[200];
    GLint charsWritten;
    glGetShaderInfoLog(vertexShader, sizeof(infoLog), &charsWritten, infoLog);
    fprintf(stderr, "vertex shader info log:\n%s\n\n", infoLog);
  }
  checkOpenGLError(__LINE__);

  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar infoLog[200];
    GLint charsWritten;
    glGetShaderInfoLog(fragmentShader, sizeof(infoLog), &charsWritten, infoLog);
    fprintf(stderr, "fragment shader info log:\n%s\n\n", infoLog);
  }
  checkOpenGLError(__LINE__);

  //
  // (4) Create program object and attach vertex and fragment shader.
  //
  program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  checkOpenGLError(__LINE__);

  //
  // (5) Link program.
  //
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    GLchar infoLog[200];
    GLint charsWritten;
    glGetProgramInfoLog(program, sizeof(infoLog), &charsWritten, infoLog);
    fprintf(stderr, "program info log:\n%s\n\n", infoLog);
  }
  checkOpenGLError(__LINE__);

  //
  // (7)
  //
  vertexPositionAttr = glGetAttribLocation(program, "vertexPosition");
  if (vertexPositionAttr == -1) {
    fprintf(stderr, "Error fetching vertex position attribute!\n");
    exit(-1);
  }

  //
  // (8) Fetch handles for uniform variables in program.
  // None, using built-ins.
  //
  ModelViewProjectionUniform = glGetUniformLocation(program, "ModelViewProjection");
  ColorUniform = glGetUniformLocation(program,"Color");

  //
  // (9) Tell GL to use our program
  //
  glUseProgram(program);
}

int mousey, mousex;
double epsilon = 0.0000001;

void mouse(int button, int state, int x, int y){
	mousex = x;
	mousey = y;
}

void mouseMotion(int x, int y){
		int dx = x - mousex, dy = y - mousey;
		double eyex, eyey, eyez;
		eyeTheta -= dx * RADIANS_PER_PIXEL;
		eyePhi -= dy * RADIANS_PER_PIXEL;
		mousex = x, mousey = y;
		if(eyePhi >= M_PI)
			eyePhi = M_PI - epsilon;
		else if(eyePhi <= 0.0)
			eyePhi = epsilon;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		sphericalToCartesian(eyeRho, eyeTheta, eyePhi,
													&eyex, &eyey, &eyez);
		gluLookAt(eyex, eyey, eyez, 0,0,0, 0,0,1);
		glutPostRedisplay();
}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
  glutInitWindowSize(500,500);
  glutInitWindowPosition(10,10);
  glutCreateWindow("Spline");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
  installShaders();
/*
	matrixIdentity(ModelView);
	matrixLookat(ModelView,
								.1, 0, 600,
								0, 0, 0,
								0, 0, 1);
	matrixIdentity(Projection);
	matrixPerspective(Projection,
										fovy, 1, 0.01, 1000);
*/
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, 1.0, 0.1, 1000);
	setView();
  Color[0] = 1.0; Color[1] = 1.0; Color[2] = 0.0;

  glutMainLoop();
  return 0;
}
