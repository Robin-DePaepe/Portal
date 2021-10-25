#include "stdafx.h"
#include "ShadowMapRenderer.h"
#include "ContentManager.h"
#include "ShadowMapMaterial.h"
#include "RenderTarget.h"
#include "MeshFilter.h"
#include "SceneManager.h"
#include "OverlordGame.h"

ShadowMapRenderer::~ShadowMapRenderer()
{
	SafeDelete(m_pShadowRT);
	SafeDelete(m_pShadowMat);
}

void ShadowMapRenderer::Initialize(const GameContext& gameContext)
{
	if (m_IsInitialized)
		return;

	m_pShadowMat = new ShadowMapMaterial{};
	m_pShadowMat->Initialize(gameContext);
	m_pShadowRT = new RenderTarget{ gameContext.pDevice };
	RENDERTARGET_DESC rtDesc;
	rtDesc.Width = SceneManager::GetInstance()->GetGame()->GetGameSettings().Window.Width;
	rtDesc.Height = SceneManager::GetInstance()->GetGame()->GetGameSettings().Window.Height;
	rtDesc.EnableColorBuffer = false;
	rtDesc.EnableColorSRV = false;
	rtDesc.EnableDepthBuffer = true;
	rtDesc.EnableDepthSRV = true;
	rtDesc.GenerateMipMaps_Color = false;
	rtDesc.ColorBufferSupplied = false;
	rtDesc.DepthBufferSupplied = true;
	m_pShadowRT->Create(rtDesc);

	m_IsInitialized = true;
}

void ShadowMapRenderer::SetLight(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 direction)
{
	m_LightPosition = position;
	m_LightDirection = direction;
	const DirectX::XMVECTOR pos{ DirectX::XMLoadFloat3(&position) };
	const DirectX::XMFLOAT3 targetVector{ position.x + direction.x, position.y + direction.y, position.z + direction.z };
	const DirectX::XMVECTOR target{ DirectX::XMLoadFloat3(&targetVector) };
	DirectX::XMVector3Normalize(target);
	const DirectX::XMFLOAT3 upVector{ 0.0f, 1.0f, 0.0f };
	const DirectX::XMVECTOR up{ DirectX::XMLoadFloat3(&upVector) };

	const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
	const DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicLH(m_Size, m_Size, 0.1f, 500.f);
	DirectX::XMStoreFloat4x4(&m_LightVP, DirectX::XMMatrixMultiply(view, projection));
}

void ShadowMapRenderer::Begin(const GameContext& gameContext) const
{
	ID3D11ShaderResourceView* const pSRV[] = { nullptr };
	gameContext.pDeviceContext->PSSetShaderResources(1, 1, pSRV);

	SceneManager::GetInstance()->GetGame()->SetRenderTarget(m_pShadowRT);
	const FLOAT color[4]{};
	m_pShadowRT->Clear(gameContext, color);
	m_pShadowMat->SetLightVP(m_LightVP);
}

void ShadowMapRenderer::End(const GameContext& gameContext) const
{
	UNREFERENCED_PARAMETER(gameContext);
	SceneManager::GetInstance()->GetGame()->SetRenderTarget(nullptr);
}


void ShadowMapRenderer::Draw(const GameContext& gameContext, MeshFilter* pMeshFilter, DirectX::XMFLOAT4X4 world, const std::vector<DirectX::XMFLOAT4X4>& bones) const
{
	if (!m_pShadowMat)
	{
		Logger::LogWarning(L"ShadowMapRenderer::Draw() > No ShadowMaterial!");
		return;
	}

	//TODO: update shader variables in material
	m_pShadowMat->SetWorld(world);
	m_pShadowMat->SetLightVP(m_LightVP);
	ShadowMapMaterial::ShadowGenType type{ ShadowMapMaterial::ShadowGenType::Static };

	if (pMeshFilter->m_HasAnimations)
	{
		type = ShadowMapMaterial::ShadowGenType::Skinned;
		m_pShadowMat->SetBones(&bones.front().m[0][0], bones.size());
	}

	//Set Inputlayout
	gameContext.pDeviceContext->IASetInputLayout(m_pShadowMat->m_pInputLayouts[type]);

	//Set Vertex Buffer
	UINT offset = 0;
	auto vertexBufferData = pMeshFilter->GetVertexBufferData(gameContext, m_pShadowMat->m_InputLayoutIds[type]);
	gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &vertexBufferData.pVertexBuffer, &vertexBufferData.VertexStride,
		&offset);

	//Set Index Buffer
	gameContext.pDeviceContext->IASetIndexBuffer(pMeshFilter->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set Primitive Topology
	gameContext.pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//TODO: invoke draw call
	//DRAW
	ID3DX11EffectTechnique* tech = m_pShadowMat->m_pShadowTechs[type];
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, gameContext.pDeviceContext);
		gameContext.pDeviceContext->DrawIndexed(pMeshFilter->m_IndexCount, 0, 0);
	}
}

void ShadowMapRenderer::UpdateMeshFilter(const GameContext& gameContext, MeshFilter* pMeshFilter)
{
	ShadowMapMaterial::ShadowGenType type{ ShadowMapMaterial::ShadowGenType::Static };
	if (pMeshFilter->m_HasAnimations)
		type = ShadowMapMaterial::ShadowGenType::Skinned;
	pMeshFilter->BuildVertexBuffer(gameContext, m_pShadowMat->m_InputLayoutIds[type], m_pShadowMat->m_InputLayoutSizes[type], m_pShadowMat->m_InputLayoutDescriptions[type]);
}

ID3D11ShaderResourceView* ShadowMapRenderer::GetShadowMap() const
{
	return m_pShadowRT->GetDepthShaderResourceView();

}
