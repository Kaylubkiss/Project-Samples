/******************************************************************************/
/*!
\file   input.cpp
\par    Purpose: Handle events upon user inputs
\par    Language: C++
\par    Platform: Visual Studio 2013, Windows 7 64-bit
\author Sang Le
\par    Email: sang.le\@digipen.edu
\par    DigiPen login: sang.le
\date   8/2/2015
*/
/******************************************************************************/

#include "input.hpp"


/******************************************************************************/
/*!
\fn     void Keyboard(unsigned char key, int x, int y)
\brief
        Events when a key is pressed.
\param  key
        Pressed key.
\param  x
        Current x-position of the mouse cursor.
\param  y
        Current y-position of the mouse cursor (from top).
*/
/******************************************************************************/
void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27:    /*  ESC */
            CleanUp();

        case 'w':
        case 'W':
            mainCam.MoveUp();
            break;

        case 's':
        case 'S':
            mainCam.MoveDown();
            break;

        case 'a':
        case 'A':
            mainCam.MoveLeft();
            break;

        case 'd':
        case 'D':
            mainCam.MoveRight();
            break;

        case 'z':
        case 'Z':
            mainCam.MoveCloser();
            break;

        case 'x':
        case 'X':
            mainCam.MoveFarther();
            break;

        case ' ':
            mainCam.Reset();
            break;
    }

    glutPostRedisplay();    /*  Set flag to force re-rendering */
}

/******************************************************************************/
/*!
\fn     void SpecialKeyboard(int key, int x, int y)
\brief
        Events when a special key is pressed. This include CTRL+ ..., ALT+ ...
\param  key
        Pressed key.
\param  x
        Current x-position of the mouse cursor.
\param  y
        Current y-position of the mouse cursor (from top).
*/
/******************************************************************************/
void SpecialKeyboard(int key, int x, int y)
{
    /* ALT+F4: Quit */
    if (key == GLUT_KEY_F4 && glutGetModifiers() == GLUT_ACTIVE_ALT)
    {
        CleanUp();
    }
    else
    /*  For now we're not handling any other special keys */
    if (key == GLUT_KEY_UP || key == GLUT_KEY_DOWN || key == GLUT_KEY_LEFT || key == GLUT_KEY_RIGHT)
    {
    }
}


/******************************************************************************/
/*!
\fn     void MouseClick(int button, int state, int x, int y)
\brief
        Events when a mouse button is clicked. 
        These include AntTweakBar behaviors too.
\param  button
        Clicked button.
\param  state
        Up/down state of the button.
\param  x
        Current x-position of the mouse cursor.
\param  y
        Current y-position of the mouse cursor (from top).
*/
/******************************************************************************/
void MouseClick(int button, int state, int x, int y)
{
    /*  If the mouse is clicked when it is over the AntTweakBar */
    if (TwEventMouseButtonGLUT(button, state, x, y))
    {

        glutPostRedisplay();
    }
    /*  Otherwise don't do anything for now */
}