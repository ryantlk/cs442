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

void matrixSet4x4(GLfloat M[4*4],
                  float m00, float m01, float m02, float m03,
                  float m10, float m11, float m12, float m13,
                  float m20, float m21, float m22, float m23,
                  float m30, float m31, float m32, float m33) {
    M[I(0,0)]=m00; M[I(0,1)]=m01; M[I(0,2)]=m02; M[I(0,3)]=m03; 
    M[I(1,0)]=m10; M[I(1,1)]=m11; M[I(1,2)]=m12; M[I(1,3)]=m13; 
    M[I(2,0)]=m20; M[I(2,1)]=m21; M[I(2,2)]=m22; M[I(2,3)]=m23; 
    M[I(3,0)]=m30; M[I(3,1)]=m31; M[I(3,2)]=m32; M[I(3,3)]=m33; 
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

#define DOT(U,V) (U[0]*V[0] + U[1]*V[1] + U[2]*V[2])
#define SCALE(s,V) (V[0] *= s, V[1] *= s, V[2] *= s)
#define CROSS(U,V,UxV)\
  (UxV[0] = U[1]*V[2] - U[2]*V[1],\
   UxV[1] = U[2]*V[0] - U[0]*V[2],\
   UxV[2] = U[0]*V[1] - U[1]*V[0])


static void norm3(GLfloat V[3]) {
  const GLfloat s = 1.0/sqrtf(DOT(V,V));
  SCALE(s,V);
}

//
// M <-- M * A
//
void matrixCat(GLfloat M[4*4], const GLfloat A[4*4]) {
  GLfloat T[4*4];
  matrixMultiply(T,M,A);
  matrixCopy(M, T);
}

void matrixLookat(GLfloat M[4*4],
                  float eyex, float eyey, float eyez,
                  float centerx, float centery, float centerz,
                  float upx, float upy, float upz) {
  GLfloat F[3];
  F[0] = centerx - eyex; 
  F[1] = centery - eyey; 
  F[2] = centerz - eyez; 
  norm3(F);
  GLfloat U[3];
  U[0] = upx;
  U[1] = upy;
  U[2] = upz;
  GLfloat S[3];
  CROSS(F,U,S);
  norm3(S);
  CROSS(S,F,U);
  GLfloat R[4*4];
  matrixSet3x3(R,
                S[0],  S[1],  S[2],
                U[0],  U[1],  U[2],
               -F[0], -F[1], -F[2]);
  matrixCat(M, R);
  matrixTranslate(M, -eyex, -eyey, -eyez);
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
  matrixCat(M, S);
}

void matrixPerspective(GLfloat M[4*4],
                       GLfloat fovy_degrees, GLfloat aspect, 
                       GLfloat zNear, GLfloat zFar) {
  const GLfloat fovy = fovy_degrees*(M_PI/180);
  const GLfloat f = 1/tanf(fovy/2);
  const GLfloat s = 1/(zNear - zFar);
  GLfloat P[4*4];
  matrixSet4x4(P,
               f/aspect, 0, 0, 0,
               0, f, 0, 0,
               0, 0, (zFar + zNear)*s, 2*zFar*zNear*s,
               0, 0, -1, 0);
  matrixCat(M, P);
}

void matrixScale(GLfloat M[4*4], GLfloat sx, GLfloat sy, GLfloat sz) {
  GLfloat S[4*4];
  matrixSet3x3(S,
               sx, 0,  0,
               0,  sy, 0,
               0,  0,  sz);
  matrixCat(M, S);
}

void matrixTranslate(GLfloat M[4*4], GLfloat dx, GLfloat dy, GLfloat dz) { 
  GLfloat S[4*4];
  matrixSet3x4(S,
               1, 0, 0, dx,
               0, 1, 0, dy,
               0, 0, 1, dz);
  matrixCat(M, S);
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
  matrixCat(M, S);
}

//
// Computes inverse-transpose of upper 3x3 of M.
//
void matrixNormal(const GLfloat M[4*4], GLfloat normalMatrix[3*3]) {
    const double determinant =    
      +M[I(0,0)]*(M[I(1,1)]*M[I(2,2)] - M[I(2,1)]*M[I(1,2)])
      -M[I(0,1)]*(M[I(1,0)]*M[I(2,2)] - M[I(1,2)]*M[I(2,0)])
      +M[I(0,2)]*(M[I(1,0)]*M[I(2,1)] - M[I(1,1)]*M[I(2,0)]);
    const double invDet = 1.0/determinant;
#define N(row,col) normalMatrix[((col)*3 + (row))]
    N(0,0) =  (M[I(1,1)]*M[I(2,2)] - M[I(2,1)]*M[I(1,2)])*invDet;
    N(1,0) = -(M[I(0,1)]*M[I(2,2)] - M[I(0,2)]*M[I(2,1)])*invDet;
    N(2,0) =  (M[I(0,1)]*M[I(1,2)] - M[I(0,2)]*M[I(1,1)])*invDet;
    N(0,1) = -(M[I(1,0)]*M[I(2,2)] - M[I(1,2)]*M[I(2,0)])*invDet;
    N(1,1) =  (M[I(0,0)]*M[I(2,2)] - M[I(0,2)]*M[I(2,0)])*invDet;
    N(2,1) = -(M[I(0,0)]*M[I(1,2)] - M[I(1,0)]*M[I(0,2)])*invDet;
    N(0,2) =  (M[I(1,0)]*M[I(2,1)] - M[I(2,0)]*M[I(1,1)])*invDet;
    N(1,2) = -(M[I(0,0)]*M[I(2,1)] - M[I(2,0)]*M[I(0,1)])*invDet;
    N(2,2) =  (M[I(0,0)]*M[I(1,1)] - M[I(1,0)]*M[I(0,1)])*invDet;
#undef N
}

void matrixReflect(GLfloat M[4*4], const GLfloat plane[4]) {
  GLfloat R[4*4];
  const float A = plane[0];
  const float B = plane[1];
  const float C = plane[2];
  const float D = plane[3];
  matrixSet3x4(R,
	       1 - 2*A*A,  -2*A*B,  -2*A*C, -2*A*D,
	          -2*B*A, 1-2*B*B,  -2*B*C, -2*B*D,
	          -2*C*A,  -2*C*B, 1-2*C*C, -2*C*D);
  matrixCat(M, R);
}

void matrixShadow(GLfloat M[4*4], 
		  const GLfloat light[4], const GLfloat plane[4]) {
  GLfloat S[4][4];

  GLfloat dot =
    plane[0]*light[0] + plane[1]*light[1] +
    plane[2]*light[2] + plane[3]*light[3];
  
  S[0][0] = dot - light[0]*plane[0];
  S[1][0] =     - light[0]*plane[1];
  S[2][0] =     - light[0]*plane[2];
  S[3][0] =     - light[0]*plane[3];
  
  S[0][1] =     - light[1]*plane[0];
  S[1][1] = dot - light[1]*plane[1];
  S[2][1] =     - light[1]*plane[2];
  S[3][1] =     - light[1]*plane[3];
  
  S[0][2] =     - light[2]*plane[0];
  S[1][2] =     - light[2]*plane[1];
  S[2][2] = dot - light[2]*plane[2];
  S[3][2] =     - light[2]*plane[3];
  
  S[0][3] =     - light[3]*plane[0];
  S[1][3] =     - light[3]*plane[1];
  S[2][3] =     - light[3]*plane[2];
  S[3][3] = dot - light[3]*plane[3];

  matrixCat(M, (GLfloat *) S);
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
