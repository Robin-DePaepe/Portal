#pragma once
#include "PostProcessingMaterial.h"

class ID3D11EffectShaderResourceVariable;

class CombineResources : public PostProcessingMaterial
{
public:
	CombineResources();
	virtual ~CombineResources() = default;

	CombineResources(const CombineResources& other) = delete;
	CombineResources(CombineResources&& other) noexcept = delete;
	CombineResources& operator=(const CombineResources& other) = delete;
	CombineResources& operator=(CombineResources&& other) noexcept = delete;
	void SetSecondTexture(RenderTarget* pRendertarget);

private:
	//functions
	void LoadEffectVariables() override;
	void UpdateEffectVariables(RenderTarget* pRendertarget) override;
	virtual void Update(const GameContext& gameContex) override { UNREFERENCED_PARAMETER(gameContex); };

	//datamembers
	ID3DX11EffectShaderResourceVariable* m_pTextureMap1Variable;
	ID3DX11EffectShaderResourceVariable* m_pTextureMap2Variable;
};
