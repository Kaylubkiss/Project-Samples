// -----------------------------------------------------------------------------
// \Project Name Lunar Sword
// \filename     Camera.cpp
// \author       Caleb
// \date         11/3/2022 5:14:22 PM
//\brief         Caleb, 100%
//
// Copyright (c) 2022 DigiPen, All rights reserved.
// -----------------------------------------------------------------------------

#include "Camera.h"
#include "Window.h"
#include "GraphicsUtility.h"
#include "ISystemManager.h"
#include "EntityFactory.h"
#include "EntityManager.h"
#include "SerializationUtility.h"
#include <istreamwrapper.h>
#include "ISystem.h"
#include "GraphicsLibraries.h"
#include "glm/glm.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include "ShaderManager.h"
#include "Interpolation.h"
#define _USE_MATH_DEFINES
#include <math.h>

// other includes

// code
namespace LunarSword
{
    int Camera::num_of_scripted_affects = 0;

    std::vector<CameraAffect*>* Camera::GetCamAffectManager()
    {
        return &cam_affect_manager;
    }

    unsigned Camera::FindCamAffect(CameraAffect* affect)
    {
        auto& manager = cam_affect_manager;

        for (int i = 0; i < manager.size(); ++i)
        {
            if (affect == manager[i])
            {
                return i;
            }
        }

        return -1;

    }

    void Camera::IncremenetNumOfScriptedFX() 
    {
        ++num_of_scripted_affects;
    }

    void Camera::DecremenetNumOfScriptedFX() 
    {
        --num_of_scripted_affects;
    }

    void Camera::QueueCamAffect(CameraAffect* effect)
    {
        //we are assuming that it is already in the vector,
        //although if not 
        auto& manager = cam_affect_manager;

        int index = FindCamAffect(effect);

        if (index == -1)
        {
            AddCamAffect(effect);

            //to avoid searching through the list more, it's already in the back.
            if (!effect->getIsActive())
            {
                return;
            }
        }

        int camera_controlled_index = Camera::getIndexControlledAffect();

        //checking the beginning after disabling effect.
        if (index == 0 && !effect->getIsActive() && 
            (static_cast<unsigned long long>(index) + 1) < manager.size() &&
            !manager[static_cast<unsigned long long>(index) + 1]->getIsActive() &&
            camera_controlled_index < 0)
        {
            return; //we won't move the block if nothing's waiting.
        }

        //if the affect was toggled to false
        if (!effect->getIsActive()) 
        {
            //if there is something next in line, then we gotta move things back -- THIS MIGHT BRING PROBLEMS!
            for (int i = index; i < manager.size() - 1; ++i)
            {
                std::swap(manager[i], manager[static_cast<long long>(i) + 1]);
            }

            //if the index is at the end, the above will not execute.
            camera_controlled_index = Camera::getIndexControlledAffect();

            if (camera_controlled_index > 0) //add another method. THIS MIGHT BE USELESS....
            {
                if (Camera::NumScriptedFX() == 0)
                {
                    std::swap(manager[0], manager[camera_controlled_index]);
                }
            }
          
            return;
        }

        //insert at the front, fully inactive list, or the controlled event needs to be interrupted.
        if (!manager[0]->getIsTimed() || !manager[0]->getIsActive() && !manager[1]->getIsActive() && index != 0) 
        {
            std::swap(manager[index], manager[0]);
            return;
        }
        else if (manager[0]->getIsTimed() && manager[0]->getIsActive() && index == 0) 
        {
            return; //we don't need to queue at this point, since it'll just be index 0
        }


        //if it's set to active then we need to put it to the very back of the queue.
        for (int i = 0; i < manager.size(); ++i)
        {
            int next_index = i + 1;

            if (manager[i]->getIsActive())
            {
                if (next_index != manager.size() &&  //treating vector as a linked list now.
                    (!manager[next_index]->getIsActive() ||
                        manager[i] != effect)) //don't swap the effect if it's already in the back of the queue.
                {
                    std::swap(manager[index], manager[next_index]);
                }
            }
        }

        camera_controlled_index = Camera::getIndexControlledAffect();

        if (camera_controlled_index > 0)
        {
            if (Camera::NumScriptedFX() == 0)
            {
                std::swap(manager[0], manager[camera_controlled_index]);
            }
        }
    }

    void Camera::AddCamAffect(CameraAffect* affect) 
    {
        cam_affect_manager.push_back(affect);
    }

    int Camera::getIndexControlledAffect()
    {
        auto& manager = CamAffectManager;
        for (int i = 0; i < manager.size(); ++i) {

            if (!manager[i]->getIsTimed()) 
            {
                return i;
            }
        }

        return -1;
    }

