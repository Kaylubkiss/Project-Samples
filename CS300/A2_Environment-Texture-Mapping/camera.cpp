/******************************************************************************/
/*!
\file   camera.cpp
\par    Purpose: Implementation of camera class
\par    Language: C++
\par    Platform: Visual Studio 2013, Windows 7 64-bit
\author Sang Le
\par    Email: sang.le\@digipen.edu
\par    DigiPen login: sang.le
\date   27/3/2015
*/
/******************************************************************************/

#include "camera.hpp"


/*  Pre-defined camera */
Camera mainCam;
Camera mirrorCam(512, 512);     /*  Mirror camera has this size to match the skybox sub-image */


/******************************************************************************/
/*!
\fn     void Camera::Reset()
\brief
        Reset the camera to its original position and orientation.
*/
/******************************************************************************/
void Camera::
Reset()
{
    radius = INIT_RADIUS;
    alpha = INIT_ALPHA;
    beta = INIT_BETA;
    upVec = BASIS[Y];

    moved = true;
}


/******************************************************************************/
/*!
\fn     void Camera::MoveUp()
\brief
        Increase cam alpha by CAM_ANGLE_STEP and rewind if needed, so that
        -NUM_STEPS_HALF_PI <= alpha < NUM_STEPS_ONEANDHALF_PI
*/
/******************************************************************************/
void Camera::
MoveUp()
{
    /*  Move camera up */
    alpha += CAM_ANGLE_STEP;

    /*  Keep alpha within [-90, 270) */
    while (alpha >= NUM_STEPS_ONEANDHALF_PI)
        alpha -= NUM_STEPS_TWO_PI;

    moved = true;
}


/******************************************************************************/
/*!
\fn     void Camera::MoveDown()
\brief
        Decrease cam alpha by CAM_ANGLE_STEP and rewind if needed, so that
        -NUM_STEPS_HALF_PI <= alpha < NUM_STEPS_ONEANDHALF_PI
*/
/******************************************************************************/
void Camera::
MoveDown()
{
    /*  Move camera down */
    alpha -= CAM_ANGLE_STEP;

    /*  Keep alpha within [-90, 270) */
    while (alpha < -NUM_STEPS_HALF_PI)
        alpha += NUM_STEPS_TWO_PI;

    moved = true;
}


/******************************************************************************/
/*!
\fn     void Camera::MoveLeft()
\brief
        Increase cam beta by CAM_ANGLE_STEP and rewind if needed, so that
        0 <= alpha < NUM_STEPS_TWO_PI
*/
/******************************************************************************/
void Camera::
MoveLeft()
{
    /*  Move camera left */
    beta += CAM_ANGLE_STEP;

    /*  Keep beta within [0, 360) */
    while (beta >= NUM_STEPS_TWO_PI)
        beta -= NUM_STEPS_TWO_PI;

    moved = true;
}


/******************************************************************************/
/*!
\fn     void Camera::MoveRight()
\brief
        Decrease cam beta by CAM_ANGLE_STEP and rewind if needed, so that
        0 <= alpha < NUM_STEPS_TWO_PI
*/
/******************************************************************************/
void Camera::
MoveRight()
{
    /*  Move camera right */
    beta -= CAM_ANGLE_STEP;

    /*  Keep beta within [0, 360) */
    while (beta < 0)
        beta += NUM_STEPS_TWO_PI;

    moved = true;
}


/******************************************************************************/
/*!
\fn     void Camera::MoveCloser()
\brief
        Decrease radius by CAM_RADIUS_STEP but cap it at CAM_MIN_RADIUS
*/
/******************************************************************************/
void Camera::
MoveCloser()
{
    /*  Move camera closer to origin */
    if (radius > CAM_MIN_RADIUS)
    {
        radius -= CAM_RADIUS_STEP;
        moved = true;
    }
}


/******************************************************************************/
/*!
\fn     void Camera::MoveFarther()
\brief
        Increase radius by CAM_RADIUS_STEP but cap it at CAM_MAX_RADIUS
*/
/******************************************************************************/
void Camera::
MoveFarther()
{
    /*  Move camera farther from origin */
    if (radius < CAM_MAX_RADIUS)
    {
        radius += CAM_RADIUS_STEP;
        moved = true;
    }
}


/******************************************************************************/
/*!
\fn     void Camera::ComputeUpVec()
\brief
        Compute a reasonable up vector depending on the current alpha and beta,
        so that the camera coordinate frame is constructed correctly.
*/
/******************************************************************************/
void Camera::
ComputeUpVec()
{
    if (alpha == NUM_STEPS_HALF_PI)
        upVec = Mat3(Rotate(-ONE_STEP * beta, BASIS[Y])) * -BASIS[X];
    else
    if (alpha == -NUM_STEPS_HALF_PI)
        upVec = Mat3(Rotate(-ONE_STEP * beta, BASIS[Y])) * BASIS[X];
    else
    if (alpha < NUM_STEPS_HALF_PI)
        upVec = BASIS[Y];
    else
        upVec = -BASIS[Y];
}


/******************************************************************************/
/*!
\fn     Mat4 Camera::ViewMat()
\brief
        Compute view transformation matrix for a camera that looks at the origin,
        based on the camera radius (distance to the origin), alpha (angle between
        origin-camera vector and xz-plane), and beta (angle between origin-camera
        vector and xy-plane).
\return
        The resulting view transformation matrix
*/
/******************************************************************************/
Mat4 Camera::
ViewMat()
{
    ComputeUpVec();
    
    float alphaAng = ONE_STEP * alpha;
    float betaAng = ONE_STEP * beta;

    float ca = std::cosf(alphaAng);
    float sa = std::sinf(alphaAng);
    float cb = std::cosf(betaAng);
    float sb = std::sinf(betaAng);

    float eyeProjXZ = radius * ca;
    pos = lookAt + Vec3(eyeProjXZ * cb, radius * sa, eyeProjXZ * sb);

    return LookAt(pos, lookAt, upVec);
}


/******************************************************************************/
/*!
\fn     Mat4 Camera::ProjMat() const
\brief
        Wrapper for glm::Frustum function.
\return
        The resulting perspective transformation matrix
*/
/******************************************************************************/
Mat4 Camera::
ProjMat() const
{
    return Frustum(leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);
}