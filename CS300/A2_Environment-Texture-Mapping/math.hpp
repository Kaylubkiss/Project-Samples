/******************************************************************************/
/*!
\file   math.hpp
\par    Purpose: Placeholder for math constants/functions
\par    Language: C++
\par    Platform: Visual Studio 2013, Windows 7 64-bit
\author Sang Le
\par    Email: sang.le\@digipen.edu
\par    DigiPen login: sang.le
\date   28/10/2014
*/
/******************************************************************************/

#ifndef MATH_HPP
#define MATH_HPP

#include <iostream>
#include <iomanip>
#include <cmath>

#define GLM_FORCE_RADIANS

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"


using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;


/*  Useful constants */
const float  PI             = 3.14159f;
const float  TWO_PI         = 2.0f * PI;
const float  HALF_PI        = 0.5f * PI;
const float  QUARTER_PI     = 0.25f * PI;
const float  EIGHTH_PI      = 0.125f * PI;
const float  FIFTHTEENTH_PI = 0.0667f * PI;
const float  SIXTEENTH_PI   = 0.0625f * PI;

const float  DEG_TO_RAD = PI / 180.0f;
const float  RAD_TO_DEG = 180.0f / PI;

const float  EPSILON = 0.00001f;

const int X = 0;
const int Y = 1;
const int Z = 2;

const Vec3 BASIS[3]{    Vec3(1.0f, 0.0f, 0.0f),
                        Vec3(0.0f, 1.0f, 0.0f),
                        Vec3(0.0f, 0.0f, 1.0f)
                   };


/******************************************************************************/
/*  Utility functions                                                         */
/******************************************************************************/
float RoundDecimal(float input);
Vec3 RoundDecimal(const Vec3 &input);
Vec4 RoundDecimal(const Vec4 &input);
bool DegenerateTri(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2);


/******************************************************************************/
/*  Wrappers for GLM functions                                                */
/******************************************************************************/

Mat4 Scale(float sx, float sy, float sz);
Mat4 Scale(const Vec3 &s);

Mat4 Rotate(float angle, const Vec3 &axis);
Mat4 Rotate(float angle, float ax, float ay, float az);

Mat4 Translate(float tx, float ty, float tz);
Mat4 Translate(const Vec3 &t);

Mat4 Frustum(float leftPlane, float rightPlane, 
             float bottomPlane, float topPlane, 
             float nearPlane, float farPlane);
Mat4 Perspective(float fovy, float aspect, float near, float far);


Mat4 LookAt(const Vec3 &eye, const Vec3 &center, const Vec3 &up);

Mat4 Inverse(const Mat4 &m);
Mat4 Transpose(const Mat4 &m);

Vec3 Normalize(const Vec3 &v);
Vec3 Cross(const Vec3 &v1, const Vec3 &v2);
float Dot(const Vec3 &v1, const Vec3 &v2);
float Distance(const Vec3 &v1, const Vec3 &v2);

template <typename T>
float *ValuePtr(T value) {  return glm::value_ptr(value);   }

void PrintMat(const Mat4 &m);   /*  for debugging matrices */
void PrintVec(const Vec3 &v);   /*  for debugging vectors */
void PrintVec(const Vec4 &v);   /*  for debugging vectors */

#endif