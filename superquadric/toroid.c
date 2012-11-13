#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
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
#define epsilon .0000001
#define IMAGENAME "mars.ppm"

typedef struct{
	int W, H;
	GLubyte (*rgba)[4];
}Image;

static GLuint texName;

int N = 60;
int M = 60;
int m = 2;
int n = 2;

GLboolean needsredraw = GL_TRUE;

GLfloat fovy = 40.0;
GLdouble eyeRho = 5;
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

static void getppm(char *fname, Image *image){
	FILE *f;
	static char buf[100];
	GLboolean gotSize, gotMax;
	int r, c;
	int W, H;
	GLubyte (*rgba)[4];

	if((f = fopen(fname, "rb")) == NULL){
		perror(fname);
		exit(-1);
	}
	if(fgets(buf, sizeof(buf), f) == NULL || strncmp("P6", buf, 2) != 0){
		fprintf(stderr, "Bogus magic string for raw PPM file '%s'!\n", fname);
		exit(-1);
	}
	for(gotSize = gotMax = GL_FALSE; !gotMax; ){
		if(fgets(buf, sizeof(buf), f) == NULL){
			fprintf(stderr, "Error parsing header for PPM file '%s'!\n", fname);
			exit(-1);
		}
		if(buf[0] == '#')
			continue;
		if(!gotSize){
			if(sscanf(buf, "%d %d", &W, &H) != 2 || W <= 0 || H <= 0){
				fprintf(stderr, "Bogus size for PPM file '%s'!\n", fname);
				exit(-1);
			}
			gotSize = GL_TRUE;
		}else{
			int max;
			if(sscanf(buf, "%d", &max) != 1 || max < 2 || max > 255){
				fprintf(stderr, "Bogus max intensity for PPM file '%s'!\n", fname);
				exit(-1);
			}
			gotMax = GL_TRUE;
		}
	}
	if((rgba = (GLubyte (*)[4]) calloc(W*H,4)) == NULL){
		perror("calloc() image");
		exit(-1);
	}
	for(r = H - 1; r >= 0; r--)
		for(c = 0; c < W; c++){
			unsigned char rgb[3];
			int index;
			if(fread(rgb, 1, 3, f) != 3){
				if(ferror(f))
					perror(fname);
				else
					fprintf(stderr, "Unexpected EOF: %s\n", fname);
				exit(-1);
			}
			index = r * W + c;
			rgba[index][0] = rgb[0];
			rgba[index][1] = rgb[1];
			rgba[index][2] = rgb[2];
			rgba[index][3] = 255;
		}
	fclose(f);
	image->W = W;
	image->H = H;
	image->rgba = rgba;
}

static void getSuperTexture(void){
	static char *ppmName = IMAGENAME;
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glGenTextures(1, &texName);
	Image image;
	getppm(ppmName, &image);
	glBindTexture(GL_TEXTURE_2D, texName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.W, image.H, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.rgba);
	free(image.rgba);
}

//
// Client matrices and color.
//
GLfloat ModelView[4*4];
GLfloat Projection[4*4];
GLfloat Color[4];

GLfloat ambientLight[3] = {1.0, 1.0, 1.0};
GLfloat light0Color[3] = {1.0, 1.0, 1.0};
GLfloat light0Position[3] = {10.0, 10.0, 10.0};

GLfloat materialAmbient[3];
GLfloat materialDiffuse[3];
GLfloat materialSpecular[3];
GLfloat materialShininess;

//
// Vertex Posistion Attribute
//
GLint vertexPositionAttr;
GLint vertexNormalAttr;
GLint vertexTexCoordAttr;

//
// GL uniform handles
//
GLint ModelViewProjectionUniform;
GLint ModelViewMatrixUniform;
GLint NormalMatrixUniform;
GLint TextureMatrixUniform;

GLint ambientLightUniform;
GLint light0ColorUniform;
GLint light0PositionUniform;

GLint materialAmbientUniform;
GLint materialDiffuseUniform;
GLint materialSpecularUniform;
GLint materialShininessUniform;

