/******************************************************************************/
/*!
\file   math.cpp
\par    Purpose: Implementations of math functions
\par    Language: C++
\par    Platform: Visual Studio 2013, Windows 7 64-bit
\author Sang Le
\par    Email: sang.le\@digipen.edu
\par    DigiPen login: sang.le
\date   28/10/2014
*/
/******************************************************************************/

#include "math.hpp"


/******************************************************************************/
/*  Wrappers for GLM functions                                                */
/******************************************************************************/

/******************************************************************************/
/*!
\fn     Mat4 Scale(float sx, float sy, float sz)
\brief
        Compute scaling matrix
\param  sx
        Scale factor along x
\param  sy
        Scale factor along y
\param  sz
        Scale factor along z
\return
        The resulting scaling matrix
*/
/******************************************************************************/
Mat4 Scale(float sx, float sy, float sz)
{
    return glm::scale(Mat4(1.0f), Vec3(sx, sy, sz));
}


/******************************************************************************/
/*!
\fn     Mat4 Scale(const Vec3 &s)
\brief
        Compute scaling matrix
\param  s
        3-component vector indicating the scale factors along x, y, z
\return
        The resulting scaling matrix
*/
/******************************************************************************/
Mat4 Scale(const Vec3 &s)
{
    return glm::scale(Mat4(1.0f), s);
}


/******************************************************************************/
/*!
\fn     Mat4 Rotate(float angle, const Vec3 &axis)
\brief
        Compute rotation matrix
\param  angle
        Rotation angle
\param  axis
        Rotation axis
\return
        The resulting rotation matrix
*/
/******************************************************************************/
Mat4 Rotate(float angle, const Vec3 &axis)
{
    return glm::rotate(Mat4(1.0f), angle, axis);
}


/******************************************************************************/
/*!
\fn     Mat4 Rotate(float angle, float ax, float ay, float az)
\brief
        Compute rotation matrix
\param  angle
        Rotation angle
\param  ax
        x-component of rotation axis
\param  ay
        y-component of rotation axis
\param  az
        z-component of rotation axis
\return
        The resulting rotation matrix
*/
/******************************************************************************/
Mat4 Rotate(float angle, float ax, float ay, float az)
{
    return glm::rotate(Mat4(1.0f), angle, Vec3(ax, ay, az));
}


/******************************************************************************/
/*!
\fn     Mat4 Translate(float tx, float ty, float tz)
\brief
        Compute translation matrix
\param  tx
        Translation along x
\param  ty
        Translation along y
\param  tz
        Translation along z
\return
        The resulting translation matrix
*/
/******************************************************************************/
Mat4 Translate(float tx, float ty, float tz)
{
    return glm::translate(Mat4(1.0f), Vec3(tx, ty, tz));
}


/******************************************************************************/
/*!
\fn     Mat4 Translate(const Vec3 &t)
\brief
        Compute translation matrix
\param  s
        3-component vector indicating the translation amounts along x, y, z
\return
        The resulting translation matrix
*/
/******************************************************************************/
Mat4 Translate(const Vec3 &t)
{
    return glm::translate(Mat4(1.0f), t);
}


/******************************************************************************/
/*!
\fn     Mat4 Frustum(float leftPlane, float rightPlane, 
                            float bottomPlane, float topPlane, 
                            float nearPlane, float farPlane)
\brief
        Compute perspective projection matrix
\param  leftPlane
        x-coordinate of the intersection between left plane and near plane
\param  rightPlane
        x-coordinate of the intersection between right plane and near plane
\param  bottomPlane
        y-coordinate of the intersection between bottom plane and near plane
\param  topPlane
        y-coordinate of the intersection between top plane and near plane
\param  nearPlane
        z-coordinate of near plane
\param  farPlane
        z-coordinate of far plane
\return
        The resulting perspective projection matrix
*/
/******************************************************************************/
Mat4 Frustum(   float leftPlane, float rightPlane, 
                float bottomPlane, float topPlane, 
                float nearPlane, float farPlane)
{
    return glm::frustum(leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);
}


/******************************************************************************/
/*!
\fn     Mat4 Perspective(float fovy, float aspect, float near, float far)
\brief
        Compute perspective projection matrix
\param  fovy
        Field-of-view y full angle (not half angle)
\param  aspect
        Width-to-height aspect ratio
\param  near
        Positive distance from camera to near plane
\param  far
        Positive distance from camera to far plane
\return
        The resulting perspective projection matrix
*/
/******************************************************************************/
Mat4 Perspective(float fovy, float aspect, float near, float far)
{
    return glm::perspective(fovy, aspect, near, far);
}


/******************************************************************************/
/*!
\fn     Mat4 LookAt(const Vec3 &eye, const Vec3 &center, const Vec3 &up)
\brief
        Compute view transformation matrix
\param  eye
        Camera position
\param  center
        Look-at position
\param  up
        Up vector
\return
        The resulting view transformation matrix
*/
/******************************************************************************/
Mat4 LookAt(const Vec3 &eye, const Vec3 &center, const Vec3 &up)
{
    return glm::lookAt(eye, center, up);
}