    const int& Camera::NumScriptedFX() 
    {
        return num_of_scripted_affects;
    }

    Camera::~Camera() 
    {
        TraceMessage("Camera Shutdown");

        for (int loop = 0; loop < (int)cam_affect_manager.size(); ++loop)
        {
            if (cam_affect_manager[loop] != NULL)
            {
                delete cam_affect_manager[loop];
                cam_affect_manager[loop] = NULL;
            }
        }

        cam_affect_manager.clear();
        cam_affect_manager.~vector();

      
    }


    Camera::Camera() : cam_effect(NULL)
    {
        TraceMessage("Camera Init");
        right_vector = { 1, 0, 0, 0 };
        up_vector = { 0, 1, 0, 0 };
        rect_width = 2;
        rect_height = 2;
        center_point = { 0, 0, 0, 1 };
        cam_effect = new CameraAffect("Camera");
        cam_affect_manager.push_back(cam_effect);
      
    }

    void Camera::Deserialize(rapidjson::Value const& jsonObj)
    {
        UNREFERENCED_PARAMETER(jsonObj);
    }

    void Camera::YShake(float dt)
    {

      auto& affect = CamAffectManager[0];
    
      float y_pushf = 0.0f;

      affect->accumulated_time = Clamp(affect->accumulated_time, 0.0f, affect->reset_time);
       
      y_pushf = -CamAffectManager[0]->shake_kick * 
          expf(-affect->reset_time * affect->accumulated_time) * 
          sinf(affect->shake_speed * affect->accumulated_time) - GetSystem(Camera)->getCenter().y;

      affect->accumulated_time += dt;

      GetSystem(Camera)->moveUp(&y_pushf);

   
    }

    void Camera::XShake(float dt)
    {
        auto& affect = CamAffectManager[0];

        float x_pushf = 0.0f;

        affect->accumulated_time = Clamp(affect->accumulated_time, 0.0f, affect->reset_time);

        x_pushf = -CamAffectManager[0]->shake_kick *
            expf(-affect->reset_time * affect->accumulated_time) * 
            sinf(affect->shake_speed * affect->accumulated_time) - GetSystem(Camera)->getCenter().x;

        affect->accumulated_time += dt;

        GetSystem(Camera)->moveRight(&x_pushf);


    }

    void Camera::RotationalShake(float dt) 
    {
        auto& affect = CamAffectManager[0];

       float r_pushf = 0.0f;

        affect->accumulated_time += dt;

        affect->accumulated_time = Clamp(affect->accumulated_time, 0.0f, affect->reset_time);

        r_pushf = -affect->shake_kick *
            expf(-affect->reset_time * affect->accumulated_time) *
            ((sinf(affect->shake_speed * affect->accumulated_time) -
                GetSystem(Camera)->getUpVector().x + GetSystem(Camera)->getRightVector().y) +
                 (cosf(affect->shake_speed * affect->accumulated_time) -
                    GetSystem(Camera)->getRightVector().x + GetSystem(Camera)->getUpVector().y));

        GetSystem(Camera)->rotate(r_pushf);

    }

    void Camera::XMove(const glm::vec2* scrollspd) 
    {
        this->moveRight(&scrollspd->x);
    }

    void Camera::YMove(const glm::vec2* scrollspd)
    {
        this->moveUp(&scrollspd->y);
    }

    void Camera::setCameraParams(const glm::vec4& C, const glm::vec4& v, float W, float H)
    {

        center_point = { C.x, C.y, C.z, C.w };

        up_vector = glm::normalize(v);

        right_vector = { up_vector.y, up_vector.x, 0, 0 };


        rect_width = W;
        rect_height = H;
    }


    bool Camera::ReceiveMessage(std::vector<std::string> incomingMessage) 
    {
        if (incomingMessage[0] != "CameraEffect" || CamAffectManager.empty())
        {
            return false;
        }

        //we check for specific messages depending on if the thing is controlled (timed) or not
        if (!CamAffectManager[0]->getIsActive()) 
        {
            glm::vec2 scroll_speed = *CamAffectManager[0]->GetScrollSpeed();
            float shake_speed = *CamAffectManager[0]->GetShakeSpeed();

            if (!CamAffectManager[0]->getIsTimed())
            {

                if (!CamAffectManager[0]->getIsActive()) 
                {
                        // non - scripted movement will only take these messages.
                        if (incomingMessage[1] == "UP" ||
                            incomingMessage[1] == "DOWN" ||
                            incomingMessage[1] == "LEFT" ||
                            incomingMessage[1] == "RIGHT")
                        {
                            CamAffectManager[0]->setIsActive(true);
                            return true;
                        }
                }
            }
            else if (CamAffectManager[0]->getIsTimed()) 
            {
                if (!CamAffectManager[0]->getIsActive())
                {
                    // scripted movement will only take these messages.
                    if (incomingMessage[1] == "UP" ||
                        incomingMessage[1] == "DOWN" ||
                        incomingMessage[1] == "LEFT" ||
                        incomingMessage[1] == "RIGHT" || 
                        incomingMessage[1] == "DoEffect")
                    {
                        CamAffectManager[0]->setIsActive(true);
                        return true;
                    }
                }
            }

        }
        else 
        {
            //this could be a vector of problems with queued and activated affects. It's activated by callback. 
            //scripted affects, which I imagine would actually be fine.
           
            if (!CamAffectManager[0]->getIsTimed()) //This only means input could be interrupted by s
            {
                if (incomingMessage[0] == "RELEASE")
                {
                    CamAffectManager[0]->setIsActive(false);
                }
            }

        }



        return false;

    }

