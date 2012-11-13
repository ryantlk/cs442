#ifndef MATRIX_H
#define MATRIX_H

#define GL_GLEXT_PROTOTYPES

#if defined(__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

void matrixIdentity(GLfloat M[4*4]);
void matrixCat(GLfloat M[4*4], const GLfloat A[4*4]);
void matrixScale(GLfloat M[4*4], GLfloat sx, GLfloat sy, GLfloat sz);
void matrixTranslate(GLfloat M[4*4], GLfloat dx, GLfloat dy, GLfloat dz);
void matrixRotate(GLfloat M[4*4],
                  GLfloat angle_degrees, GLfloat x, GLfloat y, GLfloat z);

void matrixLookat(GLfloat M[4*4],
                  float eyex, float eyey, float eyez,
                  float centerx, float centery, float centerz,
                  float upx, float upy, float upz);

void matrixOrtho(GLfloat M[4*4],
                 GLfloat left, GLfloat right,
                 GLfloat bottom, GLfloat top,
                 GLfloat hither, GLfloat yon);
void matrixPerspective(GLfloat M[4*4],
                       GLfloat fovy_degrees, GLfloat aspect, 
                       GLfloat zNear, GLfloat zFar);

void matrixNormal(const GLfloat M[4*4], GLfloat normalMatrix[3*3]);

void matrixPush(GLfloat M[4*4]);
void matrixPop(GLfloat M[4*4]);

void matrixCopy(GLfloat M[4*4], const GLfloat A[4*4]);
void matrixSet3x4(GLfloat M[4*4],
                  float m00, float m01, float m02, float m03,
                  float m10, float m11, float m12, float m13,
                  float m20, float m21, float m22, float m23);
void matrixSet3x3(GLfloat M[4*4],
                  float m00, float m01, float m02,
                  float m10, float m11, float m12,
                  float m20, float m21, float m22);
void matrixMultiply(GLfloat AB[4*4], const GLfloat A[4*4], const GLfloat B[4*4]);

#endif /* MATRIX_H */
