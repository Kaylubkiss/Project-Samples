/******************************************************************************/
/*!
\file   object.cpp
\par    Purpose: Definitions for scene objects
\par    Language: C++
\par    Platform: Visual Studio 2013, Windows 7 64-bit
\author Sang Le
\par    Email: sang.le\@digipen.edu
\par    DigiPen login: sang.le
\date   27/3/2015
*/
/******************************************************************************/

#include "object.hpp"

/*  Pre-defined meshes */
Mesh mesh[MeshID::NUM_MESHES] =
{
    CreateCube(1, 1, 1),
    LoadOBJMesh("../models/vase.obj"),
    CreatePlane(1, 1),
    CreateSphere(16, 16)
};


/*  Pre-defined objects */
std::vector<Object> obj;

/*  Mirror and sphere positions, which are used in graphics.cpp for rendering scene from these objects */
Vec3 mirrorTranslate;
Vec3 spherePos;


/*  Light pos are defined in world frame, but we need to compute their pos in view frame for
    lighting. In this frame, the vertex positions are not too large, hence the computation
    is normally more accurate.
*/
Vec3 lightPosWF[NUM_LIGHTS], lightPosVF[NUM_LIGHTS];


/*  Setting up the light position */
void SetUpLight(float height);


/******************************************************************************/
/*!
\fn     void SetUpScene()
\brief
        Set up all objects in the scene.
*/
/******************************************************************************/
void SetUpScene()
{
    Vec3 baseSize = Vec3(7.0f, 0.5f, 7.0f);
    Object base(MeshID::CUBE, ImageID::STONE_TEX, Rotate(FIFTHTEENTH_PI, BASIS[Y]) * Scale(baseSize));
    obj.push_back(base);
    

    Object vase(MeshID::VASE, ImageID::POTTERY_TEX, Translate(1.0f, 0.3f, 2.0f));
    obj.push_back(vase);


    mirrorTranslate = Vec3(1.0f, 2.0f, 0.0f);
    /*  The 180-degree rotation about the y-axis is to simulate the mirroring effect.
        Also because of this the mirror normal vectors are pointing backwards and we
        have to switch the culled face to GL_BACK in graphics.cpp
    */
    Object mirror(MeshID::PLANE, ImageID::MIRROR_TEX, Translate(mirrorTranslate) * Scale(2.0f, 2.0f, 2.0f) * Rotate(PI, BASIS[Y]));
    obj.push_back(mirror);


    /*  These 3 parts are for the base of the mirror */
    Object mirrorBase1(MeshID::CUBE, ImageID::WOOD_TEX, Translate(mirrorTranslate + Vec3(0, 0, -0.03f)) * Scale(2.2f, 2.2f, 0.05f));
    obj.push_back(mirrorBase1);

    Object mirrorBase2(MeshID::CUBE, ImageID::WOOD_TEX, Translate(mirrorTranslate + Vec3(0, -1.75, -0.6f)) * Scale(2.0f, 0.1f, 1.0f));
    obj.push_back(mirrorBase2);

    Object mirrorBase3(MeshID::CUBE, ImageID::WOOD_TEX, Translate(mirrorTranslate + Vec3(0, -0.85, -0.53f)) * Rotate(TWO_PI / 3, BASIS[X]) * Scale(0.5f, 0.05f, 2.0f));
    obj.push_back(mirrorBase3);
    

    spherePos = Vec3(-1.0f, 0.75f, -1.0f);
    Object sphere(MeshID::SPHERE, ImageID::SPHERE_TEX, Translate(spherePos));
    obj.push_back(sphere);


    SetUpLight(baseSize.x);
}


/******************************************************************************/
/*!
\fn     void SetUpLight(float height)
\brief
        Set up the light at (0, height, 0).
*/
/******************************************************************************/
void SetUpLight(float height)
{
    lightPosWF[0] = Vec3(0, height, 0);
}