GLint TexUnitUniform;
GLint ColorUniform;

void loadUniforms() {
	GLfloat ModelViewProjection[4*4], NormalMatrix[3*3], TextureMatrix[4*4];
	glGetFloatv(GL_MODELVIEW_MATRIX, ModelView);
	glGetFloatv(GL_PROJECTION_MATRIX, Projection);
	matrixMultiply(ModelViewProjection, Projection, ModelView);
	matrixNormal(ModelView, NormalMatrix);
	matrixIdentity(TextureMatrix);
	
	glUniformMatrix4fv(ModelViewProjectionUniform, 1, GL_FALSE,
						ModelViewProjection);
	glUniformMatrix4fv(ModelViewMatrixUniform, 1, GL_FALSE, ModelView);
	glUniformMatrix4fv(TextureMatrixUniform, 1, GL_FALSE, TextureMatrix);
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
		
	glUniform3fv(ColorUniform, 1, Color);
	glUniform1i(TexUnitUniform, 0);
}

void spline(void){
//	static GLboolean first = GL_TRUE;
	static GLuint vertBuffer;
	static GLuint indexBuffer;
	static GLuint normBuffer;
	static GLuint texCoordBuffer;
	static GLfloat *verts = NULL;
	static GLushort *indices = NULL;
	static GLfloat *norms = NULL;
	static GLfloat *texCoords = NULL;
	GLfloat longs[N + 1];
	GLfloat lats[M + 1];
	if(needsredraw){
		verts = realloc(verts, sizeof(GLfloat) * (M + 1) * (N + 1) * 3);
		norms = realloc(norms, sizeof(GLfloat) * (N + 1) * (M + 1) * (3));
		indices = realloc(indices, sizeof(GLfloat) * (N * (2 * (M + 1) + 4) - 4));
		texCoords = realloc(texCoords, sizeof(GLfloat) * (M + 1) * (N + 1) * 2);
		memset(longs, 0, sizeof(GLfloat) * (N + 1));
		memset(lats, 0, sizeof(GLfloat) * (M + 1));
		

		for(int i = 0; i < N + 1; i++){
			for(int j = 0; j < M + 1; j++){
				GLdouble v = (M_PI / N) * i - M_PI / 2;
				GLdouble u = (2.0 * M_PI / M) * j - M_PI;
				GLfloat abscosv = fabsf(cosf(v));
				GLfloat abssinv = fabsf(sinf(v));
				GLfloat abscosu = fabsf(cosf(u));
				GLfloat abssinu = fabsf(sinf(u));
				texCoords[2 * (i * (M + 1) + j) + 1] = i/(GLfloat)N;
				texCoords[2 * (i * (M + 1) + j) + 0] = j/(GLfloat)M;
				//printf("%f %f\n", texCoords[
				if(abscosv < epsilon || abscosu < epsilon){
					verts[3 * (i * (M + 1) + j) + 0] = 0.0;
					norms[3 * (i * (M + 1) + j) + 0] = 0.0;
				}else{
					verts[3 * (i * (M + 1) + j) + 0] = cosf(v) * powf(abscosv, (2.0 / m) - 1) * cosf(u) * powf(abscosu, (2.0 / n) - 1);
					norms[3 * (i * (M + 1) + j) + 0] = cosf(v) * powf(abscosv, (1 - 2.0 / m)) * cosf(u) * powf(abscosu, (1 - 2.0 / n));
				}
				if(abscosv < epsilon || abssinu < epsilon){
					verts[3 * (i * (M + 1) + j) + 1] = 0.0;
					norms[3 * (i * (M + 1) + j) + 1] = 0.0;
				}else{
					verts[3 * (i * (M + 1) + j) + 1] = cosf(v) * powf(abscosv, (2.0 / m) - 1) * sinf(u) * powf(abssinu, (2.0 / n) - 1);
					norms[3 * (i * (M + 1) + j) + 1] = cosf(v) * powf(abscosv, (1 - 2.0 / m)) * sinf(u) * powf(abssinu, (1 - 2.0 / n));
				}
				if(abssinv < epsilon){
					verts[3 * (i * (M + 1) + j) + 2] = 0.0;
					norms[3 * (i * (M + 1) + j) + 2] = 0.0;
				}else{	
					verts[3 * (i * (M + 1) + j) + 2] = sinf(v) * powf(abssinv, (2.0 / m) - 1);
					norms[3 * (i * (M + 1) + j) + 2] = sinf(v) * powf(abssinv, (1 - 2.0 / m));
				}
				if(i > 0){
					GLfloat x1 = verts[3 * ((i - 1) * (M + 1) + j) + 0];
					GLfloat y1 = verts[3 * ((i - 1) * (M + 1) + j) + 1];
					GLfloat z1 = verts[3 * ((i - 1) * (M + 1) + j) + 2];
					GLfloat x2 = verts[3 * (i * (M + 1) + j) + 0];
					GLfloat y2 = verts[3 * (i * (M + 1) + j) + 1];
					GLfloat z2 = verts[3 * (i * (M + 1) + j) + 2];
					GLfloat dist = sqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)+(z2-z1)*(z2-z1));
					lats[j] += dist;
				}
				if(j > 0){
					GLfloat x1 = verts[3 * (i * (M + 1) + j - 1) + 0];
					GLfloat y1 = verts[3 * (i * (M + 1) + j - 1) + 1];
					GLfloat z1 = verts[3 * (i * (M + 1) + j - 1) + 2];
					GLfloat x2 = verts[3 * (i * (M + 1) + j) + 0];
					GLfloat y2 = verts[3 * (i * (M + 1) + j) + 1];
					GLfloat z2 = verts[3 * (i * (M + 1) + j) + 2];
					GLfloat dist = sqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)+(z2-z1)*(z2-z1));
					longs[i] += dist;
				}
			}
		}

		for(int i = 0; i < N + 1; i++){
			GLfloat dist = 0.0;
			for(int j = 0; j < M + 1; j++){
				if(j == 0){
					texCoords[2 * (i * (M + 1) + j) + 0] = 0;
				}else{
					GLfloat x1 = verts[3 * (i * (M + 1) + j - 1) + 0];
					GLfloat y1 = verts[3 * (i * (M + 1) + j - 1) + 1];
					GLfloat z1 = verts[3 * (i * (M + 1) + j - 1) + 2];
					GLfloat x2 = verts[3 * (i * (M + 1) + j) + 0];
					GLfloat y2 = verts[3 * (i * (M + 1) + j) + 1];
					GLfloat z2 = verts[3 * (i * (M + 1) + j) + 2];
					dist += sqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)+(z2-z1)*(z2-z1));
					texCoords[2 * (i * (M + 1) + j) + 0] = dist / longs[i];
				}
			}
		}
		for(int j = 0; j < M + 1; j++){
			GLfloat dist = 0.0;
			for(int i = 0; i < N + 1; i++){
				if(i == 0){
					texCoords[2 * (i * (M + 1) + j) + 1] = 0.0;
				}else if(j == M){
					texCoords[2 * (i * (M + 1) + j) + 1] = texCoords[2 * (i * (M + 1)) + 1];
				}else{
					GLfloat x1 = verts[3 * ((i - 1) * (M + 1) + j) + 0];
					GLfloat y1 = verts[3 * ((i - 1) * (M + 1) + j) + 1];
					GLfloat z1 = verts[3 * ((i - 1) * (M + 1) + j) + 2];
					GLfloat x2 = verts[3 * (i * (M + 1) + j) + 0];
					GLfloat y2 = verts[3 * (i * (M + 1) + j) + 1];
					GLfloat z2 = verts[3 * (i * (M + 1) + j) + 2];
					dist += sqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)+(z2-z1)*(z2-z1));
					texCoords[2 * (i * (M + 1) + j) + 1] = dist / lats[j];
				}
			}
		}
		for(int i = 0; i < M + 1; i++){
			texCoords[2 * i] = texCoords[2 * ((M + 1) + i) + 0];
			texCoords[2 * (N * (M + 1) + i) + 0] = texCoords[2 * ((N - 1) * (M + 1) + i) + 0];
		}
		
		for(int i = 0; i < N; i++){
			for(int j = 0; j < M + 1; j++){
				int index = i * (2 * (M + 1) + 4) + j * 2;
				indices[index] = i * (M + 1) + j;
				indices[index + 1] = i * (M + 1) + j + M + 1;
				if(j == M && i != N -1){
					indices[index + 2] = i * (M + 1) + j + M + 1;
					indices[index + 3] = i * (M + 1) + j + M + 1;
					indices[index + 4] = (i + 1) * (M + 1);// + j + M + 1;
					indices[index + 5] = (i + 1) * (M + 1);// + j + M + 1;
				}
			}
		}

		printf("N = %d, M = %d\n", N, M);
		printf("n = %d, m = %d\n", n, m);

		if(normBuffer == 0){
			glGenBuffers(1, &normBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, normBuffer);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat) * (N + 1) * (M + 1), norms, GL_DYNAMIC_DRAW);

		if(vertBuffer == 0){
			glGenBuffers(1, &vertBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat) * (N + 1) * (M + 1), verts, GL_DYNAMIC_DRAW);

		if(indexBuffer == 0){
			glGenBuffers(1, &indexBuffer);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
									sizeof(GLushort) * (N * (2 * (M + 1) + 4) - 4), indices, GL_DYNAMIC_DRAW);
		if(texCoordBuffer == 0){
			glGenBuffers(1, &texCoordBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * (M + 1) * (N + 1) * 2, texCoords, GL_DYNAMIC_DRAW);
		needsredraw = GL_FALSE;
	}
	loadUniforms();
	glEnableVertexAttribArray(vertexPositionAttr);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	glVertexAttribPointer(vertexPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);

	glEnableVertexAttribArray(vertexNormalAttr);
	glBindBuffer(GL_ARRAY_BUFFER, normBuffer);
	glVertexAttribPointer(vertexNormalAttr, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);

	glEnableVertexAttribArray(vertexTexCoordAttr);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
	glVertexAttribPointer(vertexTexCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_QUAD_STRIP, (N * (2 * (M + 1) + 4) - 4), GL_UNSIGNED_SHORT, (GLvoid*) 0);
}

