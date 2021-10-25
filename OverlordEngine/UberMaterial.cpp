#include "stdafx.h"
#include "UberMaterial.h"
#include "ContentManager.h"
#include "TextureData.h"

using namespace DirectX;
using namespace std;

ID3DX11EffectVectorVariable* UberMaterial::m_pLightDirectionVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pUseDiffuseTextureVariable = nullptr;
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pDiffuseSRVvariable = nullptr;
ID3DX11EffectVectorVariable* UberMaterial::m_pDiffuseColorVariable = nullptr;
ID3DX11EffectVectorVariable* UberMaterial::m_pSpecularColorVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pUseSpecularLevelTextureVariable = nullptr;
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pSpecularLevelSRVvariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pShininessVariable = nullptr;
ID3DX11EffectVectorVariable* UberMaterial::m_pAmbientColorVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pAmbientIntensityVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pFlipGreenChannelVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pUseNormalMappingVariable = nullptr;
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pNormalMappingSRVvariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pUseEnvironmentMappingVariable = nullptr;
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pEnvironmentSRVvariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pReflectionStrengthVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pRefractionStrengthVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pRefractionIndexVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pOpacityVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pUseOpacityMapVariable = nullptr;
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pOpacitySRVvariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pUseBlinnVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pUsePhongVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pUseFresnelFalloffVariable = nullptr;
ID3DX11EffectVectorVariable* UberMaterial::m_pFresnelColorVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pFresnelPowerVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pFresnelMultiplierVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pFresnelHardnessVariable = nullptr;

UberMaterial::UberMaterial()
	: Material(L"./Resources/Effects/UberShader.fx"),
	m_pDiffuseTexture(nullptr)
	, m_pOpacityMap{ nullptr }
	, m_pEnvironmentCube{ nullptr }
	, m_pNormalMappingTexture{ nullptr }
	, m_pSpecularLevelTexture{ nullptr }
	, m_LightDirection{ 0.577f,0.577f,0.577f }
	, m_ColorDiffuse{ 1.f,1.f,1.f,1.f }
	, m_ColorSpecular{ 1.f,1.f,1.f,1.f }
	, m_ColorFresnel{ 1.f,1.f,1.f,1.f }
	, m_ColorAmbient{ 0.f,0.f,0.f,1.f }
	, m_Shininess{ 15 }
	, m_bDiffuseTexture{ false }
	, m_bSpecularLevelTexture{ false }
	, m_bOpacityMap{ false }
	, m_bEnvironmentMapping{ false }
	, m_bNormalMapping{ false }
	, m_bSpecularBlinn{ false }
	, m_bSpecularPhong{ false }
	, m_bFresnelFaloff{ false }
	, m_bFlipGreenChannel{ false }
	, m_AmbientIntensity{ 0.f }
	, m_ReflectionStrength{ 0.5f }
	, m_RefractionStrength{ 0.f }
	, m_RefractionIndex{ 0.3f }
	, m_Opacity{ 1.f }
	, m_FresnelPower{ 1.f }
	, m_FresnelMultiplier{ 1.f }
	, m_FresnelHardness{ 0.f }
{}


void UberMaterial::SetLightDirection(XMFLOAT3 direction)
{
	m_LightDirection = direction;
}

void UberMaterial::EnableDiffuseTexture(bool enable)
{
	m_bDiffuseTexture = enable;
}

void UberMaterial::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<TextureData>(assetFile);
}

void UberMaterial::SetDiffuseColor(XMFLOAT4 color)
{
	m_ColorDiffuse = color;
}

void UberMaterial::SetFresnelPower(float power)
{
	m_FresnelPower = power;
}

void UberMaterial::SetFresnelMultiplier(float multiplier)
{
	m_FresnelMultiplier = multiplier;
}


void UberMaterial::SetFresnelHardness(float hardness)
{
	m_FresnelHardness = hardness;
}

