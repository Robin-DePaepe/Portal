#pragma once
#include "PostProcessingMaterial.h"

class ID3D11EffectShaderResourceVariable;

class PostGrayscale : public PostProcessingMaterial
{
public:
	PostGrayscale();
	PostGrayscale(const PostGrayscale& other) = delete;
	PostGrayscale(PostGrayscale&& other) noexcept = delete;
	PostGrayscale& operator=(const PostGrayscale& other) = delete;
	PostGrayscale& operator=(PostGrayscale&& other) noexcept = delete;
	virtual ~PostGrayscale() = default;

private:
	//functions
	void LoadEffectVariables() override;
	void UpdateEffectVariables(RenderTarget* pRendertarget) override;
	virtual void Update(const GameContext& gameContex) override { UNREFERENCED_PARAMETER(gameContex); };

	//datamembers
	ID3DX11EffectShaderResourceVariable* m_pTextureMapVariable;
};