void reshape(int w, int h) {
	glViewport(0,0, w,h);
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, Color);
	glPolygonMode(GL_FRONT, GL_FILL);
	for(int i = 0; i < 3; i++){
		materialSpecular[i] = 0.9;
	}
	materialAmbient[0] = 0.59;
	materialAmbient[1] = 0.36;
	materialAmbient[2] = 0.32;
	materialDiffuse[0] = 1.0;
	materialDiffuse[1] = 1.0;
	materialDiffuse[2] = 1.0;
	materialShininess = 40.0;
	spline();
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
#define ESC 27
	if (key == ESC) exit(0);
#undef ESC
	//increase/decrease slices/dices
	if(key == 'n'){
		N--;
		needsredraw = GL_TRUE;
		glutPostRedisplay();
	}
	if(key == 'm'){
		M--;
		needsredraw = GL_TRUE;
		glutPostRedisplay();
	}
	if(key == 'M'){
		M++;
		needsredraw = GL_TRUE;
		glutPostRedisplay();
	}
	if(key == 'N'){
		N++;
		needsredraw = GL_TRUE;
		glutPostRedisplay();
	}
	if(key == 'j'){
		n--;;
		needsredraw = GL_TRUE;
		glutPostRedisplay();
	}
	if(key == 'J'){
		n++;
		needsredraw = GL_TRUE;
		glutPostRedisplay();
	}
	if(key == 'k'){
		m--;;
		needsredraw = GL_TRUE;
		glutPostRedisplay();
	}
	if(key == 'K'){
		m++;
		needsredraw = GL_TRUE;
		glutPostRedisplay();
	}
}

