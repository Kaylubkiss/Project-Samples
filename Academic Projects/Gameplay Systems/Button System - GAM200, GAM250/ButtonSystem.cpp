// -----------------------------------------------------------------------------
// \Project Name Lunar Sword
// \filename     ButtonSystem.cpp
// \author       Michael Haynes, Caleb Kissinger
// \date         10/8/2022 9:43:29 PM
// \brief        Michael: 80%, everything else
//			     		 Caleb Kissinger: 20%, CollidedButton
//
// Copyright (c) 2022 DigiPen, All rights reserved.
// -----------------------------------------------------------------------------

#include "ButtonSystem.h"
#include "ComponentManager.h"
#include "ISystemManager.h"
#include "Transform.h"
#include "ISystem.h"
#include "Sprite.h"
#include "GraphicsUtility.h"
#include "ShaderManager.h"
#include "MessagingSystem.h"
#include "EntityManager.h"
#include "Window.h"
#include "Audio.h"
#include "Collision.h"
#include "SceneManager.h"
#include "InputSystem.h"
#include "Sprite.h"

// other includes

// code
namespace LunarSword
{
	//componentManager's problem now
	//static std::vector<Button*> buttonList; //one time definition

	ButtonSystem::ButtonSystem() : ISystem()
	{
		TraceMessage("ButtonSystem Init");


		// Create the handler for the buttons
		Handler* hand = new Handler;
		hand->onClick = onClickButton;
		hand->onHoverOver = onHoverButton;

		// Register the handler
		GetSystem(InputSystem)->RegisterMouseHandler(hand);
	}
	const std::string ButtonSystem::GetName() 
	{

		return "ButtonSystem";

	}
	ButtonSystem::~ButtonSystem() 
	{
		TraceMessage("ButtonSystem Shutdown");

	}

	void ButtonSystem::Render() 
	{


	}

	// Used in _DEBUG drawing to draw the collision border of the button
	void ButtonSystem::TrueRender(SceneId::SceneEnum active_scene, unsigned int meshID, float debug_border_thickness, Color* debug_border_RGBA, GLuint shader)
	{

		auto ButtonSystemRender = [active_scene, meshID, debug_border_thickness, debug_border_RGBA, shader](Button* buttonComp)
		{
			// If the current Button is in the scene, draw the border around it
			if (buttonComp->GetSceneId().GetEnum() == active_scene)
			{
				// Get the parent of the button, and the transfrom from the parent
				EntityId const& parent = buttonComp->GetParentId();

				auto manager = GetSystem(EntityManager);

				auto ptr = manager->GetEntity(parent);

				if (ptr)
				{
					if (!ptr->GetIsEnabled())
					{
						return;
					}
				}

				Transform* transform = GetSystem(ComponentManager)->GetFrom<Transform>(parent);

				// Store the transform's scale
				glm::vec4 transform_scale = *transform->getScale() * GraphicsUtility::vector(buttonComp->getRelativeSize().x, buttonComp->getRelativeSize().y);

				// Start drawing each border
				for (int loop = 0; loop < 4; ++loop)
				{
					// Scale and move the border first
					glm::mat4 scale_and_translate;
					// For the left and right borders
					if (loop % 2 == 0)
					{

						scale_and_translate = GraphicsUtility::translate(GraphicsUtility::vector(transform_scale.x / 2.0f - debug_border_thickness / 2.0f, 0.0f))
							* GraphicsUtility::scale(debug_border_thickness, transform_scale.y);
					}
					// For top and bottom borders
					else
					{
						scale_and_translate = GraphicsUtility::translate(GraphicsUtility::vector(transform_scale.y / 2.0f - debug_border_thickness / 2.0f, 0.0f))
							* GraphicsUtility::scale(debug_border_thickness, transform_scale.x);
					}


					// Get the current border's offset using the translation, rotate based on current loop, and the scale_and_translate matricies
					glm::mat4 updated_matrix = GraphicsUtility::translate(*transform->getTranslation()) * 
						GraphicsUtility::rotate(((float)loop * 90.0f) + glm::radians(transform->getRotation()))
						* scale_and_translate * GraphicsUtility::rotate(-90.0f); //CAMERA_TOUCH


					// Now draw the current border
					GetSystem(Renderer)->RendererDraw(updated_matrix, meshID, 4294967295,
						0, debug_border_RGBA, shader);
				}
			}
		};

		ComponentManager* manager = GetSystem(ComponentManager);
		manager->ForEach<Button>(ButtonSystemRender, false);


	}

