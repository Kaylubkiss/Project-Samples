/******************************************************************************/
/*!
@file   main.cpp
@par    Purpose: Main file of the program
@par    Language: C++
@par    Platform: Visual Studio 2013, Windows 7 64-bit
@author Sang Le
@par    Email: sang.le\@digipen.edu
@par    DigiPen login: sang.le
@date   27/3/2015
*/
/******************************************************************************/

#include <iostream>
#include <iomanip>

#include "input.hpp"
#include "graphics.hpp"


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


/******************************************************************************/
/*!
@fn     void CreateGLContext(int argc, char **argv)
@brief
        Create the context for OpenGL commands to work.
@param  argc
        argc from main function
@param  argv
        argv from main function
*/
/******************************************************************************/
void CreateGLContext(int argc, char **argv)
{
    /* GLUT Window Creation & Initialization */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(mainCam.width, mainCam.height);
    glutInitWindowPosition(100, 0);
    glutCreateWindow("Room");

    glutInitContextVersion(3, 3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
}


/******************************************************************************/
/*!
@fn     void InitGLExtensions()
@brief
        Initialize the OpenGL extensions supported on the current platform.
*/
/******************************************************************************/
void InitGLExtensions()
{
    /* GL3W Initialization */
    if (gl3wInit())
    {
        std::cerr << "Failed to initialize OpenGL.\n";
        exit(1);
    }

    if (!gl3wIsSupported(3, 3))
    {
        std::cerr << "OpenGL 3.3 not supported.\n";
        exit(1);
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL version  : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
}


/******************************************************************************/
/*!
@fn     void CreateAntTweakBar()
@brief
        Create the AntTweakBar to display information and receive user inputs.
*/
/******************************************************************************/
void CreateAntTweakBar()
{
    TwInit(TW_OPENGL_CORE, NULL);

    TwBar *myBar;
    myBar = TwNewBar("Parameters");
    TwDefine("Parameters color='150 150 150' alpha=85 fontSize=3 position='10 10' size='300 100' valuesWidth=150 ");

    /*  Displaying FPS */
    TwAddVarRO(myBar, "Frame Rate", TW_TYPE_FLOAT, &fps, "");
    

    /*  Sphere reflection & refraction */
    TwEnumVal refType[3] =  {   { RefType::REFLECTION_ONLY, "Reflection Only" }, 
                                { RefType::REFRACTION_ONLY, "Refraction Only" }, 
                                { RefType::REFLECTION_REFRACTION, "Reflection & Refraction" }
                            };
    TwType sphereRefType = TwDefineEnum("Ball Ref Type", refType, RefType::NUM_REFTYPES);
    TwAddVarRW(myBar, "Sphere", sphereRefType, &sphereRef, "");


    /*  Parallax mapping toggling */
    TwAddVarRW(myBar, "Parallax Mapping", TW_TYPE_BOOLCPP, &parallaxMappingOn, "");
}


/******************************************************************************/
/*!
@fn     void MainLoop()
@brief
        The main loop of OpenGL rendering and user interactions.
*/
/******************************************************************************/
void MainLoop()
{
    /* GLUT callback loop */
    glutDisplayFunc(Render);
    glutIdleFunc(Render);

    glutReshapeFunc(Resize);

    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeyboard);

    glutMouseFunc(MouseClick);

    glutMainLoop();
}


/******************************************************************************/
/*!
@fn     main
@brief
        Entry point of the program.
@param  argc
        Number of arguments when the program is executed.
@param  argv
        The array of arguments when the program is executed.
*/
/******************************************************************************/
void main(int argc, char **argv)
{
    /*  This function can be used in Windows to disable the console.
        Currently, we still keep our console for debugging purpose. */
    //FreeConsole();

    CreateGLContext(argc, argv);
    InitGLExtensions();

    SetUp();

    CreateAntTweakBar();

    /*  The main loop for user drawing and interactions */
    MainLoop();
}