#pragma once
#include "Material.h"

class TextureData;
class DiffuseMaterial final : public Material
{
public:
	DiffuseMaterial(std::wstring effectFile = L"./Resources/Effects/PosNormTex3d.fx");
	~DiffuseMaterial() = default;

	void SetDiffuseTexture(const std::wstring& assetFile);

protected:
	virtual void LoadEffectVariables();
	virtual void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent);

private:
	TextureData* m_pDiffuseTexture;
	static ID3DX11EffectShaderResourceVariable* m_pDiffuseSRVvariable;

	//disable default copy constructor and assignment operator
	DiffuseMaterial(const DiffuseMaterial& obj) = delete;
	DiffuseMaterial& operator=(const DiffuseMaterial& obj) = delete;
};

