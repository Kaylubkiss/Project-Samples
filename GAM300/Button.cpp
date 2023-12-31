// -----------------------------------------------------------------------------
// \Project Name Lunar Sword
// \filename     Button.cpp
// \author       Caleb
// \date         10/8/2022 2:35:16 PM
// \brief        Caleb, 100%
//
// Copyright (c) 2022 DigiPen, All rights reserved.
// -----------------------------------------------------------------------------

#include "Button.h"
#include "EntityManager.h"
#include "ButtonSystem.h"
#include "Window.h"
#include "ISystemManager.h"
#include <iostream>
#include <vector>
#include "Window.h"
#include "MessagingSystem.h"
#include "GraphicsUtility.h"
#include "SceneManager.h"
#include "Audio.h"

// other includes

// code
namespace LunarSword 
{
	
	Button::Button() : Button(ComponentId::GetInvalid())
	{
		setup_finished = false;
		isHoveredOver = false;
		just_activated = false;
		hover_over_hue = false;
	}

	Button::Button(ComponentId _id):
	IComponent(_id),  relative_size(), isHoveredOver()
	{
		setup_finished = false;
		isHoveredOver = false;
		just_activated = false;
		hover_over_hue = false;
	}

	Button::~Button() 
	{
	}
	
	glm::vec2 Button::getRelativeSize() 
	{
		return relative_size;
	}

	

	void Button::Deserialize(rapidjson::Value const& jsonObj) 
	{
		if (jsonObj.HasMember("Hue Shift Hover Over Effect"))
		{
			hover_over_hue = jsonObj["Hue Shift Hover Over Effect"].GetBool();
		}


		relative_size.x = jsonObj["RelativeWidth"].GetFloat();
		relative_size.y = jsonObj["RelativeHeight"].GetFloat();

		

		//std::vector<std::string> message_list;

		for (unsigned i = 0; i < jsonObj["onClickMessages"].GetArray().Size(); ++i) 
		{
			std::vector<std::string> msgs;

			for (unsigned j = 0; j < jsonObj["onClickMessages"].GetArray()[i].Size(); ++j) 
			{
				std::string text = jsonObj["onClickMessages"].GetArray()[i].GetArray()[j].GetString();
				
				
				msgs.push_back(text);
			
	
			}
			messagesClick.push_back(msgs);
		
		}

		for (unsigned i = 0; i < jsonObj["onHoverMessages"].GetArray().Size(); ++i)
		{
			std::vector<std::string> msgs;

			for (unsigned j = 0; j < jsonObj["onHoverMessages"].GetArray()[i].Size(); ++j)
			{
				std::string text = jsonObj["onHoverMessages"].GetArray()[i].GetArray()[j].GetString();

				msgs.push_back(text);


			}
			messagesHover.push_back(msgs);

		}

		if (jsonObj.HasMember("onEndHoverMessages"))
		{
			for (unsigned i = 0; i < jsonObj["onEndHoverMessages"].GetArray().Size(); ++i)
			{
				std::vector<std::string> msgs;

				for (unsigned j = 0; j < jsonObj["onEndHoverMessages"].GetArray()[i].Size(); ++j)
				{
					std::string text = jsonObj["onEndHoverMessages"].GetArray()[i].GetArray()[j].GetString();

					msgs.push_back(text);


				}
				messagesEndHover.push_back(msgs);

			}
		}

		origMessagesClick = messagesClick;
		origMessagesHover = messagesHover;
		origMessagesEndHover = messagesEndHover;
	}

	IComponent* Button::InternalClone(EntityId const& toAttach) const 
	{
		auto* clone = GetSystem(ComponentManager)->Create<Button>(toAttach);

		clone->relative_size.x= relative_size.x;
		clone->relative_size.y = relative_size.y;

		clone->origMessagesClick = origMessagesClick;
		clone->origMessagesHover = origMessagesHover;
		clone->origMessagesEndHover = origMessagesEndHover;

		clone->messagesClick = origMessagesClick;
		clone->messagesHover = origMessagesHover;
		clone->messagesEndHover = origMessagesEndHover;

		clone->setup_finished = false;
		clone->hover_over_hue = hover_over_hue;

		return clone;
	}



	std::vector<std::vector<std::string>>* Button::getMessageClickArray() 
	{
		return &messagesClick;
	}

	std::vector<std::vector<std::string>>* Button::getMessageHoverArray()
	{
		return &messagesHover;
	}

	bool Button::GetHoverState() 
	{
		return isHoveredOver;
	}

	// Will also send onEndHoverMessages
	void Button::SetHoverState(bool state, bool got_clicked)
	{
		if (hover_over_hue)
		{
			EntityId parent = GetParentId();
			Color* color_ptr = GetSystem(ComponentManager)->GetFrom<Sprite>(parent)->GetRGBA();
			if (state)
			{
				color_ptr->activateHueShift();
			}
			else
			{
				color_ptr->deactivateHueShift();
			}
		}

		if (isHoveredOver != state && got_clicked == false && state == true)
		{
			Audio* audio = GetSystem(ComponentManager)->GetFrom<Audio>(GetParentId());
			if (audio && audio->GetAllowSound())
			{
				audio->PlayAudio(SoundType::cHoverButtonSound);
			}
		}

		// If now done, send the onEndHoverMessages
		if (isHoveredOver == true && state == false && messagesEndHover.empty() == false && got_clicked == false)
		{
			// Set up the messages
			SetUpMessageNames();

			GetSystem(MessagingSystem)->MessagingSendManyMessages(messagesEndHover);
		}

		isHoveredOver = state;
	}

	void Button::SetUpMessageNames()
	{
		// If already finished with this, just return
		if (setup_finished)
		{
			return;
		}

		setup_finished = true;

		// Put in the $$SELF
		for (auto& sub : messagesClick)
		{
			for (auto& str : sub)
			{
				if (str == "$$SELF_NAME")
				{
					str = GetSystem(EntityManager)->GetEntity(GetParentId())->GetName();
				}

			}
		}
		for (auto& sub : messagesHover)
		{
			for (auto& str : sub)
			{
				if (str == "$$SELF_NAME")
				{
					str = GetSystem(EntityManager)->GetEntity(GetParentId())->GetName();
				}

			}
		}
		for (auto& sub : messagesEndHover)
		{
			for (auto& str : sub)
			{
				if (str == "$$SELF_NAME")
				{
					str = GetSystem(EntityManager)->GetEntity(GetParentId())->GetName();
				}

			}
		}
	}

	bool Button::GetJustActivated()
	{
		return just_activated;
	}

	void Button::SetJustActivated(bool new_state)
	{
		just_activated = new_state;
	}
}
