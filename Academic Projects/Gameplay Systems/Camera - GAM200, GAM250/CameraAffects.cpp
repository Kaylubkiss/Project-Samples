// -----------------------------------------------------------------------------
// \Project Name Lunar Sword
// \filename     CameraAffects.cpp
// \author       Caleb
// \date         1/31/2023 11:34:42 AM
// \brief        Pretentiously titled, 
								 //but it's all about effects for the camera.
//
// Copyright � 2023 DigiPen, All rights reserved.
// -----------------------------------------------------------------------------

#include "CameraAffects.h"
#include <fstream>
#include <iostream>
#include <istreamwrapper.h>
#include "Utility.h"
#include "Interpolation.h"

namespace LunarSword
{

    //IF YOU DON'T WANT TO GIVE THE AFFECT ANY PARTICULAR ATTRIBUTES, SAUL'S GOT YOU COVERED!!
    CameraAffect::CameraAffect() : isActive(false),
    affect_time(0.0f), is_timed(false), reset_time(affect_time)
    {
        std::string path = "./Data/CameraAffects/Camera.json";

        Deserialize(path, this);

    }

    const float* CameraAffect::GetShakeSpeed()
    {
        return &shake_speed;
    }

    const float* CameraAffect::GetShakeSpeed() const
    {
        return &shake_speed;
    }

    const glm::vec2* CameraAffect::GetScrollSpeed()
    {
        return &scrollspd;
    }

    const glm::vec2* CameraAffect::GetScrollSpeed() const
    {
        return &scrollspd;
    }

    const float* CameraAffect::GetResetTime() const
    {
        return &reset_time;
    }

    bool CameraAffect::getIsTimed()
    {
        return is_timed;
    }

    void QueueCamAffect(CameraAffect* effect)
    {
        //we are assuming that it is already in the vector,
        //although if not 
        auto& manager = *GetSystem(Camera)->GetCamAffectManager();

        int index = GetSystem(Camera)->FindCamAffect(effect);

        if (index == -1)
        {
            GetSystem(Camera)->AddCamAffect(effect);

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
        if (!manager[0]->getIsTimed() || 
            !manager[0]->getIsActive() && 
            !manager[1]->getIsActive() && 
            index != 0)
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

    bool CameraAffect::CamAffectUpdate(float dt) //timer stuff
    {
        auto& camera_affect_manager = *GetSystem(Camera)->GetCamAffectManager();

     if (camera_affect_manager[0]->is_timed)
     {
         camera_affect_manager[0]->affect_time -= dt;

         camera_affect_manager[0]->doShake(dt);

         if (camera_affect_manager[0]->affect_time <= 0.0f)
         {
             GetSystem(Camera)->setCameraParams({ 0, 0, 0, 1 }, { 0, 1, 0, 0 }, 
             GetSystem(Camera)->getRectWidth(), GetSystem(Camera)->getRectHeight()); //we're resetting the camera position.
             GetSystem(Camera)->modifyView();

             camera_affect_manager[0]->affect_time = reset_time;

             accumulated_time = 0.0f;
             
             return false;
         }
     }

     return true;
    }

    bool CameraAffect::getIsActive()
    {
        return isActive;
    }

    const float* CameraAffect::GetAffectTime() const {

        return &affect_time;
    }


    void CameraAffect::setIsActive(bool val)
    {
        auto& manager = *GetSystem(Camera)->GetCamAffectManager();

        if (is_timed)
        {
            if (isActive != val)
            {
                isActive = val;

                if (isActive == false)
                {
                    Camera::DecremenetNumOfScriptedFX();
                }
                else
                {
                    Camera::IncremenetNumOfScriptedFX();
                }
            }
            else
            {
                assert("already this value, stop!");
                return;
            }
        }

        GetSystem(Camera)->QueueCamAffect(this);


    }

    CameraAffect::CameraAffect(std::string name) : isActive(false),
        affect_time(0.0f), is_timed(false), reset_time(0.0f) //affect_time is not jsoned yet
    {
        std::string path = "./Data/CameraAffects/" + name + ".json";

        Deserialize(path, this);

    }


    void CameraAffect::Deserialize(std::string path, CameraAffect* effect)
    {
        //creating json object.
       /* using pf = rapidjson::ParseFlag;
        rapidjson::Document jsonObj;
        std::ifstream ifs(path);
        rapidjson::IStreamWrapper isw(ifs);
        jsonObj.ParseStream<pf::kParseCommentsFlag | pf::kParseTrailingCommasFlag | pf::kParseEscapedApostropheFlag>(isw);
        assert(("File parsing error occured", !jsonObj.HasParseError()));*/

        rapidjson::Document jsonObj = CreateJsonObject(path); //I don't know if this works, actually.

        bool shake_type = jsonObj["YShake"].GetBool();
        bool shake_type2 = jsonObj["XShake"].GetBool();
        bool shake_type3 = jsonObj["RotationalShake"].GetBool();

        if (shake_type)
        {
            effect->doShake = Camera::YShake;
        }
        else if (!effect->doShake && shake_type2)
        {
            effect->doShake = Camera::XShake;
        }
        else if (!effect->doShake && shake_type3)
        {
            effect->doShake = Camera::RotationalShake;
        }
        else
        {
            effect->doShake = NULL;
        }


        //deserialize on the spot!!
        effect->shake_kick = jsonObj["ShakeKick"].GetFloat();
        effect->shake_speed = jsonObj["ShakeSpeed"].GetFloat();
        effect->scrollspd.x = jsonObj["X_Speed"].GetFloat();
        effect->scrollspd.y = jsonObj["Y_Speed"].GetFloat();
        effect->is_timed = jsonObj["IsTimed"].GetBool();
        effect->affect_time = jsonObj["AffectTime"].GetFloat();

        if (effect->is_timed == false)
        {
            effect->affect_time = 0.0f;
        }

        effect->reset_time = affect_time;

    }
}