void UberMaterial::LoadEffectVariables()
{
	if (!m_pDiffuseSRVvariable) m_pDiffuseSRVvariable = GetEffect()->GetVariableByName("gTextureDiffuse")->AsShaderResource();
	if (!m_pEnvironmentSRVvariable) m_pEnvironmentSRVvariable = GetEffect()->GetVariableByName("gCubeEnvironment")->AsShaderResource();
	if (!m_pSpecularLevelSRVvariable) m_pSpecularLevelSRVvariable = GetEffect()->GetVariableByName("gTextureSpecularIntensity")->AsShaderResource();
	if (!m_pOpacitySRVvariable) m_pOpacitySRVvariable = GetEffect()->GetVariableByName("gTextureOpacity")->AsShaderResource();
	if (!m_pNormalMappingSRVvariable) m_pNormalMappingSRVvariable = GetEffect()->GetVariableByName("gTextureNormal")->AsShaderResource();

	if (!m_pLightDirectionVariable) m_pLightDirectionVariable = GetEffect()->GetVariableByName("gLightDirection")->AsVector();
	if (!m_pAmbientColorVariable) m_pAmbientColorVariable = GetEffect()->GetVariableByName("gColorAmbient")->AsVector();
	if (!m_pSpecularColorVariable) m_pSpecularColorVariable = GetEffect()->GetVariableByName("gColorSpecular")->AsVector();
	if (!m_pDiffuseColorVariable) m_pDiffuseColorVariable = GetEffect()->GetVariableByName("gColorDiffuse")->AsVector();
	if (!m_pFresnelColorVariable) m_pFresnelColorVariable = GetEffect()->GetVariableByName("gColorFresnel")->AsVector();

	if (!m_pUseDiffuseTextureVariable) m_pUseDiffuseTextureVariable = GetEffect()->GetVariableByName("gUseTextureDiffuse")->AsScalar();
	if (!m_pUseSpecularLevelTextureVariable) m_pUseSpecularLevelTextureVariable = GetEffect()->GetVariableByName("gUseTextureSpecularIntensity")->AsScalar();
	if (!m_pShininessVariable) m_pShininessVariable = GetEffect()->GetVariableByName("gShininess")->AsScalar();
	if (!m_pAmbientIntensityVariable) m_pAmbientIntensityVariable = GetEffect()->GetVariableByName("gAmbientIntensity")->AsScalar();
	if (!m_pFlipGreenChannelVariable) m_pFlipGreenChannelVariable = GetEffect()->GetVariableByName("gFlipGreenChannel")->AsScalar();
	if (!m_pUseNormalMappingVariable) m_pUseNormalMappingVariable = GetEffect()->GetVariableByName("gUseTextureNormal")->AsScalar();
	if (!m_pUseEnvironmentMappingVariable) m_pUseEnvironmentMappingVariable = GetEffect()->GetVariableByName("gUseEnvironmentMapping")->AsScalar();
	if (!m_pReflectionStrengthVariable) m_pReflectionStrengthVariable = GetEffect()->GetVariableByName("gReflectionStrength")->AsScalar();
	if (!m_pRefractionStrengthVariable) m_pRefractionStrengthVariable = GetEffect()->GetVariableByName("gRefractionStrength")->AsScalar();
	if (!m_pRefractionIndexVariable) m_pRefractionIndexVariable = GetEffect()->GetVariableByName("gRefractionIndex")->AsScalar();
	if (!m_pOpacityVariable) m_pOpacityVariable = GetEffect()->GetVariableByName("gOpacityIntensity")->AsScalar();
	if (!m_pUseBlinnVariable) m_pUseBlinnVariable = GetEffect()->GetVariableByName("gUseSpecularBlinn")->AsScalar();
	if (!m_pUsePhongVariable) m_pUsePhongVariable = GetEffect()->GetVariableByName("gUseSpecularPhong")->AsScalar();
	if (!m_pUseFresnelFalloffVariable) m_pUseFresnelFalloffVariable = GetEffect()->GetVariableByName("gUseFresnelFalloff")->AsScalar();
	if (!m_pFresnelPowerVariable) m_pFresnelPowerVariable = GetEffect()->GetVariableByName("gFresnelPower")->AsScalar();
	if (!m_pFresnelMultiplierVariable) m_pFresnelMultiplierVariable = GetEffect()->GetVariableByName("gFresnelMultiplier")->AsScalar();
	if (!m_pFresnelHardnessVariable) m_pFresnelHardnessVariable = GetEffect()->GetVariableByName("gFresnelHardness")->AsScalar();
	if (!m_pUseOpacityMapVariable) m_pUseOpacityMapVariable = GetEffect()->GetVariableByName("gUseTextureOpacity")->AsScalar();


}

void UberMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	UNREFERENCED_PARAMETER(gameContext);
	UNREFERENCED_PARAMETER(pModelComponent);

	if (m_pDiffuseTexture && m_pDiffuseSRVvariable) m_pDiffuseSRVvariable->SetResource(m_pDiffuseTexture->GetShaderResourceView());
	if (m_pEnvironmentCube && m_pEnvironmentSRVvariable) m_pEnvironmentSRVvariable->SetResource(m_pEnvironmentCube->GetShaderResourceView());
	if (m_pSpecularLevelTexture && m_pSpecularLevelSRVvariable) m_pSpecularLevelSRVvariable->SetResource(m_pSpecularLevelTexture->GetShaderResourceView());
	if (m_pOpacityMap && m_pOpacitySRVvariable) m_pOpacitySRVvariable->SetResource(m_pOpacityMap->GetShaderResourceView());
	if (m_pNormalMappingTexture && m_pNormalMappingSRVvariable) m_pNormalMappingSRVvariable->SetResource(m_pNormalMappingTexture->GetShaderResourceView());

	if (m_pLightDirectionVariable) m_pLightDirectionVariable->SetFloatVector(&m_LightDirection.x);
	if (m_pDiffuseColorVariable) m_pDiffuseColorVariable->SetFloatVector(&m_ColorDiffuse.x);
	if (m_pSpecularColorVariable) m_pSpecularColorVariable->SetFloatVector(&m_ColorSpecular.x);
	if (m_pAmbientColorVariable) m_pAmbientColorVariable->SetFloatVector(&m_ColorAmbient.x);
	if (m_pFresnelColorVariable) m_pFresnelColorVariable->SetFloatVector(&m_ColorFresnel.x);

	if (m_pUseDiffuseTextureVariable) m_pUseDiffuseTextureVariable->SetBool(m_bDiffuseTexture);
	if (m_pUseSpecularLevelTextureVariable) m_pUseSpecularLevelTextureVariable->SetBool(m_bSpecularLevelTexture);
	if (m_pFlipGreenChannelVariable) m_pFlipGreenChannelVariable->SetBool(m_bFlipGreenChannel);
	if (m_pUseNormalMappingVariable) m_pUseNormalMappingVariable->SetBool(m_bNormalMapping);
	if (m_pUseEnvironmentMappingVariable) m_pUseEnvironmentMappingVariable->SetBool(m_bEnvironmentMapping);
	if (m_pUseOpacityMapVariable) m_pUseOpacityMapVariable->SetBool(m_bOpacityMap);
	if (m_pUseBlinnVariable) m_pUseBlinnVariable->SetBool(m_bSpecularBlinn);
	if (m_pUsePhongVariable) m_pUsePhongVariable->SetBool(m_bSpecularPhong);
	if (m_pUseFresnelFalloffVariable) m_pUseFresnelFalloffVariable->SetBool(m_bFresnelFaloff);

	if (m_pShininessVariable) m_pShininessVariable->SetInt(m_Shininess);

	if (m_pAmbientIntensityVariable) m_pAmbientIntensityVariable->SetFloat(m_AmbientIntensity);
	if (m_pReflectionStrengthVariable) m_pReflectionStrengthVariable->SetFloat(m_ReflectionStrength);
	if (m_pRefractionStrengthVariable) m_pRefractionStrengthVariable->SetFloat(m_RefractionStrength);
	if (m_pRefractionIndexVariable) m_pRefractionIndexVariable->SetFloat(m_RefractionIndex);
	if (m_pOpacityVariable) m_pOpacityVariable->SetFloat(m_Opacity);
	if (m_pFresnelPowerVariable) m_pFresnelPowerVariable->SetFloat(m_FresnelPower);
	if (m_pFresnelMultiplierVariable) m_pFresnelMultiplierVariable->SetFloat(m_FresnelMultiplier);
	if (m_pFresnelHardnessVariable) m_pFresnelHardnessVariable->SetFloat(m_FresnelHardness);

}

void UberMaterial::SetSpecularColor(XMFLOAT4 color)
{
	m_ColorSpecular = color;
}

void UberMaterial::EnableSpecularLevelTexture(bool enable)
{
	m_bSpecularLevelTexture = enable;
}

void UberMaterial::SetSpecularLevelTexture(const wstring& assetFile)
{
	m_pSpecularLevelTexture = ContentManager::Load<TextureData>(assetFile);
}

void UberMaterial::SetShininess(int shininess)
{
	m_Shininess = shininess;
}

void UberMaterial::SetAmbientColor(XMFLOAT4 color)
{
	m_ColorAmbient = color;
}

void UberMaterial::SetAmbientIntensity(float intensity)
{
	m_AmbientIntensity = intensity;
}

void UberMaterial::FlipNormalGreenCHannel(bool flip)
{
	m_bFlipGreenChannel = flip;
}

void UberMaterial::EnableNormalMapping(bool enable)
{
	m_bNormalMapping = enable;
}

void UberMaterial::SetNormalMapTexture(const wstring& assetFile)
{
	m_pNormalMappingTexture = ContentManager::Load<TextureData>(assetFile);
}

void UberMaterial::EnableEnvironmentMapping(bool enable)
{
	m_bEnvironmentMapping = enable;
}

void UberMaterial::SetEnvironmentCube(const wstring& assetFile)
{
	m_pEnvironmentCube = ContentManager::Load<TextureData>(assetFile);
}

void UberMaterial::SetReflectionStrength(float strength)
{
	m_ReflectionStrength = strength;
}

void UberMaterial::SetRefractionStrength(float strength)
{
	m_RefractionStrength = strength;
}

void UberMaterial::SetRefractionIndex(float index)
{
	m_RefractionIndex = index;
}

void UberMaterial::SetOpacity(float opacity)
{
	m_Opacity = opacity;
}

void UberMaterial::EnableOpacityMap(bool enable)
{
	m_bOpacityMap = enable;
}

void UberMaterial::SetOpacityTexture(const wstring& assetFile)
{
	m_pOpacityMap = ContentManager::Load<TextureData>(assetFile);
}

void UberMaterial::EnableSpecularBlinn(bool enable)
{
	m_bSpecularBlinn = enable;
}

void UberMaterial::EnableSpecularPhong(bool enable)
{
	m_bSpecularPhong = enable;
}

void UberMaterial::EnableFresnelFaloff(bool enable)
{
	m_bFresnelFaloff = enable;
}

void UberMaterial::SetFresnelColor(XMFLOAT4 color)
{
	m_ColorFresnel = color;
}
