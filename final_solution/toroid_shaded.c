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
#define TUBE_N 512
#define TUBE_M 16
#define a 100
#define b 40
#define R 20
#define p 3
#define q 8
#define INDICES_PER_STRIP (2 * TUBE_M + 2)
#define NUM_STRIPS 512
#define NUM_STRIP_INDICES (NUM_STRIPS * INDICES_PER_STRIP)
#define VERTEX_INDEX(i,j) ((i)*TUBE_M + (j))

GLfloat fovy = 40.0;
GLdouble eyeRho = 600;
GLdouble eyeTheta = 0;
GLdouble eyePhi = M_PI/2;
GLushort tubeStrips[NUM_STRIP_INDICES];

void createMeshStripIndices(void){
	int n = 0;
	int i, j;
	for(i = 0; i < TUBE_N; i++){
		for(j = 0; j < TUBE_M; j++){
			tubeStrips[n++] = VERTEX_INDEX((i+1)%TUBE_N,j);
			tubeStrips[n++] = VERTEX_INDEX(i,j);
		}
		tubeStrips[n++] = VERTEX_INDEX((i+1)%TUBE_N,0);
		tubeStrips[n++] = VERTEX_INDEX(i,0);
	}
}

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

GLfloat ambientLight[3] = {0.4, 0.4, 0.4};
GLfloat light0Color[3] = {0.0, 0.0, 1.0};
GLfloat light0Position[3] = {600.0, 0.0, 0.0};

GLfloat materialAmbient[3];
GLfloat materialDiffuse[3];
GLfloat materialSpecular[3];
GLfloat materialShininess;


//
// Vertex Posistion Attribute
//
GLint vertexPositionAttr;
GLint vertexNormalAttr;

//
// GL uniform handles
//
GLint ModelViewProjectionUniform;
GLint ModelViewMatrixUniform;
GLint NormalMatrixUniform;

GLint ambientLightUniform;
GLint light0ColorUniform;
GLint light0PositionUniform;

GLint materialAmbientUniform;
GLint materialDiffuseUniform;
GLint materialSpecularUniform;
GLint materialShininessUniform;

GLint ColorUniform;

void loadUniforms() {
  GLfloat ModelViewProjection[4*4], NormalMatrix[3*3];
	glGetFloatv(GL_MODELVIEW_MATRIX, ModelView);
	glGetFloatv(GL_PROJECTION_MATRIX, Projection);
  matrixMultiply(ModelViewProjection, Projection, ModelView);
	matrixNormal(ModelView, NormalMatrix);
  glUniformMatrix4fv(ModelViewProjectionUniform, 1, GL_FALSE,
                     ModelViewProjection);
	glUniformMatrix4fv(ModelViewMatrixUniform, 1, GL_FALSE,
											ModelView);
	glUniformMatrix3fv(NormalMatrixUniform, 1, GL_FALSE, NormalMatrix);

	//load lights
	glUniform3fv(ambientLightUniform, 1, ambientLight);
	glUniform3fv(light0ColorUniform, 1, light0Color);
	glUniform3fv(light0PositionUniform, 1, light0Position);

	//load material props
	glUniform3fv(materialAmbientUniform, 1, materialAmbient);
	glUniform3fv(materialDiffuseUniform, 1, materialDiffuse);
	glUniform3fv(materialSpecularUniform, 1, materialSpecular);
	glUniform1f(materialShininessUniform, materialShininess);

  //glUniform3fv(ColorUniform, 1, Color);
}

void cross(GLdouble U[3], GLdouble V[3], GLdouble N[3]){
	N[0] = U[1] * V[2] - U[2] * V[1];
	N[1] = U[2] * V[0] - U[0] * V[2];
	N[2] = U[0] * V[1] - U[1] * V[0];
}

double dot(GLdouble U[3], GLdouble V[3]){
	return U[0] * V[0] + U[1] * V[1] + U[2] * V[2];
}

void scale(GLfloat s, GLdouble U[3]){
	U[0] *= s;
	U[1] *= s;
	U[2] *= s;
}

void normalize(GLdouble U[3]){
	GLfloat s = 1.0/sqrtf(dot(U, U));
	scale(s, U);
}

