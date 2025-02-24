// -----------------------------------------------------------------------------
// \Project Name Lunar Sword
// \filename     Camera.h
// \author       Caleb
// \date         11/3/2022 5:14:38 PM
//\brief         Caleb, 100%
//
// Copyright (c) 2022 DigiPen, All rights reserved.
// -----------------------------------------------------------------------------

#ifndef Camera_H
#define Camera_H

// Forward references

// Includes
#include "GraphicsLibraries.h"
#include "IComponent.h"
#include "ISystem.h"
#include "EntityBehavior.h"
#include "Entity.h"
#include "CameraAffects.h"


namespace LunarSword
{
	class Camera;
	class CameraAffect;

	namespace CameraUtility
	{
		glm::mat4 cameraToWorld(const Camera& cam); //________
		glm::mat4 worldToCamera(const Camera& cam); //________
		glm::mat4 cameraToNDC(const Camera& cam); //__________
		glm::mat4 NDCToCamera(const Camera& cam); //__________
	}

	class Camera : public ISystem
	{
	public:  // methods
		Camera();
		~Camera();
		void setCameraParams(const glm::vec4& C, const glm::vec4& v, float W, float H);
		Camera& moveRight(const float* x);
		Camera& moveUp(const float* y);
		Camera& rotate(float t);
		Camera& zoom(float f);
		void modifyView();
		const glm::vec4& getRightVector() const;
		const glm::vec4& getCenter() const;
		const glm::vec4& getUpVector() const;
		const float& getRectWidth() const;
		const float& getRectHeight() const;
		void Update(float dt);
		static const int& NumScriptedFX();
		static int getIndexControlledAffect();
		static void IncremenetNumOfScriptedFX();
		static void DecremenetNumOfScriptedFX();

		static const glm::mat4 getWorldToNDC();

		static void YShake(float dt);
		static void XShake(float dt);
		static void RotationalShake(float dt);
		void XMove(const glm::vec2* scrollspd);
		void YMove(const glm::vec2* scrollspd);

		bool ReceiveMessage(std::vector<std::string> incomingMessage);

		void Deserialize(rapidjson::Value const& jsonObj);

		std::vector<CameraAffect*>* GetCamAffectManager();
		void QueueCamAffect(CameraAffect* effect);
		void AddCamAffect(CameraAffect* affect);
		unsigned FindCamAffect(CameraAffect* affect);

	public:  // variables

	private: // methods
	private: // variables
		std::vector<CameraAffect*> cam_affect_manager;
		glm::vec4 center_point;
		glm::vec4 right_vector, up_vector;
		float rect_width, rect_height;
		static int num_of_scripted_affects;
		static int index_controlled_affect;
		CameraAffect* cam_effect;
		/*CameraAffect* cam_effect2;
		CameraAffect* cam_effect3;*/


	};
	
	/*class CameraAffect 
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
		

	};*/

}

#endif

