#include "stdafx.h"
#include "PostGrayScale.h"
#include "RenderTarget.h"

PostGrayscale::PostGrayscale()
	: PostProcessingMaterial(L"./Resources/Effects/Post/Grayscale.fx", 1),
	m_pTextureMapVariable(nullptr)
{
}

void PostGrayscale::LoadEffectVariables()
{
	if (!m_pTextureMapVariable)
	{
		m_pTextureMapVariable = GetEffect()->GetVariableByName("gTexture")->AsShaderResource();

		if (!m_pTextureMapVariable->IsValid())
		{
			Logger::LogWarning(L"PostGrayscale::LoadEffectVariables() > \'gTexture\' variable not found!");
			m_pTextureMapVariable = nullptr;
		}
	}
}

void PostGrayscale::UpdateEffectVariables(RenderTarget* pRendertarget)
{
	if (m_pTextureMapVariable->IsValid())
	{
		m_pTextureMapVariable->SetResource(pRendertarget->GetShaderResourceView());
	}
}