//
// GLSL source code for our vertex shader.
//
const GLchar *vertexShaderSource = {
	"attribute vec4 vertexPosition;"
	"attribute vec3 vertexNormal;\n"
	"attribute vec2 vertexTexCoord;\n"
	"varying vec2 texCoord;\n"
	"varying vec4 Color;\n"
	"uniform mat4 TextureMatrix;\n"
	"uniform mat4 ModelViewProjection;\n"
	"uniform mat4 ModelViewMatrix;\n"
	"uniform mat3 NormalMatrix;\n"
	"uniform vec3 light0Color;\n"
	"uniform vec3 light0Position;\n"
	"uniform vec3 materialAmbient;\n"
	"uniform vec3 materialDiffuse;\n"
	"uniform vec3 materialSpecular;\n"
	"uniform vec3 ambientLight;\n"
	"uniform float materialShininess;\n"
	
	"void main() {\n"
	"	gl_Position = ModelViewProjection*vertexPosition;\n"
	"	texCoord = (TextureMatrix * vec4(vertexTexCoord, 0.0, 1.0)).st;\n"
	"	vec3 P = vec3(ModelViewMatrix * vertexPosition);\n"
	"	vec3 N = normalize(NormalMatrix * vertexNormal);\n"
	"	vec3 L = normalize(light0Position - P);\n"
	"	float cos_theta = dot(L,N);\n"
	"	vec3 I_ambient = materialAmbient * ambientLight;\n"
	"	vec3 I_diffuse = materialDiffuse * light0Color * max(0.0, cos_theta);\n"
	"	vec3 I_specular = vec3(0.0, 0.0, 0.0);\n"
	"	if(cos_theta > 0.0)\n{"
	"		vec3 R = reflect(-L,N);\n"
	"		vec3 V = normalize(-P);\n"
	"		float cos_alpha = dot(R,V);\n"
	"		I_specular = materialSpecular * light0Color * pow(max(0.0, cos_alpha), materialShininess);\n"
	"	}\n"
	"	vec3 I = I_ambient + I_diffuse + I_specular;\n"
	"	Color = vec4(I, 1.0);\n"
 	"}\n"
};

