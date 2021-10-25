#include "stdafx.h"
#include "PostProcessingMaterial.h"
#include "RenderTarget.h"
#include "OverlordGame.h"
#include "ContentManager.h"
#include "SceneManager.h"

PostProcessingMaterial::PostProcessingMaterial(std::wstring effectFile, unsigned int renderIndex,
	std::wstring technique)
	: m_IsInitialized(false),
	m_pInputLayout(nullptr),
	m_pInputLayoutSize(0),
	m_effectFile(std::move(effectFile)),
	m_InputLayoutID(0),
	m_RenderIndex(renderIndex),
	m_pRenderTarget(nullptr),
	m_pVertexBuffer(nullptr),
	m_pIndexBuffer(nullptr),
	m_NumVertices(0),
	m_NumIndices(0),
	m_VertexBufferStride(0),
	m_pEffect(nullptr),
	m_pTechnique(nullptr),
	m_TechniqueName(std::move(technique))
{
}

PostProcessingMaterial::~PostProcessingMaterial()
{
	SafeDelete(m_pRenderTarget);

	m_pInputLayout->Release();
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
}

void PostProcessingMaterial::Initialize(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);

	if (!m_IsInitialized)
	{
		//1. LoadEffect (LoadEffect(...))
		if (!LoadEffect(gameContext, m_effectFile))
		{
			Logger::LogError(L"Error loading effect in post processing material ");
		}
		//2. CreateInputLaytout (CreateInputLayout(...))
		EffectHelper::BuildInputLayout(gameContext.pDevice, m_pTechnique, &m_pInputLayout, m_pInputLayoutDescriptions, m_pInputLayoutSize, m_InputLayoutID);
		//   CreateVertexBuffer (CreateVertexBuffer(...)) > As a TriangleStrip (FullScreen Quad)
		CreateVertexBuffer(gameContext);
		CreateIndexBuffer(gameContext);

		//3. Create RenderTarget (m_pRenderTarget)
		//		Take a look at the class, figure out how to initialize/create a RenderTarget Object
		//		GameSettings > OverlordGame::GetGameSettings()
		auto gameSettings{ SceneManager::GetInstance()->GetGame()->GetGameSettings() };

		m_pRenderTarget = new RenderTarget(gameContext.pDevice);
		RENDERTARGET_DESC rtDesc;

		rtDesc.Width = gameSettings.Window.Width;
		rtDesc.Height = gameSettings.Window.Height;

		rtDesc.EnableColorBuffer = true;
		rtDesc.EnableColorSRV = true;
		rtDesc.EnableDepthBuffer = true;
		rtDesc.EnableDepthSRV = false;

		m_pRenderTarget->Create(rtDesc);

		m_IsInitialized = true;
	}
}

bool PostProcessingMaterial::LoadEffect(const GameContext& gameContext, const std::wstring& effectFile)
{
	UNREFERENCED_PARAMETER(gameContext);

	//TODO: complete
	//Load Effect through ContentManager
	m_pEffect = ContentManager::Load<ID3DX11Effect>(effectFile);
	//Check if m_TechniqueName (default constructor parameter) is set
	if (m_TechniqueName.empty())
	{
		m_pTechnique = m_pEffect->GetTechniqueByIndex(0);
	}
	else
	{
		m_pTechnique = m_pEffect->GetTechniqueByName((char*)m_TechniqueName.c_str());
	}

	LoadEffectVariables();

	return true;
}

void PostProcessingMaterial::Draw(const GameContext& gameContext, RenderTarget* previousRendertarget)
{

	//TODO: complete
	//1. Clear the object's RenderTarget (m_pRenderTarget) [Check RenderTarget Class]
	const FLOAT color[4]{};
	m_pRenderTarget->Clear(gameContext, color);
	//2. Call UpdateEffectVariables(...)
	UpdateEffectVariables(previousRendertarget);
	//3. Set InputLayout
	gameContext.pDeviceContext->IASetInputLayout(m_pInputLayout);
	//4. Set VertexBuffer
	UINT offset = 0;
	gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertexBufferStride, &offset);
	gameContext.pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//5. Set PrimitiveTopology (TRIANGLELIST)
	gameContext.pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//6. Draw 
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, gameContext.pDeviceContext);
		gameContext.pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}

	// Generate Mips
	gameContext.pDeviceContext->GenerateMips(m_pRenderTarget->GetShaderResourceView());
}

void PostProcessingMaterial::CreateVertexBuffer(const GameContext& gameContext)
{
	m_NumVertices = 4;

	if (m_pVertexBuffer)	SafeRelease(m_pVertexBuffer);

	//Create vertex array containing three elements in system memory
	//fill a buffer description to copy the vertexdata into graphics memory
	//create a ID3D10Buffer in graphics memory containing the vertex info
	std::vector<VertexPosTex> buffer;

	buffer.push_back(VertexPosTex(DirectX::XMFLOAT3{ -1.f, 1.f, 0.f }, DirectX::XMFLOAT2{ 0,0 }));//top left
	buffer.push_back(VertexPosTex(DirectX::XMFLOAT3{ -1.f, -1.f, 0.f }, DirectX::XMFLOAT2{ 0,1 }));//bottom left
	buffer.push_back(VertexPosTex(DirectX::XMFLOAT3{ 1.f, 1.f, 0.f }, DirectX::XMFLOAT2{ 1,0 }));//top right
	buffer.push_back(VertexPosTex(DirectX::XMFLOAT3{ 1.f,-1.f, 0.f }, DirectX::XMFLOAT2{ 1,1 }));//bottom right

	D3D11_BUFFER_DESC vertexBuffDesc;
	vertexBuffDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBuffDesc.ByteWidth = sizeof(VertexPosTex) * m_NumVertices;
	vertexBuffDesc.CPUAccessFlags = 0;
	vertexBuffDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	vertexBuffDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = buffer.data();
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;

	gameContext.pDevice->CreateBuffer(&vertexBuffDesc, &subresourceData, &m_pVertexBuffer);
	m_VertexBufferStride = sizeof(VertexPosTex);
}

void PostProcessingMaterial::CreateIndexBuffer(const GameContext& gameContext)
{
	m_NumIndices = 6;

	if (m_pIndexBuffer) 	SafeRelease(m_pIndexBuffer);

	std::vector<UINT> buffer{ 3, 1 , 0 , 0, 2, 3 };

	D3D11_BUFFER_DESC indexBufDesc;
	indexBufDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.ByteWidth = sizeof(UINT) * m_NumIndices;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	indexBufDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = buffer.data();
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;

	gameContext.pDevice->CreateBuffer(&indexBufDesc, &subresourceData, &m_pIndexBuffer);
}
