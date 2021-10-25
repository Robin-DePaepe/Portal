#pragma once
#include "PostProcessingMaterial.h"

class ID3D11EffectShaderResourceVariable;

class CenterGlow : public PostProcessingMaterial
{
public:
	CenterGlow();
	virtual ~CenterGlow() = default;

	CenterGlow(const CenterGlow& other) = delete;
	CenterGlow(CenterGlow&& other) noexcept = delete;
	CenterGlow& operator=(const CenterGlow& other) = delete;
	CenterGlow& operator=(CenterGlow&& other) noexcept = delete;

	void SetGlowSizeRatio(const float ratio);
	void UseCubeShape(bool value);

private:
	//functions
	void LoadEffectVariables() override;
	void UpdateEffectVariables(RenderTarget* pRendertarget) override;
	virtual void Update(const GameContext& gameContex) override;

	//datamembers
	ID3DX11EffectShaderResourceVariable* m_pTextureMapVariable;
	ID3DX11EffectScalarVariable* m_pGlowSizeRatio; //ratio 0-1 on how much white in the screen
	ID3DX11EffectScalarVariable* m_pGlowOpacity; 
	ID3DX11EffectScalarVariable* m_pUseCubeShape;

	float m_GlowSizeRatio,m_GlowOpacity;
	const float m_MaxOpacity, m_PulseRate;
	bool m_UseCubeShape;//else its a circle
	bool m_PulseIncreasing;
};