//
// GLSL source code for our fragment shader.
//
const GLchar *fragmentShaderSource = {
	"uniform sampler2D texUnit;\n"
	"varying vec2 texCoord;\n"
	"varying vec4 Color;\n"
	"void main() {\n"
	"	vec4 texel = texture2D(texUnit, texCoord);\n"
	"	gl_FragColor = texel * Color;\n"
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
	vertexTexCoordAttr = glGetAttribLocation(program, "vertexTexCoord");
	if (vertexPositionAttr == -1 || vertexNormalAttr == -1 || vertexTexCoordAttr == -1) {
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
	TextureMatrixUniform = glGetUniformLocation(program, "TextureMatrix");
	TexUnitUniform = glGetUniformLocation(program, "texUnit");
	ambientLightUniform = glGetUniformLocation(program, "ambientLight");
	light0ColorUniform = glGetUniformLocation(program, "light0Color");
	light0PositionUniform = glGetUniformLocation(program, "light0Position");
	materialAmbientUniform = glGetUniformLocation(program, "materialAmbient");
	materialDiffuseUniform = glGetUniformLocation(program, "materialDiffuse");
	materialSpecularUniform = glGetUniformLocation(program, "materialSpecular");
	materialShininessUniform = glGetUniformLocation(program, "materialShininess");
	
	
//	ColorUniform = glGetUniformLocation(program,"Color");
	
	//
	// (9) Tell GL to use our program
	//
	glUseProgram(program);
}

int mousey, mousex;
//double epsilon = 0.0000001;

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
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500,500);
	glutInitWindowPosition(10,10);
	glutCreateWindow("SuperQuad");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	installShaders();
	getSuperTexture();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	glClearColor(1.0, 1.0, 1.0, 0.0);
	gluPerspective(40.0, 1.0, 0.1, 1000);
	setView();
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
	return 0;
}
