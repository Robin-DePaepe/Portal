#include "stdafx.h"
#include "PortalMaterial.h"
#include "SceneManager.h"
#include "OverlordGame.h"
#include "RenderTarget.h"

ID3DX11EffectShaderResourceVariable* PortalMaterial::m_pTextureMapVariable = nullptr;
ID3DX11EffectScalarVariable* PortalMaterial::m_pIsBlue = nullptr;
ID3DX11EffectScalarVariable* PortalMaterial::m_pIsActive = nullptr;

PortalMaterial::PortalMaterial(bool isBlue)
	: Material(L"./Resources/Effects/Portal.fx"),
	m_IsBlue{ isBlue },
	m_IsActive{ false }
{}

void PortalMaterial::SetPortalResourceView(ID3D11ShaderResourceView* view)
{
	if (m_pTextureMapVariable)
	{
		m_pTextureMapVariable->SetResource(view);
	}
}


void PortalMaterial::LoadEffectVariables()
{
	if (!m_pTextureMapVariable)
	{
		m_pTextureMapVariable = GetEffect()->GetVariableByName("gPortalTexture")->AsShaderResource();
		if (!m_pTextureMapVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial_Shadow::LoadEffectVariables() > \'gPortalTexture\' variable not found!");
			m_pTextureMapVariable = nullptr;
		}
	}
	if (!m_pIsBlue)
	{
		m_pIsBlue = GetEffect()->GetVariableByName("gIsBlue")->AsScalar();

		if (!m_pIsBlue->IsValid())
		{
			Logger::LogWarning(L"CenterGlow::LoadEffectVariables() > \'gIsBlue\' variable not found!");
			m_pIsBlue = nullptr;
		}
	}
	if (!m_pIsActive)
	{
		m_pIsActive = GetEffect()->GetVariableByName("gIsActive")->AsScalar();

		if (!m_pIsActive->IsValid())
		{
			Logger::LogWarning(L"CenterGlow::LoadEffectVariables() > \'gIsActive\' variable not found!");
			m_pIsActive = nullptr;
		}
	}
}


void PortalMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	UNREFERENCED_PARAMETER(gameContext);
	UNREFERENCED_PARAMETER(pModelComponent);

	if (m_pIsBlue->IsValid())
	{
		m_pIsBlue->SetBool(m_IsBlue);
	}	
	
	if (m_pIsActive->IsValid())
	{
		m_pIsActive->SetBool(m_IsActive);
	}
}
