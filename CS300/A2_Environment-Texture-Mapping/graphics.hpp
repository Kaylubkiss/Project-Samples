/******************************************************************************/
/*!
\file   graphics.hpp
\par    Purpose: Declaring the graphics-related variables & functions
\par    Language: C++
\par    Platform: Visual Studio 2013, Windows 7 64-bit
\author Sang Le
\par    Email: sang.le\@digipen.edu
\par    DigiPen login: sang.le
\date   31/3/2015
*/
/******************************************************************************/

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP


#include "gl/gl3w.h"
#include "gl/freeglut.h"
#include "AntTweakBar.h"


#include "mesh.hpp"
#include "object.hpp"
#include "camera.hpp"

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "freeglut.lib")
#pragma comment (lib, "AntTweakBar.lib")


/*  For displaying frame-per-second */
extern float fps;


/*  For toggling the reflection/refraction of the sphere */
struct RefType
{
    enum { REFLECTION_ONLY = 0, REFRACTION_ONLY, REFLECTION_REFRACTION, NUM_REFTYPES };
};

extern int sphereRef;


/*  For toggling parallax mapping */
extern bool parallaxMappingOn;


/*  Public functions */
void SetUp();
void CleanUp();
void Resize(int w, int h);
void Render();

#endif