	bool ButtonSystem::ReceiveMessage(std::vector<std::string> incomingMessage)
	{
		return false;
	}

	void ButtonSystem::Update(float dt) 
	{
		auto ButtonSystemUpdate = [dt](Button* buttonComp)
		{
			if (buttonComp->GetHoverState()) 
			{
				GetSystem(MessagingSystem)->MessagingSendManyMessages(*(*buttonComp).getMessageHoverArray());

				// Set the just_activated state to false
				buttonComp->SetJustActivated(false);

				// Update hue shift
				GetSystem(ComponentManager)->GetFrom<Sprite>(buttonComp->GetParentId())->GetRGBA()->updateHueShift(dt);
			}
		};

		ComponentManager* manager = GetSystem(ComponentManager);
		manager->ForEach<Button>(ButtonSystemUpdate);

	}

	bool ButtonSystem::CollidedButton(Button* button, const glm::vec2& object) 
	{

		EntityId const& parent = button->GetParentId();

		//do a little check first
		auto manager = GetSystem(EntityManager);

		auto ptr = manager->GetEntity(parent);

		if (ptr)
		{
			if (!ptr->GetIsEnabled())
			{
				return false;
			}
		}

		//now let's proceed with calcluations
		Transform* transformParent = GetSystem(ComponentManager)->GetFrom<Transform>(parent);

		if (transformParent == NULL) return false;

		const glm::vec4* scaleParent = transformParent->getScale();
		const glm::vec4* positionParent = transformParent->getTranslation();

		int window_height = GetSystem(Window)->getWindowHeight();
		int window_width = GetSystem(Window)->getWindowWidth();

		int wind_oldwidth = GetSystem(Window)->getOldWidth();
		int wind_oldheight = GetSystem(Window)->getOldHeight();


		float current_scale_x = (*scaleParent).x * button->getRelativeSize().x *
			(static_cast<float>(window_width) / static_cast<float>(wind_oldwidth));
		float current_scale_y = (*scaleParent).y * button->getRelativeSize().y *
			(static_cast<float>(window_height) / static_cast<float>(wind_oldheight));

		glm::vec4 box_vec((*positionParent).x *
				(static_cast<float>(window_width) / static_cast<float>(wind_oldwidth)) + window_width / 2,
				((-1 * (*positionParent).y) *
					(static_cast<float>(window_height) / static_cast<float>(wind_oldheight))) + window_height / 2,
				0, 1);

		
		Collision::Box boxButton = Collision::constructBox(box_vec.x - (current_scale_x) / 2,
			box_vec.y - (current_scale_y) / 2,
			box_vec.x - (current_scale_x) / 2 + current_scale_x,
			box_vec.y + (current_scale_y) / 2 ); //use ctr for box


		if (Collision::PointBoxCollide({ object.x, object.y }, boxButton))
		{
			return true;
		}

		return false;

	}

		
	bool onClickButton(GLFWwindow* window, int button, int action) //this will probably be put into buttonSystem
	{
		glm::dvec2 currCursorPosition = { 0,0 };

		glfwGetCursorPos(window, &currCursorPosition.x, &currCursorPosition.y);

		ComponentManager* manager = GetSystem(ComponentManager);

		std::list<SceneId::SceneEnum>& activeScenes = GetSystem(SceneManager)->GetActiveScenesNon();

		// Used to prevent drawing the same thing multiple times
		SceneId::SceneEnum prev = SceneId::SceneEnum::cInvalid;

		// Loop through all the scenes backwards
		for (int loop = (int)activeScenes.size() - 1; loop >= 0; --loop)
		{
			// Get the id for this scene
			std::list<SceneId::SceneEnum>::iterator id = activeScenes.begin();
			std::advance(id, loop);

			// Used to prevent drawing the same thing multiple times
			if (prev == *id)
			{
				continue;
			}
		
			prev = *id;

			auto ButtonSystemUpdate = [currCursorPosition, action, id](Button* button)
			{
				// If the button isn't in this scene, return false
				if (button->GetSceneId().GetEnum() != *id)
				{
					return false;
				}


				if (GetSystem(ButtonSystem)->CollidedButton(button, currCursorPosition))
				{
					if (action == GLFW_RELEASE)
					{
						EntityId const& parent = button->GetParentId();

						button->SetUpMessageNames();

						// Set the hover state to false, and make sure that the audio doesn't play
						button->SetHoverState(false, true);

						GetSystem(MessagingSystem)->MessagingSendManyMessages(*button->getMessageClickArray());
						Audio* audio = GetSystem(ComponentManager)->GetFrom<Audio>(parent);
						if (audio && audio->GetAllowSound())
						{
							audio->PlayAudio(SoundType::cButtonSound);
						}
					}

					return true;

				}

				return false;

			};

			// If the button returns true, exit this function; if not, keep looping
			bool result = manager->ForEachCoR<Button>(ButtonSystemUpdate);

			if (result)
			{
				return result;
			}
		}

		return false;
	}

