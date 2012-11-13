#include <stdio.h>
#include <math.h>
#include <string.h>
#include "matrix.h"
#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif


//
// Column-Major indexing of 4x4 matrix
//
#define I(row,col) ((col)*4 + (row))

void matrixCopy(GLfloat M[4*4], const GLfloat A[4*4]) {
  for (int i = 0; i < 4*4; i++)
    M[i] = A[i];
}

void matrixSet3x4(GLfloat M[4*4],
                  float m00, float m01, float m02, float m03,
                  float m10, float m11, float m12, float m13,
                  float m20, float m21, float m22, float m23) {
    M[I(0,0)]=m00; M[I(0,1)]=m01; M[I(0,2)]=m02; M[I(0,3)]=m03; 
    M[I(1,0)]=m10; M[I(1,1)]=m11; M[I(1,2)]=m12; M[I(1,3)]=m13; 
    M[I(2,0)]=m20; M[I(2,1)]=m21; M[I(2,2)]=m22; M[I(2,3)]=m23; 
    M[I(3,0)]=0;   M[I(3,1)]=0 ;  M[I(3,2)]=0;   M[I(3,3)]=1; 
}

void matrixSet3x3(GLfloat M[4*4],
                  float m00, float m01, float m02,
                  float m10, float m11, float m12,
                  float m20, float m21, float m22) {
    M[I(0,0)]=m00; M[I(0,1)]=m01; M[I(0,2)]=m02; M[I(0,3)]=0; 
    M[I(1,0)]=m10; M[I(1,1)]=m11; M[I(1,2)]=m12; M[I(1,3)]=0; 
    M[I(2,0)]=m20; M[I(2,1)]=m21; M[I(2,2)]=m22; M[I(2,3)]=0; 
    M[I(3,0)]=0;   M[I(3,1)]=0 ;  M[I(3,2)]=0;   M[I(3,3)]=1; 
}

void matrixIdentity(GLfloat M[4*4]) {
  for (int c = 0; c < 4; c++)
    for (int r = 0; r < 4; r++)
      M[I(r,c)] = (r == c) ? 1 : 0;
}

void matrixMultiply(GLfloat AB[4*4], const GLfloat A[4*4], const GLfloat B[4*4]) {
  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 4; c++) {
      GLfloat s = 0;
      for (int i = 0; i < 4; i++)
        s += A[I(r,i)]*B[I(i,c)];
      AB[I(r,c)] = s;
    }
}

void matrixOrtho(GLfloat M[4*4],
                 GLfloat left, GLfloat right,
                 GLfloat bottom, GLfloat top,
                 GLfloat hither, GLfloat yon) {
  GLfloat S[4*4];
  matrixSet3x4(S,
               2/(right - left), 0,  0, -(right + left)/(right - left),
               0, 2/(top - bottom), 0, -(top + bottom)/(top - bottom),
               0, 0, -2/(yon-hither), -(yon + hither)/(yon - hither));
  GLfloat T[4*4];
  matrixMultiply(T,M,S);
  matrixCopy(M, T);
}


void matrixScale(GLfloat M[4*4], GLfloat sx, GLfloat sy, GLfloat sz) {
  GLfloat S[4*4];
  matrixSet3x3(S,
               sx, 0,  0,
               0,  sy, 0,
               0,  0,  sz);
  GLfloat T[4*4];
  matrixMultiply(T,M,S);
  matrixCopy(M, T);
}

void matrixTranslate(GLfloat M[4*4], GLfloat dx, GLfloat dy, GLfloat dz) { 
  GLfloat S[4*4];
  matrixSet3x4(S,
               1, 0, 0, dx,
               0, 1, 0, dy,
               0, 0, 1, dz);
  GLfloat T[4*4];
  matrixMultiply(T,M,S);
  matrixCopy(M, T);
}

void matrixRotate(GLfloat M[4*4],
                  GLfloat angle_degrees, GLfloat x, GLfloat y, GLfloat z) {
  const GLfloat p = 1/sqrtf(x*x + y*y + z*z);
  x *= p; y *= p; z *= p;
  const float angle = angle_degrees * (M_PI/180);
  const float c = cosf(angle);
  const float s = sinf(angle);
  const float c_ = 1 - c;
  const float zc_ = z*c_;
  const float yc_ = y*c_;
  const float xzc_ = x*zc_;
  const float xyc_ = x*y*c_;
  const float yzc_ = y*zc_;
  const float xs = x*s;
  const float ys = y*s;
  const float zs = z*s;
  GLfloat S[4*4];
  matrixSet3x3(S, 
               x*x*c_ + c,  xyc_ - zs,   xzc_ + ys,
               xyc_ + zs,   y*yc_ + c,   yzc_ - xs,
               xzc_ - ys,   yzc_ + xs,   z*zc_ + c);
  GLfloat T[4*4];
  matrixMultiply(T,M,S);
  matrixCopy(M, T);
}

//
// Client matrix stack.
//
#define STACK_MAX 20
static int matrixStackSize = 0;
static GLfloat matrixStack[STACK_MAX][4*4];

void matrixPush(GLfloat M[4*4]) {
  memcpy(matrixStack[matrixStackSize++], M, 4*4*sizeof(GLfloat));
}

void matrixPop(GLfloat M[4*4]) {
  memcpy(M, matrixStack[--matrixStackSize], 4*4*sizeof(GLfloat));
}