void spline_mesh(void){
	static GLboolean first = GL_TRUE;
	static GLuint vertBuffer;
	static GLuint normalBuffer;
	static GLuint indexBuffer;
	GLdouble tube[TUBE_N][TUBE_M][3];
	GLdouble tubeNormals[TUBE_N][TUBE_M][3];

	if(first){
		double t, u;
		int i, j, k;
		double dt = 2 * M_PI / TUBE_N;
		double du = 2 * M_PI / TUBE_M;

		for(i = 0, t = 0.0; i < TUBE_N; i++, t += dt){
			GLdouble x = (a + b * cos(q * t)) * cos(p * t);
			GLdouble y = (a + b * cos(q * t)) * sin(p * t);
			GLdouble z = b * sin(q * t);
			GLdouble tangx = -p * y - b * q * sin(q * t) * cos(p * t);
			GLdouble tangy = p * x - b * q * sin(q * t) * sin(p * t);
			GLdouble tangz = b * q * cos(q * t);
			GLdouble accelx = -p * tangy + b * q * (p * sin(q * t) * sin(p * t) - q * cos(q * t) * cos(p * t));
			GLdouble accely = p * tangx - b * q * (p * sin(q * t) * cos(p * t) + q * cos(q * t) * sin(p * t));
			GLdouble accelz = -(q * q) * b * sin(q * t);
			GLdouble C[3] = {x, y, z};
			GLdouble T[3] = {tangx, tangy, tangz};
			GLdouble A[3] = {accelx, accely, accelz};
			GLdouble N[3];
			GLdouble B[3];
			cross(T, A, B);
			normalize(T);
			normalize(B);
			cross(B, T, N);
			for(j = 0, u = 0.0; j < TUBE_M; j++, u += du){
				for(k = 0; k < 3; k++){
					tubeNormals[i][j][k] = cos(u) * B[k] + sin(u) * N[k];
					tube[i][j][k] = C[k] + R * tubeNormals[i][j][k];
				}
			}
		}
		createMeshStripIndices();
		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tubeStrips), tubeStrips, GL_STATIC_DRAW);

		glGenBuffers(1, &vertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tube), tube, GL_STATIC_DRAW);
	
		glGenBuffers(1, &normalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tubeNormals), tubeNormals, GL_STATIC_DRAW);	

		first = GL_FALSE;
	}

	loadUniforms();

	glEnableVertexAttribArray(vertexPositionAttr);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glVertexAttribPointer(vertexPositionAttr, 3, GL_DOUBLE, GL_FALSE, 0, (GLvoid*) 0);
	
	glEnableVertexAttribArray(vertexNormalAttr);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer(vertexNormalAttr, 3, GL_DOUBLE, GL_FALSE, 0, (GLvoid*) 0);

	int i;
	char *offset;
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	for(i = 0, offset = 0; i < NUM_STRIPS; i++, offset += INDICES_PER_STRIP * sizeof(GLushort)){
		glDrawElements(GL_QUAD_STRIP, INDICES_PER_STRIP, GL_UNSIGNED_SHORT, (GLvoid*) offset);
	}
}

void reshape(int w, int h) {
  glViewport(0,0, w,h);
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT, GL_FILL);

	for(int i = 0; i < 3; i++){
		materialAmbient[i] = 0.2;
		materialSpecular[i] = 0.3;
	}
	materialDiffuse[0] = 0.5;
	materialDiffuse[1] = 0.5;
	materialDiffuse[2] = 0.5;
	materialShininess = 20.0;
	spline_mesh();
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
	"attribute vec4 vertexPosition;"
  "attribute vec3 vertexNormal;\n"
	"varying vec4 color;\n"
  "uniform mat4 ModelViewProjection;\n"
	"uniform mat4 ModelViewMatrix;\n"
	"uniform mat3 NormalMatrix;\n"
	"uniform vec3 ambientLight;\n"
	"uniform vec3 light0Color;\n"
	"uniform vec3 light0Position;\n"
	"uniform vec3 materialAmbient;\n"
	"uniform vec3 materialDiffuse;\n"
	"uniform vec3 materialSpecular;\n"
	"uniform float materialShininess;\n"
  "void main() {\n"
  "	gl_Position = ModelViewProjection*vertexPosition;\n"
	"	vec3 P = vec3(ModelViewMatrix*vertexPosition);\n"
	" vec3 N = normalize(NormalMatrix * vertexNormal);\n"
	"	vec3 L = normalize(light0Position - P);\n"
	"	vec3 I_ambient = materialAmbient * ambientLight;\n"
	" float cos_theta = dot(L, N);\n"
	" vec3 I_diffuse = materialDiffuse * light0Color * max(0.0, cos_theta);\n"
	"	vec3 I_specular = vec3(0.0, 0.0, 0.0);\n"
	"	if(cos_theta > 0.0){\n"
	"		vec3 R = reflect(-L,N);\n"
	"		vec3 V = normalize(-P);\n"
	"		float cos_alpha = dot(R,V);\n"
	"		I_specular = materialSpecular * light0Color * pow(max(0.0, cos_alpha),materialShininess);\n"
	"	}\n"
	"	vec3 I = I_ambient + I_diffuse + I_specular;\n"
	"	color = vec4(I, 1.0);\n"
  "}\n"
};

//
// GLSL source code for our fragment shader.
//
const GLchar *fragmentShaderSource = {
  "varying vec4 color;\n"
  "void main() {\n"
  "  gl_FragColor = color;\n"
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
	vertexNormalAttr = glGetAttribLocation(program, "vertexNormal");
  if (vertexPositionAttr == -1 || vertexNormalAttr == -1) {
    fprintf(stderr, "Error fetching vertex position attribute!\n");
    exit(-1);
  }

  //
  // (8) Fetch handles for uniform variables in program.
  // None, using built-ins.
  //
  ModelViewProjectionUniform = glGetUniformLocation(program, "ModelViewProjection");
	ModelViewMatrixUniform = glGetUniformLocation(program, "ModelViewMatrix");
	NormalMatrixUniform = glGetUniformLocation(program, "NormalMatrix");
	ambientLightUniform = glGetUniformLocation(program, "ambientLight");
	light0ColorUniform = glGetUniformLocation(program, "light0Color");
	light0PositionUniform = glGetUniformLocation(program, "light0Position");
	materialAmbientUniform = glGetUniformLocation(program, "materialAmbient");
	materialDiffuseUniform = glGetUniformLocation(program, "materialDiffuse");
	materialSpecularUniform = glGetUniformLocation(program, "materialSpecular");
	materialShininessUniform = glGetUniformLocation(program, "materialShininess");

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
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(500,500);
  glutInitWindowPosition(10,10);
  glutCreateWindow("Spline");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
  installShaders();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, 1.0, 0.1, 1000);
	setView();

  glutMainLoop();
  return 0;
}