	bool onHoverButton(GLFWwindow* window, double xpos, double ypos)
	{
		// Used to set all button's hovered state to false if one is foudn
		bool success = false;
		bool* succ_ptr = &success;

		glm::dvec2 currCursorPosition = { xpos, ypos };

		//glfwGetCursorPos(window, &currCursorPosition.x, &currCursorPosition.y);

		ComponentManager* manager = GetSystem(ComponentManager);

		std::list<SceneId::SceneEnum>& activeScenes = GetSystem(SceneManager)->GetActiveScenesNon();

		// Used to prevent drawing the same thing multiple times
        SceneId::SceneEnum prev = SceneId::SceneEnum::cInvalid;

		// Loop through all the scenes backwards
		for (int loop = (int)activeScenes.size() - 1; loop >= 0; --loop)
		{
			// Get the id for this scene
			std::list<SceneId::SceneEnum>::iterator id = activeScenes.begin();
			std::advance(id, loop);

			// Used to prevent drawing the same thing multiple times
			if (prev == *id)
			{
				continue;
			}
			prev = *id;


			auto ButtonSystemUpdate = [currCursorPosition, id, succ_ptr](Button* button)
			{
				// If the button isn't in this scene, return
				if (button->GetSceneId().GetEnum() != *id)
				{
					return;
				}

				// Only check if no other button has been successful yet
				if (*succ_ptr == false && GetSystem(ButtonSystem)->CollidedButton(button, currCursorPosition))
				{
					if (!button->GetHoverState())
					{
						// Set the hover state to true
						button->SetHoverState(true);
					}

					// Set the just_activated state to true to prevent setting it to false instantly
					button->SetJustActivated(true);

					// Set up the message names
					button->SetUpMessageNames();

					*succ_ptr = true;
					return;
				}
				else
				{
					// Only set the hover state to false if the button wasn't just activated
					if (button->GetJustActivated() == false)
					{
						button->SetHoverState(false);
					}
				}
			};

			// If the button returns true, exit this function; if not, keep looping
			manager->ForEach<Button>(ButtonSystemUpdate);

		}

		return success;
	}
	
}
