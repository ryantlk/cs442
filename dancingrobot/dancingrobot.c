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

#define torso_angle_max -15
#define torso_angle_min 15
#define head_angle_max -10
#define head_angle_min 10
#define right_upper_arm_angle_max 0
#define right_upper_arm_angle_min 30
#define right_lower_arm_angle_max -30
#define right_lower_arm_angle_min 30
#define right_wrist_angle_max -25
#define right_wrist_angle_min 25
#define left_upper_arm_angle_max -30
#define left_upper_arm_angle_min 0
#define left_lower_arm_angle_max -30
#define left_lower_arm_angle_min 30
#define left_wrist_angle_max -25
#define left_wrist_angle_min 25
#define right_upper_leg_angle_max -15
#define right_upper_leg_angle_min 15
#define right_lower_leg_angle_max -15
#define right_lower_leg_angle_min 25
#define right_foot_angle_max -15
#define right_foot_angle_min 15
#define left_upper_leg_angle_max -15
#define left_upper_leg_angle_min 15
#define left_lower_leg_angle_max -15
#define left_lower_leg_angle_min 25
#define left_foot_angle_max -15
#define left_foot_angle_min 15

double torso_angle;
double head_angle;
double right_upper_arm_angle;
double right_lower_arm_angle;
double right_wrist_angle;
double left_upper_arm_angle;
double left_lower_arm_angle;
double left_wrist_angle;
double right_upper_leg_angle;
double right_lower_leg_angle;
double right_foot_angle;
double left_upper_leg_angle;
double left_lower_leg_angle;
double left_foot_angle;

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
  matrixMultiply(ModelViewProjection, Projection, ModelView);
  glUniformMatrix4fv(ModelViewProjectionUniform, 1, GL_FALSE,
                     ModelViewProjection);
  glUniform3fv(ColorUniform, 1, Color);
}

