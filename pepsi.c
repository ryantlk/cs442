#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "matrix.h"
#include "bottle.h"
#if defined(__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

GLuint texName;

void loadTexture(void) {
  const char *fname = "pepsi.ppm";
  FILE *f = fopen(fname, "rb");
  if (f == NULL) {perror(fname); exit(-1);}
  char buf[50];
  if (fgets(buf, sizeof(buf), f) == NULL || strcmp("P6\n",buf) != 0) {
    fprintf(stderr, "%s not a raw PPM file!\n", fname);
    exit(-1);
  }
  while (fgets(buf, sizeof(buf), f) != NULL && buf[0] == '#')
    ;
  int W, H;
  if (sscanf(buf, "%d %d", &W, &H) != 2 || W <= 0 || H <= 0) {
    fprintf(stderr, "%s has bogus size!\n", fname);
    exit(-1);
  }
  while (fgets(buf, sizeof(buf), f) != NULL && buf[0] == '#')
    ;
  int maxval;
  if (sscanf(buf, "%d", &maxval) != 1 || maxval != 255) {
    fprintf(stderr, "%s has bogus max channel value!\n", fname);
    exit(-1);
  }
  GLubyte *pixels = (GLubyte *) malloc(W*H*3); 
  for (int r = H-1; r >= 0; r--) {   // flip upside down
    GLubyte *p = pixels + 3*W*r;
    if (fread(p, 3, W, f) != W) {
      fprintf(stderr, "%s : unable to read pixels!\n", fname);
      exit(-1);
    }
  }
  fclose(f);

  // XXX glPixelStorei(GL_PACK_ALIGNMENT, 1);    // XXX from GL to client
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // from client to GL
  
  glGenTextures(1, &texName);
  glBindTexture(GL_TEXTURE_2D, texName);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, 
	       GL_RGB, GL_UNSIGNED_BYTE, pixels);

  free(pixels);
}

//
// Client matrices, lights, and material property.
//
GLfloat ModelView[4*4];
GLfloat Projection[4*4];
GLfloat TextureMatrix[4*4];

GLfloat ambientLight[3] = {0.2, 0.2, 0.2};
GLfloat light0Color[3] = {1.0, 1.0, 1.0};
GLfloat light0Position[3] = {10.0, 10.0, 10.0};

GLfloat materialAmbient[3] = {0.3, 0.3, 0.3};
GLfloat materialDiffuse[3] = {0.3, 0.1, 0.0};
GLfloat labelDiffuse[3] = {1, 1, 1};
GLfloat materialSpecular[3] = {0.5, 0.5, 0.5};
GLfloat materialShininess = 20;

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
GLint labelDiffuseUniform;
GLint materialSpecularUniform;
GLint materialShininessUniform;

GLint texUnitUniform;

void loadUniforms() {
  //
  // Load matrices.
  //
  GLfloat ModelViewProjection[4*4], NormalMatrix[3*3];
  matrixMultiply(ModelViewProjection, Projection, ModelView);
  matrixNormal(ModelView, NormalMatrix);
  glUniformMatrix4fv(ModelViewProjectionUniform, 1, GL_FALSE,
                     ModelViewProjection);
  glUniformMatrix4fv(ModelViewMatrixUniform, 1, GL_FALSE, ModelView);
  glUniformMatrix3fv(NormalMatrixUniform, 1, GL_FALSE, NormalMatrix);
  glUniformMatrix4fv(TextureMatrixUniform, 1, GL_FALSE, TextureMatrix);

  //
  // Load lights.
  //
  glUniform3fv(ambientLightUniform, 1, ambientLight);
  glUniform3fv(light0ColorUniform, 1, light0Color);
  glUniform3fv(light0PositionUniform, 1, light0Position);

  //
  // Load material properties.
  //
  glUniform3fv(materialAmbientUniform, 1, materialAmbient);
  glUniform3fv(materialDiffuseUniform, 1, materialDiffuse);
  glUniform3fv(labelDiffuseUniform, 1, labelDiffuse);
  glUniform3fv(materialSpecularUniform, 1, materialSpecular);
  glUniform1f(materialShininessUniform, materialShininess);
 
  //
  // Only using texture unit 0.
  //
  glUniform1i(texUnitUniform, 0);
}

