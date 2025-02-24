// -----------------------------------------------------------------------------
// \Project Name Lunar Sword
// \filename     Bloom.h
// \author       cwkmi
// \date         3/17/2023 9:34:55 PM
// \brief        //TODO
//
// Copyright � 2023 DigiPen, All rights reserved.
// -----------------------------------------------------------------------------

#ifndef Bloom_H
#define Bloom_H

// Forward references

// Includes
#include "Effects.h"

namespace LunarSword
{
	class Bloom : public Effect
	{
	public:  // methods
	public:  // variables
		Bloom() = delete;
		~Bloom();
		Bloom(unsigned int fbo_id = 0);
		void SetBrightness(const float& exp) override; //HDR class????
		void IncrementBrightness(const float& exp) override;
		void Draw();
		unsigned int GetInitRenderToID() const;
		void UpdateFBOAttachments();

		float GetBrightness();

	private: // methods
		void RenderEffect(const unsigned int effect);
	private: // variables
		enum { HORIZONTAL_BLUR, VERTICAL_BLUR, HDR, THRESHOLD, COMBINED };
		
		FrameBuffer* Effect[5];

		unsigned current_vao = -1;

		std::string shader[5] = {"HorizontalBlur", "VerticalBlur", "HDR", "Threshold", "Combined"};
		
		float hdr_bright = 0.0f;


	};
}
#endif

