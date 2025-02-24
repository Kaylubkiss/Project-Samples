// -----------------------------------------------------------------------------
// \Project Name Lunar Sword
// \filename     ButtonManager.h
// \author       cwkmi
// \date         10/8/2022 9:13:37 PM
// \brief        //FILL IN
//
// Copyright (c) 2022 DigiPen, All rights reserved.
// -----------------------------------------------------------------------------

#ifndef ButtonManager_H
#define ButtonManager_H

// Forward references

// Includes
#include <vector>
#include "Button.h"
#include "ISystem.h"
#include "Renderer.h"
#include "SceneId.h"
#include "GraphicsLibraries.h"
#include "GraphicsUtility.h"

namespace LunarSword
{

	class ButtonSystem : public ISystem
	{

	public:
		ButtonSystem();
		~ButtonSystem();
		const std::string GetName() override;
		bool ReceiveMessage(std::vector<std::string> incomingMessage);
		void Update(float dt) override;
		void Render() override;
		bool CollidedButton(Button* button, const glm::vec2& object);




		//componentManager's problem now
		/*void AddButton(Button* button);

		std::vector<Button*> GetButtonList();*/

		friend class Renderer;
		friend class Button;
	private:
		void TrueRender(SceneId::SceneEnum active_scene, unsigned int meshID, float debug_border_thickness, Color* debug_border_RGBA, GLuint shader);
	};
	bool onClickButton(GLFWwindow* window, int button, int action);

	bool onHoverButton(GLFWwindow* window, double xpos, double ypos);
}
#endif