void bottle(void) {
  static GLuint vertBuffer;
  static GLuint normalBuffer;
  static GLuint texCoordBuffer;
  static GLuint indexBuffer;

  static GLboolean first = GL_TRUE;
  if (first) {
    glGenBuffers(1, &vertBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
    glBufferData(GL_ARRAY_BUFFER, NUM_BOTTLE_VERTS*3*sizeof(GLfloat),
                 bottleVerts, GL_STATIC_DRAW);

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, NUM_BOTTLE_VERTS*3*sizeof(GLfloat),
                 bottleNormals, GL_STATIC_DRAW);

    glGenBuffers(1, &texCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, NUM_BOTTLE_VERTS*2*sizeof(GLfloat),
                 bottleTexCoords, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 NUM_BOTTLE_QUAD_STRIP_INDICES*sizeof(GLushort),
                 bottleQuadStripIndices, GL_STATIC_DRAW);
    
    first = GL_FALSE;
  }

  loadUniforms();

  glEnableVertexAttribArray(vertexPositionAttr);
  glBindBuffer(GL_ARRAY_BUFFER, vertBuffer); 
  glVertexAttribPointer(vertexPositionAttr, 3, GL_FLOAT, 
                        GL_FALSE, 0, (GLvoid*) 0);

  glEnableVertexAttribArray(vertexNormalAttr);
  glBindBuffer(GL_ARRAY_BUFFER, normalBuffer); 
  glVertexAttribPointer(vertexNormalAttr, 3, GL_FLOAT, 
                        GL_FALSE, 0, (GLvoid*) 0);

  glEnableVertexAttribArray(vertexTexCoordAttr);
  glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer); 
  glVertexAttribPointer(vertexTexCoordAttr, 2, GL_FLOAT, 
                        GL_FALSE, 0, (GLvoid*) 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer); 
  for (int strip = 0; strip < NUM_BOTTLE_QUAD_STRIPS; strip++) {
    const int offset = VERTS_PER_BOTTLE_QUAD_STRIP*sizeof(GLushort)*strip;
    glDrawElements(GL_QUAD_STRIP, VERTS_PER_BOTTLE_QUAD_STRIP,
                   GL_UNSIGNED_SHORT, (GLchar*) 0 + offset);
  }
}

void bunchOfbottles(void) {
  const int N = 3;
  const GLfloat R = 4.0;
  const GLfloat ds = 2*R/(N-1);
  GLfloat y = -R;
  for (int j = 0; j < N; j++, y += ds) {
    GLfloat x = -R;
    for (int i = 0; i < N; i++, x += ds) {
      matrixPush(ModelView);
      matrixTranslate(ModelView, x, y, 0);
      bottle();
      matrixPop(ModelView);
    }
  }
}

void reshape(int w, int h) {
  glViewport(0,0, w,h);
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT, GL_FILL);

  bunchOfbottles();
  // bottle();

  glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
#define ESC 27
  if (key == ESC) exit(0);
}

int mousex, mousey;

void mouse(int button, int state, int x, int y) {
  if (state == GLUT_DOWN) {
    if (button == GLUT_LEFT_BUTTON) {
      mousex = x;
      mousey = y;
    }
  }
}

GLfloat R = 10.0, theta = M_PI/3, phi = M_PI/3; // eye in spherical coords
const GLfloat lookat[3] = {0, 0, 3.4};

void eyeToCartesion(GLfloat eye[3]) {
  const GLfloat sine_phi = sinf(phi);
  eye[0] = R*sine_phi*cosf(theta);
  eye[1] = R*sine_phi*sinf(theta);
  eye[2] = R*cosf(phi);
}

void setView(void) {
  const GLfloat epsilon = 0.001;
  if (phi <= 0)
    phi = epsilon;
  else if (phi >= M_PI)
    phi = M_PI - epsilon;
  GLfloat eye[3];
  eyeToCartesion(eye);
  matrixIdentity(ModelView);
  matrixLookat(ModelView,
               eye[0] + lookat[0], eye[1] + lookat[1], eye[2] + lookat[2],
	       lookat[0], lookat[1], lookat[2],
               0, 0, 1);
}

