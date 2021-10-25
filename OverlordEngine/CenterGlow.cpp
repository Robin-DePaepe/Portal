#include "stdafx.h"
#include "CenterGlow.h"
#include "RenderTarget.h"

CenterGlow::CenterGlow()
	: PostProcessingMaterial(L"./Resources/Effects/Post/CenterGlow.fx", 0),
	m_pTextureMapVariable(nullptr),
	m_GlowSizeRatio(0.f),
	m_GlowOpacity(0.f),
	m_pGlowSizeRatio(nullptr),
	m_pGlowOpacity(nullptr),
	m_pUseCubeShape(nullptr),
	m_PulseRate{ 0.2f },
	m_MaxOpacity{ 0.3f },
	m_UseCubeShape{ false },
	m_PulseIncreasing{ true }
{
}

void CenterGlow::SetGlowSizeRatio(const float ratio)
{
	m_GlowSizeRatio = ratio;
	m_GlowOpacity = 0.f;
}

void CenterGlow::UseCubeShape(bool value)
{
	m_UseCubeShape = value;
	m_GlowOpacity = 0.f;
}

void CenterGlow::LoadEffectVariables()
{
	if (!m_pTextureMapVariable)
	{
		m_pTextureMapVariable = GetEffect()->GetVariableByName("gTexture")->AsShaderResource();

		if (!m_pTextureMapVariable->IsValid())
		{
			Logger::LogWarning(L"CenterGlow::LoadEffectVariables() > \'gTexture\' variable not found!");
			m_pTextureMapVariable = nullptr;
		}
	}
	if (!m_pGlowSizeRatio)
	{
		m_pGlowSizeRatio = GetEffect()->GetVariableByName("gGlowSizeRatio")->AsScalar();

		if (!m_pGlowSizeRatio->IsValid())
		{
			Logger::LogWarning(L"CenterGlow::LoadEffectVariables() > \'gGlowSizeRatio\' variable not found!");
			m_pGlowSizeRatio = nullptr;
		}
	}
	if (!m_pGlowOpacity)
	{
		m_pGlowOpacity = GetEffect()->GetVariableByName("gOpacity")->AsScalar();

		if (!m_pGlowOpacity->IsValid())
		{
			Logger::LogWarning(L"CenterGlow::LoadEffectVariables() > \'gOpacity\' variable not found!");
			m_pGlowOpacity = nullptr;
		}
	}
	if (!m_pUseCubeShape)
	{
		m_pUseCubeShape = GetEffect()->GetVariableByName("gUseCube")->AsScalar();

		if (!m_pUseCubeShape->IsValid())
		{
			Logger::LogWarning(L"CenterGlow::LoadEffectVariables() > \'gUseCube\' variable not found!");
			m_pUseCubeShape = nullptr;
		}
	}
}

void CenterGlow::UpdateEffectVariables(RenderTarget* pRendertarget)
{
	if (m_pTextureMapVariable->IsValid())
	{
		m_pTextureMapVariable->SetResource(pRendertarget->GetShaderResourceView());
	}
	if (m_pGlowSizeRatio->IsValid())
	{
		m_pGlowSizeRatio->SetFloat(m_GlowSizeRatio);
	}
	if (m_pGlowOpacity->IsValid())
	{
		m_pGlowOpacity->SetFloat(m_GlowOpacity);
	}
	if (m_pUseCubeShape->IsValid())
	{
		m_pUseCubeShape->SetBool(m_UseCubeShape);
	}
}

void CenterGlow::Update(const GameContext& gameContex)
{
	if (m_PulseIncreasing) m_GlowOpacity += m_PulseRate * gameContex.pGameTime->GetElapsed();
	else m_GlowOpacity -= m_PulseRate * gameContex.pGameTime->GetElapsed();

	if (m_GlowOpacity > m_MaxOpacity) m_PulseIncreasing = !m_PulseIncreasing;
	else if (m_GlowOpacity <= 0.f) m_PulseIncreasing = !m_PulseIncreasing;
}
