// -----------------------------------------------------------------------------
// \Project Name Lunar Sword
// \filename     Button.h
// \author       cwkmi
// \date         10/8/2022 2:35:26 PM
// \brief        //FILL IN
//
// Copyright (c) 2022 DigiPen, All rights reserved.
// -----------------------------------------------------------------------------

#ifndef Button_H
#define Button_H

// Forward references

// Includes
#pragma once
#include "Mesh.h"
#include "Sprite.h"
#include "InputSystem.h"
#include "IComponent.h"
#include "Collision.h"
#include "Transform.h"
#include "GraphicsLibraries.h"


namespace LunarSword
{
	class Button : public IComponent
	{
	public:  // methods
		Button();
		Button(ComponentId _id);
		~Button();
		glm::vec2 getRelativeSize();
	/*	glm::vec2 getScreenSize();*/
		IComponent* InternalClone(EntityId const& toAttach) const;
		void Deserialize(rapidjson::Value const& jsonObj);
		bool GetHoverState();
		void SetHoverState(bool state, bool got_clicked = false);
		void SetUpMessageNames();
		std::vector<std::vector<std::string>>* getMessageHoverArray();
		std::vector<std::vector<std::string>>* getMessageClickArray();

		// Used for hovering things
		bool GetJustActivated();
		void SetJustActivated(bool new_state);
	/*	const Handler* GetHandler();*/
	public:  // variables

	private: // methods

	private: // variables
		glm::vec2 relative_size;

		std::vector<std::vector<std::string>> messagesClick;
		std::vector<std::vector<std::string>> messagesHover;
		std::vector<std::vector<std::string>> messagesEndHover;

		std::vector<std::vector<std::string>> origMessagesClick;
		std::vector<std::vector<std::string>> origMessagesHover;
		std::vector<std::vector<std::string>> origMessagesEndHover;

		bool setup_finished;

		bool isHoveredOver;		
		bool just_activated;

		bool hover_over_hue;
	};
}
#endif