/******************************************************************************/
/*!
\fn     Vec3 Normalize(Vec3 v)
\brief
        Normalize a vector
\param  v
        The input vector
\return
        The resulting normalized vector
*/
/******************************************************************************/
Vec3 Normalize(const Vec3 &v)
{
    return glm::normalize(v);
}


/******************************************************************************/
/*!
\fn     Vec3 Cross(const Vec3 &v1, const Vec3 &v2)
\brief
        Cross product of two 3-component vectors
\param  v1
        The 1st vector
\param  v2
        The 2nd vector
\return
        The resulting cross product
*/
/******************************************************************************/
Vec3 Cross(const Vec3 &v1, const Vec3 &v2)
{
    return glm::cross(v1, v2);
}


/******************************************************************************/
/*!
\fn     float Dot(const Vec3 &v1, const Vec3 &v2)
\brief
        Dot product of two 3-component vectors
\param  v1
        The 1st vector
\param  v2
        The 2nd vector
\return
        The resulting dot product
*/
/******************************************************************************/
float Dot(const Vec3 &v1, const Vec3 &v2)
{
    return glm::dot(v1, v2);
}


/******************************************************************************/
/*!
\fn     float Distance(const Vec3 &v1, const Vec3 &v2)
\brief
        Distance between two 3D points.
\param  v1
        Position vector of the 1st point
\param  v2
        Position vector of the 2nd point
\return
        The resulting distance
*/
/******************************************************************************/
float Distance(const Vec3 &v1, const Vec3 &v2)
{
    return glm::distance(v1, v2);
}


/******************************************************************************/
/*!
\fn     Mat4 Inverse(const Mat4 &m)
\brief
        Find inverse of a matrix
\param  m
        The input matrix
\return
        The resulting inverse matrix
*/
/******************************************************************************/
Mat4 Inverse(const Mat4 &m)
{
    return glm::inverse(m);
}


/******************************************************************************/
/*!
\fn     Mat4 Transpose(const Mat4 &m)
\brief
        Find transpose of a matrix
\param  m
        The input matrix
\return
        The resulting transpose matrix
*/
/******************************************************************************/
Mat4 Transpose(const Mat4 &m)
{
    return glm::transpose(m);
}


/******************************************************************************/
/*!
\fn     void PrintMat(const Mat4 &m)
\brief
        Print out a matrix so that translation appears in the last column
\param  m
        The input matrix
*/
/******************************************************************************/
void PrintMat(const Mat4 &m)
{
    std::cout << std::fixed;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
            std::cout << std::setw(8) << std::setprecision(2) << m[j][i] << " ";
        std::cout << "\n";
    }
    std::cout << "\n";
}


/******************************************************************************/
/*!
\fn     void PrintVec(const Vec3 &v)
\brief
        Print out a 3-component vector
\param  m
        The input vector
*/
/******************************************************************************/
void PrintVec(const Vec3 &v)
{
    std::cout << std::fixed;
    for (int i = 0; i < 3; ++i)
        std::cout << std::setw(8) << std::setprecision(2) << v[i] << " ";
    std::cout << "\n";
}


/******************************************************************************/
/*!
\fn     void PrintVec(const Vec4 &v)
\brief
        Print out a 4-component vector
\param  m
        The input vector
*/
/******************************************************************************/
void PrintVec(const Vec4 &v)
{
    std::cout << std::fixed;
    for (int i = 0; i < 4; ++i)
        std::cout << std::setw(8) << std::setprecision(2) << v[i] << " ";
    std::cout << "\n";
}


/******************************************************************************/
/*!
\fn     float RoundDecimal(float input)
\brief
        Round a value to 4-decimal places
\param  input
        The given value
\return
        The rounded value
*/
/******************************************************************************/
float RoundDecimal(float input)
{
    return std::floor(input * 10000.0f + 0.5f) / 10000.0f;
}


/******************************************************************************/
/*!
\fn     Vec3 RoundDecimal(const Vec3 &input)
\brief
        Round the components of a vector3 to 4-decimal places
\param  input
        The given vector
\return
        The vector with rounded values
*/
/******************************************************************************/
Vec3 RoundDecimal(const Vec3 &input)
{
    return Vec3(RoundDecimal(input[0]), RoundDecimal(input[1]), RoundDecimal(input[2]));
}


/******************************************************************************/
/*!
\fn     Vec4 RoundDecimal(const Vec4 &input)
\brief
        Round the components of a vector4 to 4-decimal places
\param  input
        The given vector
\return
        The vector with rounded values
*/
/******************************************************************************/
Vec4 RoundDecimal(const Vec4 &input)
{
    return Vec4(RoundDecimal(input[0]), RoundDecimal(input[1]), RoundDecimal(input[2]), RoundDecimal(input[3]));
}


/******************************************************************************/
/*!
\fn     bool DegenerateTri(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2)
\brief
        Check if a triangle is degenerate, that is if the length between any
        two vertices is close to 0.
\param  v0
        The 1st vertex
\param  v1
        The 2nd vertex
\param  v2
        The 3rd vertex
\return
        Whether the triangle is degenerate
*/
/******************************************************************************/
bool DegenerateTri(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2)
{
    return (Distance(v0, v1) < EPSILON ||
            Distance(v1, v2) < EPSILON ||
            Distance(v2, v0) < EPSILON);
}