void mouseMotion(int x, int y) {
  const int dx = x - mousex;
  const int dy = y - mousey;
  const GLfloat degreesPerPixel = 1.0;
  const GLfloat toRadians = degreesPerPixel*M_PI/180;
  const GLfloat dtheta = -dx*toRadians;
  const GLfloat dphi = -dy*toRadians;
  theta += dtheta;
  phi += dphi;
  setView();
  mousex = x;
  mousey = y;
  glutPostRedisplay();
}

GLchar *getShaderSource(const char *fname) {
  FILE *f = fopen(fname, "r");
  if (f == NULL) {perror(fname); exit(-1);}
  fseek(f, 0L, SEEK_END);
  int len = ftell(f);
  rewind(f);
  GLchar *source = (GLchar *) malloc(len + 1);
  if (fread(source,1,len,f) != len) {
    if (ferror(f))
      perror(fname);
    else if (feof(f))
      fprintf(stderr, "Unexpected EOF when reading '%s'!\n", fname);
    else
      fprintf(stderr, "Unable to load '%s'!\n", fname);
    exit(-1);
  }
  source[len] = '\0';
  fclose(f);
  return source;
}

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
  const GLchar *vertexShaderSource = getShaderSource("vertex.vs");
  const GLchar *fragmentShaderSource = getShaderSource("fragment.fs");
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
  // (7) Get vertex attribute locations
  //
  vertexPositionAttr = glGetAttribLocation(program, "vertexPosition");
  vertexNormalAttr = glGetAttribLocation(program, "vertexNormal");
  vertexTexCoordAttr = glGetAttribLocation(program, "vertexTexCoord");
  if (vertexPositionAttr == -1 || vertexNormalAttr == -1 || vertexTexCoordAttr == -1) {
    fprintf(stderr, "Error fetching a vertex attribute!\n");
    exit(-1);
  }

  //
  // (8) Fetch handles for uniform variables in program.
  //
  ModelViewProjectionUniform = glGetUniformLocation(program, "ModelViewProjection");
  ModelViewMatrixUniform = glGetUniformLocation(program, "ModelViewMatrix");
  NormalMatrixUniform = glGetUniformLocation(program, "NormalMatrix");
  TextureMatrixUniform = glGetUniformLocation(program, "TextureMatrix");
  ambientLightUniform = glGetUniformLocation(program, "ambientLight");
  light0ColorUniform = glGetUniformLocation(program, "light0Color");
  light0PositionUniform = glGetUniformLocation(program, "light0Position");
  materialAmbientUniform = glGetUniformLocation(program, "materialAmbient");
  materialDiffuseUniform = glGetUniformLocation(program, "materialDiffuse");
  labelDiffuseUniform = glGetUniformLocation(program, "labelDiffuse");
  materialSpecularUniform = glGetUniformLocation(program, "materialSpecular");
  materialShininessUniform = glGetUniformLocation(program, "materialShininess");
  texUnitUniform = glGetUniformLocation(program, "texUnit");

  //
  // (9) Tell GL to use our program
  //
  glUseProgram(program);
}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(500,500);
  glutInitWindowPosition(10,10);
  glutCreateWindow("PEPSI");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);  
  glutMotionFunc(mouseMotion);  

  installShaders();

  loadTexture();
  glEnable(GL_TEXTURE_2D);

  setView();

  const GLfloat fovy = 40.0;
  const GLfloat zNear = 0.1, zFar = 60.0;
  matrixIdentity(Projection);
  matrixPerspective(Projection,
                    fovy, 1, zNear, zFar);

  const GLfloat labelHeight = 0.4;
  const GLfloat labelWidth =  1.0;
  const GLfloat labels0 = 0.0;
  const GLfloat labelt0 = 0.58;
  matrixIdentity(TextureMatrix);
  matrixTranslate(TextureMatrix, -labels0, -labelt0, 0);
  matrixScale(TextureMatrix, 1/labelWidth, 1/labelHeight, 1);

  glClearColor(0.0, 0.0, 0.0, 1.0);

  glutMainLoop();
  return 0;
}

