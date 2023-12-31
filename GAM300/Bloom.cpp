// -----------------------------------------------------------------------------
// \Project Name Lunar Sword
// \filename     Bloom.cpp
// \author       Caleb
// \date         3/17/2023 9:34:14 PM
// \brief        //TODO
//
// Copyright � 2023 DigiPen, All rights reserved.
// -----------------------------------------------------------------------------

#include "Bloom.h"
#include "ShaderManager.h"
#include "ISystemManager.h"
#include "GraphicalEffects.h"

// other includes
namespace LunarSword 
{
	

    Bloom::Bloom(unsigned int fbo_id)
    {
        for (int index = 0; index < 5; ++index)
        {
            this->Effect[index] = GenerateHDREffect();
            GetSystem(ShaderManager)->CreateShader(shader[index]);
        }

        SetBrightness(1.0f);
    }


    Bloom::~Bloom() 
    {
        for (auto& effect : this->Effect) 
        {
            delete effect;
        }
    }


    void Bloom::RenderEffect(const unsigned int effect) 
    { 
            glActiveTexture(GL_TEXTURE0);
            glUseProgram(GetSystem(ShaderManager)->GetShaderIDByName(this->shader[effect]));

            if (this->Effect[effect]->getVao() != current_vao) 
            {
                glBindVertexArray(this->Effect[effect]->getVao());
                current_vao = this->Effect[effect]->getVao();
            }
            
            glBindTexture(GL_TEXTURE_2D, this->Effect[effect]->GetTextureID());

            glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    unsigned int Bloom::GetInitRenderToID() const
    {
        return this->Effect[THRESHOLD]->getId();
    }

    void Bloom::SetBrightness(const float& exp) 
    {
        this->hdr_bright = exp;

        if (hdr_bright < 0.0f) hdr_bright = 0.0f;

        GLuint exposure_location = 
        GetSystem(ShaderManager)->GetShaderPtr("HDR")->GetAttachUniformVariable("exposure");

        if (exposure_location < 0) throw std::exception("bloom uniform variable does not exist!\n");

        glUseProgram(GetSystem(ShaderManager)->GetShaderIDByName(this->shader[HDR]));

        glUniform1f(exposure_location, hdr_bright);

        glUseProgram(0);
    }

    void Bloom::IncrementBrightness(const float& exp)
    {
        this->hdr_bright += exp;

        if (hdr_bright < 0.0f) hdr_bright = 0.0f;

        GLuint exposure_location =
            GetSystem(ShaderManager)->GetShaderPtr("HDR")->GetAttachUniformVariable("exposure");

        if (exposure_location < 0) throw std::exception("bloom uniform variable does not exist!\n");

        glUseProgram(GetSystem(ShaderManager)->GetShaderIDByName(this->shader[HDR]));

        glUniform1f(exposure_location, hdr_bright);

        glUseProgram(0);
    }

	void Bloom::Draw() 
	{
        glBindFramebuffer(GL_FRAMEBUFFER, this->Effect[HORIZONTAL_BLUR]->getId());

        RenderEffect(THRESHOLD);

        for (int i = 0; i < 2; ++i) 
        {
            glBindFramebuffer(GL_FRAMEBUFFER, this->Effect[VERTICAL_BLUR]->getId());

            RenderEffect(HORIZONTAL_BLUR);

            glBindFramebuffer(GL_FRAMEBUFFER, this->Effect[HORIZONTAL_BLUR]->getId());

            RenderEffect(VERTICAL_BLUR);

        }
        //can keep alternating between horizontal and vertical blur here to get 
        //a better blur. However, there are more efficient methods
        //to keep this fast.

        glBindFramebuffer(GL_FRAMEBUFFER, this->Effect[COMBINED]->getId());

        RenderEffect(VERTICAL_BLUR);

        glBindFramebuffer(GL_FRAMEBUFFER, this->Effect[HDR]->getId());

        glActiveTexture(GL_TEXTURE1);

        glBindTexture(GL_TEXTURE_2D, this->Effect[THRESHOLD]->GetTextureID());

        RenderEffect(COMBINED);

        glBindTexture(GL_TEXTURE_2D, this->Effect[THRESHOLD]->GetTextureID());

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        RenderEffect(HDR);

        current_vao = -1;

	}

    void Bloom::UpdateFBOAttachments()
    {
        for (auto& effect : Effect) 
        {
            effect->UpdateAttachments();
        }
    }

    float Bloom::GetBrightness()
    {
        return hdr_bright;
    }
}
// code