    Camera& Camera::moveRight(const float* x)
    {
        glm::mat4 translate = { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {*x, 0, 0, 1} };

        glm::mat4 center_translate = CameraUtility::cameraToWorld(*this);

        center_translate *= translate;

        center_point = center_translate[3];

        return *this;
    }

    void Camera::modifyView() 
    {

        float win_width = (float)GetSystem(Window)->getWindowWidth();
        float win_height = (float)GetSystem(Window)->getWindowHeight();

        const Camera* camera = GetSystem(Camera);

        glm::mat4 view =
        glm::lookAt(glm::vec3(camera->getCenter().x,camera->getCenter().y, 
        camera->getCenter().w), glm::vec3(camera->getCenter().x, camera->getCenter().y, 0.0f),
        static_cast<glm::vec3>(camera->getUpVector()));


        std::vector<Shader*>* shaders = GetSystem(ShaderManager)->GetShaders();
        for (unsigned int loop = 0; loop < shaders->size(); ++loop)
        {
            // Set the matrices on the color shader
            glUseProgram((*shaders)[loop]->getId());
            CheckError();
            GLint viewLoc = glGetUniformLocation((*shaders)[loop]->getId(), "view");
            CheckError();
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            CheckError();
        }

        // Reset the shader
        glUseProgram(0);
        CheckError();

    }

    void Camera::Update(float dt) 
    {
        if (!CamAffectManager.empty()) 
        {
            if (CamAffectManager[0]->getIsActive())
            {
                if (CamAffectManager[0]->CamAffectUpdate(dt))
                {
                    modifyView();
                }
                else
                {
                    CamAffectManager[0]->setIsActive(false);
                }

            }

        }

    }

    Camera& Camera::moveUp(const float* y)
    {
        glm::mat4 translate = { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, *y, 0, 1} };

        glm::mat4 center_translate = CameraUtility::cameraToWorld(*this);

        center_translate *= translate;

        center_point = center_translate[3];

        return *this;
    }

    Camera& Camera::rotate(float t)
    {
        glm::mat4 CameraRotate = GraphicsUtility::rotate(t);

        right_vector = CameraRotate * right_vector;

        up_vector = CameraRotate * up_vector;


        return *this;
    }


    Camera& Camera::zoom(float f)
    {
        rect_width *= f;
        rect_height *= f;

        return *this;
    }

   
    const glm::vec4& Camera::getRightVector() const
    {
        return right_vector;
    }

    const glm::vec4& Camera::getUpVector() const
    {
        return this->up_vector;
    }

    const glm::vec4& Camera::getCenter() const 
    {
        return center_point;
    }

    const float& Camera::getRectWidth() const {

        return rect_width;
    }

    const float& Camera::getRectHeight() const
    {
        return rect_height;
    }

    const glm::mat4 Camera::getWorldToNDC() 
    {
        using namespace CameraUtility;

        const Camera* camera = GetSystem(Camera);

        return cameraToNDC(*camera) * worldToCamera(*camera);
       
    }

    namespace CameraUtility 
    {
        using namespace GraphicsUtility;

        glm::mat4 cameraToWorld(const Camera& cam)
        {
            return affine(cam.getRightVector(), cam.getUpVector(), cam.getCenter());
        }

        glm::mat4 worldToCamera(const Camera& cam)
        {
            return affineInverse(affine(cam.getRightVector(), cam.getUpVector(), cam.getCenter()));
        }

        glm::mat4 cameraToNDC(const Camera& cam)
        {
            return scale(2 / cam.getRectWidth(), 2 / cam.getRectHeight());
        }

        glm::mat4 NDCToCamera(const Camera& cam)
        {
            return scale(cam.getRectWidth() / 2, cam.getRectHeight() / 2);
        }
    }
}