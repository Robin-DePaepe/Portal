#pragma once
#include "PostProcessingMaterial.h"

class ID3D11EffectShaderResourceVariable;

class PostBlur : public PostProcessingMaterial
{
public:
	PostBlur();
	virtual ~PostBlur() = default;

	PostBlur(const PostBlur& other) = delete;
	PostBlur(PostBlur&& other) noexcept = delete;
	PostBlur& operator=(const PostBlur& other) = delete;
	PostBlur& operator=(PostBlur&& other) noexcept = delete;

	void SetBlurStrength(const float strength) { m_BlurStrength = strength; }
private:
	//functions
	void LoadEffectVariables() override;
	void UpdateEffectVariables(RenderTarget* pRendertarget) override;
	virtual void Update(const GameContext& gameContex) override { UNREFERENCED_PARAMETER(gameContex); };

	//datamembers
	ID3DX11EffectShaderResourceVariable* m_pTextureMapVariable;
	ID3DX11EffectScalarVariable* m_pPixelOffsetMapVariable; //strength of blur

	float m_BlurStrength; 
};
