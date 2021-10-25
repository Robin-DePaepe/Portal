#include "stdafx.h"
#include "CombineResources.h"
#include "RenderTarget.h"

CombineResources::CombineResources()
	: PostProcessingMaterial(L"./Resources/Effects/Post/CombineResources.fx", 10),
	m_pTextureMap1Variable(nullptr),
	m_pTextureMap2Variable(nullptr)
{
}

void CombineResources::SetSecondTexture(RenderTarget* pRendertarget)
{
	if (m_pTextureMap2Variable->IsValid())
	{
		m_pTextureMap2Variable->SetResource(pRendertarget->GetShaderResourceView());
	}
}

void CombineResources::LoadEffectVariables()
{
	if (!m_pTextureMap1Variable)
	{
		m_pTextureMap1Variable = GetEffect()->GetVariableByName("gTexture1")->AsShaderResource();

		if (!m_pTextureMap1Variable->IsValid())
		{
			Logger::LogWarning(L"CenterGlow::LoadEffectVariables() > \'gTexture1\' variable not found!");
			m_pTextureMap1Variable = nullptr;
		}
	}
	if (!m_pTextureMap2Variable)
	{
		m_pTextureMap2Variable = GetEffect()->GetVariableByName("gTexture2")->AsShaderResource();

		if (!m_pTextureMap2Variable->IsValid())
		{
			Logger::LogWarning(L"CenterGlow::LoadEffectVariables() > \'gTexture2\' variable not found!");
			m_pTextureMap2Variable = nullptr;
		}
	}
}

void CombineResources::UpdateEffectVariables(RenderTarget* pRendertarget)
{
	if (m_pTextureMap1Variable->IsValid())
	{
		m_pTextureMap1Variable->SetResource(pRendertarget->GetShaderResourceView());
	}
}