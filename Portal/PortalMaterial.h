#pragma once
#include "Material.h"

class PortalMaterial final : public Material
{
public:
	PortalMaterial(bool isBlue);
	virtual ~PortalMaterial() = default;

	PortalMaterial(const PortalMaterial& other) = delete;
	PortalMaterial(PortalMaterial&& other) noexcept = delete;
	PortalMaterial& operator=(const PortalMaterial& other) = delete;
	PortalMaterial& operator=(PortalMaterial&& other) noexcept = delete;

	void SetPortalResourceView(ID3D11ShaderResourceView* view);
	void SetPortalActive(bool value) { m_IsActive = value; }
private:
	//functions
	void LoadEffectVariables() override;
	void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent) override;

	//datamembers
	static	ID3DX11EffectShaderResourceVariable* m_pTextureMapVariable;
	static	ID3DX11EffectScalarVariable* m_pIsBlue;
	static	ID3DX11EffectScalarVariable* m_pIsActive;

	const bool m_IsBlue;
	bool m_IsActive;
};