void arm_segment(void){
	static GLboolean first = GL_TRUE;
	static GLuint vertBuffer;
	if(first){
		static GLfloat verts[4][2] = {
			{.5, 0}, {.5, -6}, {-.5, -6}, {-.5,0}
		};
		glGenBuffers(1, &vertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		first = GL_FALSE;
	}
	loadUniforms();
	glEnableVertexAttribArray(vertexPositionAttr);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glVertexAttribPointer(vertexPositionAttr, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void torso(void){
	static GLboolean first = GL_TRUE;
	static GLuint vertBuffer;
	if(first){
		static GLfloat verts[10][2] = {
			{-2.5, 0}, {-2.5, 2}, {-3.5, 6}, {-4.5, 7}, {-1.5, 10}, 
			{1.5, 10}, {4.5, 7}, {3.5, 6}, {2.5, 2}, {2.5, 0}
		};
		glGenBuffers(1, &vertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		first = GL_FALSE;
	}
	loadUniforms();
	glEnableVertexAttribArray(vertexPositionAttr);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glVertexAttribPointer(vertexPositionAttr, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glDrawArrays(GL_LINE_LOOP, 0, 10);
}

void lower_torso(void){
	static GLboolean first = GL_TRUE;
	static GLuint vertBuffer;
	if(first){
		static GLfloat verts[6][2] = {
			{-2.5, 0}, {2.5, 0}, {2.5, -1}, {1.5, -2}, {-1.5, -2}, {-2.5, -1}
		};
		glGenBuffers(1, &vertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		first = GL_FALSE;
	}
	loadUniforms();
	glEnableVertexAttribArray(vertexPositionAttr);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glVertexAttribPointer(vertexPositionAttr, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glDrawArrays(GL_LINE_LOOP, 0, 6);
}

void hand(void){
	static GLboolean first = GL_TRUE;
	static GLuint vertBuffer;
	if(first){
		static GLfloat verts[8][2] = {
			{1, 0}, {1, -1}, {.5, -1}, {.5, -.5}, {-.5, -.5}, 
			{-.5, -1}, {-1, -1}, {-1, 0}
		};
		glGenBuffers(1, &vertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		first = GL_FALSE;
	}
	loadUniforms();
	glEnableVertexAttribArray(vertexPositionAttr);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glVertexAttribPointer(vertexPositionAttr, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glDrawArrays(GL_LINE_LOOP, 0, 8);
}

void eye(void){
	static GLboolean first = GL_TRUE;
	static GLuint vertBuffer;
	if(first){
		static GLfloat verts[4][2] = {
			{-1, 0}, {-1, .5}, {1, .5}, {1, 0}
		};
		glGenBuffers(1, &vertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		first = GL_FALSE;
	}
	loadUniforms();
	glEnableVertexAttribArray(vertexPositionAttr);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glVertexAttribPointer(vertexPositionAttr, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void head(void){
	static GLboolean first = GL_TRUE;
	static GLuint vertBuffer;
	if(first){
		static GLfloat verts[10][2] = {
			{-1.5, 0}, {-1.5, 3}, {-.5, 4}, {.5, 4}, {1.5, 3}, 
			{1.5, 1.5}, {0, 1.5}, {0, .5}, {1.5, .5}, {1.5, 0}
		};
		glGenBuffers(1, &vertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		first = GL_FALSE;
	}
	loadUniforms();
	glEnableVertexAttribArray(vertexPositionAttr);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glVertexAttribPointer(vertexPositionAttr, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glDrawArrays(GL_LINE_LOOP, 0, 10);
}

void foot(void){
	static GLboolean first = GL_TRUE;
	static GLuint vertBuffer;
	if(first){
		static GLfloat verts[6][2] = {
			{0, 0}, {1, -1}, {2, -1}, {2, -2}, {-1, -2}, {-1, -1}
		};
		glGenBuffers(1, &vertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		first = GL_FALSE;
	}
	loadUniforms();
	glEnableVertexAttribArray(vertexPositionAttr);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glVertexAttribPointer(vertexPositionAttr, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glDrawArrays(GL_LINE_LOOP, 0, 6);
}

void joint(void){
#define N 30
	static GLboolean first = GL_TRUE;
	static GLuint vertBuffer;
	if(first){
		GLfloat verts[N][2];
		for(int i = 0; i < N; i++){
			verts[i][0] = cos(i * 2 * M_PI / N);
			verts[i][1] = sin(i * 2 * M_PI / N);
		}
		glGenBuffers(1, &vertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		first = GL_FALSE;
	}
	loadUniforms();
	glEnableVertexAttribArray(vertexPositionAttr);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glVertexAttribPointer(vertexPositionAttr, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glDrawArrays(GL_LINE_LOOP, 0, N);
}

void reshape(int w, int h) {
  glViewport(0,0, w,h);

  matrixIdentity(Projection);
  if (w > h)
    matrixOrtho(Projection, -20.0, 20.0, -20.0*h/w, 20.0*w/h, -1, +1);
  else
    matrixOrtho(Projection, -20.0*w/h, 20.0*w/h, -20.0, 20.0, -1, +1);

  matrixIdentity(ModelView);
}

void fist(float wristangle){
	matrixPush(ModelView);
	matrixRotate(ModelView, wristangle, 0, 0, 1);

	matrixPush(ModelView);
	matrixScale(ModelView, .5, .5, 1);
	joint();
	matrixPop(ModelView);
	
	hand();

	matrixPop(ModelView);
}

void forearm(float lowerarmangle, float wristangle){
	matrixPush(ModelView);
	matrixRotate(ModelView, lowerarmangle, 0, 0, 1);
	
	matrixPush(ModelView);
	matrixScale(ModelView, .5, .5, 1);
	joint();
	matrixPop(ModelView);

	arm_segment();

	matrixPush(ModelView);
	matrixTranslate(ModelView, 0, -6, 0);
	fist(wristangle);
	matrixPop(ModelView);
	

	matrixPop(ModelView);
}

void arm(float upperarmangle, float lowerarmangle, float wristangle){
	matrixPush(ModelView);
	matrixRotate(ModelView, upperarmangle, 0, 0, 1);

	matrixPush(ModelView);
	matrixScale(ModelView, .625, .625, 1);
	joint();
	matrixPop(ModelView);

	matrixPush(ModelView);
	matrixScale(ModelView, 1.25, 1, 1);
	arm_segment();
	matrixPop(ModelView);

	matrixTranslate(ModelView, 0, -6, 0);
	forearm(lowerarmangle, wristangle);
	
	matrixPop(ModelView);
}

void feet(float footangle){
	matrixPush(ModelView);
	matrixRotate(ModelView, footangle, 0, 0, 1);
	
	matrixPush(ModelView);
	matrixScale(ModelView, .9, .9, 1);
	joint();
	matrixPop(ModelView);

	matrixPush(ModelView);
	matrixScale(ModelView, -1, 1, 1);
	foot();
	matrixPop(ModelView);

	matrixPop(ModelView);
}

void lower_leg(float lowerlegangle, float footangle){
	matrixPush(ModelView);
	matrixRotate(ModelView, lowerlegangle, 0, 0, 1);

	matrixPush(ModelView);
	matrixScale(ModelView, .9, .9, 1);
	joint();
	matrixPop(ModelView);

	matrixPush(ModelView);
	matrixScale(ModelView, 1.8, .8 , 1);
	arm_segment();
	matrixPop(ModelView);

	matrixTranslate(ModelView, 0, -4.8, 0);
	feet(footangle);

	matrixPop(ModelView);
}

void leg(float upperlegangle, float lowerlegangle, float footangle){
	matrixPush(ModelView);
	matrixRotate(ModelView, upperlegangle, 0, 0, 1);

	joint();

	matrixPush(ModelView);
	matrixScale(ModelView, 2, 1, 1);
	arm_segment();
	matrixPop(ModelView);


	matrixTranslate(ModelView, 0, -6, 0);
	lower_leg(lowerlegangle, footangle);	

	matrixPop(ModelView);
}

void right_leg(float upperlegangle, float lowerlegangle, float footangle){
	matrixPush(ModelView);
	matrixScale(ModelView, -1, 1, 1);
	leg(upperlegangle, lowerlegangle, footangle);
	matrixPop(ModelView);
}

double right_upper_arm_angle;
double right_lower_arm_angle;
double right_wrist_angle;
double left_upper_arm_angle;
double left_lower_arm_angle;
double left_wrist_angle;
double right_upper_leg_angle;
double right_lower_leg_angle;
double right_foot_angle;
double left_upper_leg_angle;
double left_lower_leg_angle;
double left_foot_angle;

void robot(){
	matrixPush(ModelView);
	matrixRotate(ModelView, torso_angle, 0, 0, 1);
	torso();

	matrixPush(ModelView);
	matrixTranslate(ModelView, 0, 10.2, 0);
	matrixRotate(ModelView, head_angle, 0, 0, 1);
	head();
	matrixPush(ModelView);
	matrixTranslate(ModelView, .5, 2.5, 0);
	eye();
	matrixPop(ModelView);
	matrixPop(ModelView);

	matrixPush(ModelView);
	matrixTranslate(ModelView, -4.5, 7, 0);
	matrixRotate(ModelView, -45, 0, 0, 1);
	arm(left_upper_arm_angle, left_lower_arm_angle, left_wrist_angle);
	matrixPop(ModelView);

	matrixPush(ModelView);
	matrixTranslate(ModelView, 4.5, 7, 0);
	matrixRotate(ModelView, 45, 0, 0, 1);
	arm(right_upper_arm_angle, right_lower_arm_angle, right_wrist_angle);
	matrixPop(ModelView);

	matrixPush(ModelView);
	lower_torso();
	matrixPop(ModelView);

	matrixPush(ModelView);
	matrixTranslate(ModelView, -2, -1.5, 0);
	matrixRotate(ModelView, -30, 0, 0, 1);
	leg(left_upper_leg_angle, left_lower_leg_angle, left_foot_angle);
	matrixPop(ModelView);

	matrixPush(ModelView);
	matrixTranslate(ModelView, 2, -1.5, 0);
	matrixRotate(ModelView, 30, 0, 0, 1);
	right_leg(right_upper_leg_angle, right_lower_leg_angle, right_foot_angle);
	matrixPop(ModelView);

	matrixPop(ModelView);
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT);
	robot(torso_angle, head_angle);
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
  "attribute vec2 vertexPosition;"
  "uniform mat4 ModelViewProjection;\n" 
  "void main() {\n"
  "  gl_Position = ModelViewProjection*vec4(vertexPosition, 0.0, 1.0);\n"
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

float getAngle(float freq, float min, float max, float t){
	return (max - min) * sin(freq * 2 * M_PI * t) + .5 * (min + max);
}

static void idle(void){
	GLfloat seconds = glutGet(GLUT_ELAPSED_TIME)/1000.0;
	
	torso_angle = getAngle(.7, torso_angle_min, torso_angle_max, seconds);
	head_angle = getAngle(.7, head_angle_min, head_angle_max, seconds);
	right_upper_arm_angle = getAngle(.5, right_upper_arm_angle_min, 
									right_upper_arm_angle_max, seconds);
	right_lower_arm_angle = getAngle(.35, right_lower_arm_angle_min, 
									right_lower_arm_angle_max, seconds);
	right_wrist_angle = getAngle(.75, right_wrist_angle_min, right_wrist_angle_max, seconds);
	left_upper_arm_angle = getAngle(.35, left_upper_arm_angle_min, 
									left_upper_arm_angle_max, seconds);
	left_lower_arm_angle = getAngle(.5, left_lower_arm_angle_min, 
									left_lower_arm_angle_max, seconds);
	left_wrist_angle = getAngle(.85, left_wrist_angle_min, left_wrist_angle_max, seconds);
	right_upper_leg_angle = getAngle(.5, right_upper_leg_angle_min, 
									right_upper_leg_angle_max, seconds);
	right_lower_leg_angle = getAngle(.35, right_lower_leg_angle_min, 
									right_lower_leg_angle_max, seconds);
	right_foot_angle = getAngle(.75, right_foot_angle_min, right_foot_angle_max, seconds);
	left_upper_leg_angle = getAngle(.35, left_upper_leg_angle_min, 
									left_upper_leg_angle_max, seconds);
	left_lower_leg_angle = getAngle(.5, left_lower_leg_angle_min, 
									left_lower_leg_angle_max, seconds);
	left_foot_angle = getAngle(.75, left_foot_angle_min, left_foot_angle_max, seconds);

	glutPostRedisplay();
}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
  glutInitWindowSize(500,500);
  glutInitWindowPosition(10,10);
  glutCreateWindow("Robot Left Arm");
  glutDisplayFunc(display);
	glutIdleFunc(idle);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  installShaders();
  Color[0] = 1.0; Color[1] = 1.0; Color[2] = 0.0;
  glutMainLoop();
  return 0;
}
