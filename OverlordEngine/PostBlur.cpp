#include "stdafx.h"
#include "PostBlur.h"
#include "RenderTarget.h"

PostBlur::PostBlur()
	: PostProcessingMaterial(L"./Resources/Effects/Post/Blur.fx", 2),
	m_pTextureMapVariable(nullptr),
	m_pPixelOffsetMapVariable(nullptr),
	m_BlurStrength{2}
{
}

void PostBlur::LoadEffectVariables()
{
	if (!m_pTextureMapVariable)
	{
		m_pTextureMapVariable = GetEffect()->GetVariableByName("gTexture")->AsShaderResource();

		if (!m_pTextureMapVariable->IsValid())
		{
			Logger::LogWarning(L"PostBlur::LoadEffectVariables() > \'gTexture\' variable not found!");
			m_pTextureMapVariable = nullptr;
		}
	}
	if (!m_pPixelOffsetMapVariable)
	{
		m_pPixelOffsetMapVariable = GetEffect()->GetVariableByName("gPixelOffset")->AsScalar();

		if (!m_pPixelOffsetMapVariable->IsValid())
		{
			Logger::LogWarning(L"PostBlur::LoadEffectVariables() > \'gPixelOffset\' variable not found!");
			m_pPixelOffsetMapVariable = nullptr;
		}
	}
}

void PostBlur::UpdateEffectVariables(RenderTarget* pRendertarget)
{
	if (m_pTextureMapVariable->IsValid())
	{
		m_pTextureMapVariable->SetResource(pRendertarget->GetShaderResourceView());
	}
	if (m_pPixelOffsetMapVariable->IsValid())
	{
		m_pPixelOffsetMapVariable->SetFloat(m_BlurStrength);
	}
}