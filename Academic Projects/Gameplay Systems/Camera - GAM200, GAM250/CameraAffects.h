// -----------------------------------------------------------------------------
// \Project Name Lunar Sword
// \filename     CameraAffects.h
// \author       Caleb
// \date         1/31/2023 11:32:38 AM
// \brief        The header for the pretentiously named interface.
//
// Copyright � 2023 DigiPen, All rights reserved.
// -----------------------------------------------------------------------------

#ifndef CameraAffect_H
#define CameraAffect_H

#include "GraphicsLibraries.h"
#include "Camera.h"
#include "ISystemManager.h"
#include <vector>
#include <iostream>
#include "ISystem.h"

namespace LunarSword
{
	class Camera;
	class CameraAffect;

	#define CamAffectManager (*(GetSystem(Camera)->GetCamAffectManager()))

	class CameraAffect
	{
	public:

		CameraAffect();
		CameraAffect(std::string name);
		const float* GetShakeSpeed();
		const float* GetShakeSpeed() const;
		const glm::vec2* GetScrollSpeed();
		const glm::vec2* GetScrollSpeed() const;
		const float* GetAffectTime() const;
		const float* GetResetTime() const;
		bool getIsActive();
		bool getIsTimed();
		void setIsActive(bool val);
		void Deserialize(std::string path, CameraAffect* effect);
		bool CamAffectUpdate(float dt);
		void (*doShake)(float dt);



	private:
		float shake_speed;
		float shake_kick;
		glm::vec2 scrollspd;
		float affect_time;
		float accumulated_time;
		float reset_time;
		bool is_timed;
		bool isActive;

		friend class Camera;


	};
}
#endif

