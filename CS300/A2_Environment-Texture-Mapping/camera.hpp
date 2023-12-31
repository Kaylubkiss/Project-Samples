/******************************************************************************/
/*!
\file   camera.hpp
\par    Purpose: Definition of camera class
\par    Language: C++
\par    Platform: Visual Studio 2013, Windows 7 64-bit
\author Sang Le
\par    Email: sang.le\@digipen.edu
\par    DigiPen login: sang.le
\date   27/3/2015
*/
/******************************************************************************/

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "math.hpp"


/*  The angles and radius in this file are defined as integers to avoid accumulated imprecision */

/*  The steps for rotation */
const int   NUM_STEPS_PI            = 36;
const int   NUM_STEPS_TWO_PI        = NUM_STEPS_PI * 2;
const int   NUM_STEPS_HALF_PI       = NUM_STEPS_PI / 2;
const int   NUM_STEPS_QUARTER_PI    = NUM_STEPS_PI / 4;
const int   NUM_STEPS_ONEANDHALF_PI = NUM_STEPS_HALF_PI * 3;
const float ONE_STEP                = PI / NUM_STEPS_PI;

/*  Step for camera angle change */
const int CAM_ANGLE_STEP = 1;

/*  Limit for camera distance from the origin */
const int CAM_MAX_RADIUS    = 8;
const int CAM_MIN_RADIUS    = 4;
const int CAM_RADIUS_STEP   = 1;

const int   INIT_ALPHA = 6;
const int   INIT_BETA = 18;     /*  90 degrees */

const int   INIT_RADIUS = 8;


class Camera
{
    public:
        int     width, height;      /*  Viewport width & height */
        bool    resized;            /*  to trigger projection matrix update */
        float   nearPlane, farPlane, topPlane, bottomPlane, rightPlane, leftPlane;

        int     radius;
        int     alpha;
        int     beta;
        Vec3    upVec;
        bool    moved;              /*  to trigger view matrix update */

        Vec3    pos, lookAt;

        /*  Default parameters construct a camera 
            lying along the z-axis at CAM_MAX_RADIUS from origin.
        */
        Camera():
            width(800), height(800),
            resized(true),
            nearPlane(0.1f * CAM_RADIUS_STEP), farPlane(5.0f * CAM_MAX_RADIUS),
            topPlane(nearPlane / 900.0f * height), bottomPlane(-topPlane),
            rightPlane(topPlane * width / height),  leftPlane(-rightPlane),
            radius(INIT_RADIUS), alpha(INIT_ALPHA), beta(INIT_BETA),
            upVec(BASIS[Y]), lookAt(Vec3(0, 0, 0)), moved(true) { }

        Camera(int width, int height):
            width(width), height(height),
            resized(true), moved(true) {}

        /*  Functions to update camera position upon user input */
        void Reset();
        void MoveUp();
        void MoveDown();
        void MoveLeft();
        void MoveRight();
        void MoveCloser();
        void MoveFarther();

        void ComputeUpVec();
        Mat4 ViewMat();
        Mat4 ProjMat() const;
};


/*  Pre-defined camera */
extern Camera mainCam;
extern Camera mirrorCam;

#endif
