//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ShadowMapMaterial.h"
#include "ContentManager.h"

ShadowMapMaterial::~ShadowMapMaterial()
{
	for (UINT i{}; i < NUM_TYPES; ++i)
	{
		m_InputLayoutDescriptions[i].clear();
		m_pInputLayouts[i]->Release();
	}
}

void ShadowMapMaterial::Initialize(const GameContext& gameContext)
{
	if (!m_IsInitialized)
	{
		m_pShadowEffect = ContentManager::Load<ID3DX11Effect>(L"Resources/Effects/ShadowMapGenerator.fx");

		for (UINT i{}; i < NUM_TYPES; ++i)
		{
			if (!m_pShadowTechs[i])
				m_pShadowTechs[i] = m_pShadowEffect->GetTechniqueByIndex(i);

			EffectHelper::BuildInputLayout(gameContext.pDevice, m_pShadowTechs[i], &m_pInputLayouts[i], m_InputLayoutDescriptions[i], m_InputLayoutSizes[i], m_InputLayoutIds[i]);
		}

		if (!m_pWorldMatrixVariable)
		{
			m_pWorldMatrixVariable = m_pShadowEffect->GetVariableByName("gWorld")->AsMatrix();
			if (!m_pWorldMatrixVariable->IsValid())
			{
				Logger::LogWarning(L"ShadowMapMaterial::Initialize() > \'gWorld\' variable not found!");
				m_pWorldMatrixVariable = nullptr;
			}
		}
		if (!m_pLightVPMatrixVariable)
		{
			m_pLightVPMatrixVariable = m_pShadowEffect->GetVariableByName("gLightViewProj")->AsMatrix();
			if (!m_pLightVPMatrixVariable->IsValid())
			{
				Logger::LogWarning(L"ShadowMapMaterial::Initialize() > \'gLightViewProj\' variable not found!");
				m_pLightVPMatrixVariable = nullptr;
			}
		}
		if (!m_pBoneTransforms)
		{
			m_pBoneTransforms = m_pShadowEffect->GetVariableByName("gBones")->AsMatrix();
			if (!m_pBoneTransforms->IsValid())
			{
				Logger::LogWarning(L"ShadowMapMaterial::Initialize() > \'gBones\' variable not found!");
				m_pBoneTransforms = nullptr;
			}
		}
	}
}

void ShadowMapMaterial::SetLightVP(DirectX::XMFLOAT4X4 lightVP) const
{
	m_pLightVPMatrixVariable->SetMatrix(&lightVP.m[0][0]);
}

void ShadowMapMaterial::SetWorld(DirectX::XMFLOAT4X4 world) const
{
	m_pWorldMatrixVariable->SetMatrix(&world.m[0][0]);
}

void ShadowMapMaterial::SetBones(const float* pData, int count) const
{
	m_pBoneTransforms->SetMatrixArray(pData, 0, count);
}
