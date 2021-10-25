#include "stdafx.h"
#include "ParticleEmitterComponent.h"
#include <utility>
#include "EffectHelper.h"
#include "ContentManager.h"
#include "TextureDataLoader.h"
#include "Particle.h"
#include "TransformComponent.h"

ParticleEmitterComponent::ParticleEmitterComponent(std::wstring  assetFile, int particleCount) :
	m_pVertexBuffer(nullptr),
	m_pEffect(nullptr),
	m_pParticleTexture(nullptr),
	m_pInputLayout(nullptr),
	m_pInputLayoutSize(0),
	m_Settings(ParticleEmitterSettings()),
	m_ParticleCount(particleCount),
	m_ActiveParticles(0),
	m_LastParticleInit(0.0f),
	m_AssetFile(std::move(assetFile)),
	m_HasSpawnedBurst{false}
{
	for (int i = 0; i < m_ParticleCount; i++)
	{
		m_Particles.push_back(new Particle(m_Settings));
	}
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	for (Particle* pParticle : m_Particles)
	{
		delete pParticle;
	}
	m_Particles.clear();

	m_pInputLayout->Release();
	m_pVertexBuffer->Release();
}

void ParticleEmitterComponent::Initialize(const GameContext& gameContext)
{
	LoadEffect(gameContext);
	CreateVertexBuffer(gameContext);

	m_pParticleTexture = ContentManager::Load<TextureData>(m_AssetFile);

	if (m_Settings.startParticleBurst > m_ParticleCount) Logger::LogError(L"Start particle burst is larger then the particle count! Can't spawn enough particles.");
}

void ParticleEmitterComponent::LoadEffect(const GameContext& gameContext)
{
	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"./Resources/Effects/ParticleRenderer.fx");

	m_pDefaultTechnique = m_pEffect->GetTechniqueByIndex(0);

	m_pWvpVariable = m_pEffect->GetVariableBySemantic("WorldViewProjection")->AsMatrix();

	if (!m_pWvpVariable->IsValid())
	{
		Logger::LogError(L"ParticleRenderer::Initialize() > Shader variable \'WorldViewProjection\' not valid!");
		return;
	}

	m_pViewInverseVariable = m_pEffect->GetVariableBySemantic("ViewInverse")->AsMatrix();
	if (!m_pViewInverseVariable->IsValid())
	{
		Logger::LogError(L"ParticleRenderer::Initialize() > Shader variable \'ViewInverse\' not valid!");
		return;
	}

	m_pTextureVariable = m_pEffect->GetVariableByName("gParticleTexture")->AsShaderResource();
	if (!m_pTextureVariable->IsValid())
	{
		Logger::LogError(L"ParticleRenderer::Initialize() > Shader variable \'gParticleTexture\' not valid!");
		return;
	}

	EffectHelper::BuildInputLayout(gameContext.pDevice, m_pDefaultTechnique, &m_pInputLayout, m_pInputLayoutSize);
}

void ParticleEmitterComponent::CreateVertexBuffer(const GameContext& gameContext)
{
	if (m_pVertexBuffer)	SafeRelease(m_pVertexBuffer);

	//create description
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(ParticleVertex) * m_ParticleCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;

	HRESULT result = gameContext.pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pVertexBuffer);

	if (result != S_OK)
	{
		Logger::LogError(L"Creating buffer in particle emitter failed");
	}
}

void ParticleEmitterComponent::Update(const GameContext& gameContext)
{
	float particleInterval = ((m_Settings.MinEnergy + m_Settings.MaxEnergy) / 2.f) / m_ParticleCount;

	m_LastParticleInit += gameContext.pGameTime->GetElapsed();

	m_ActiveParticles = 0;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	mappedResource.pData = &m_pVertexBuffer;
	gameContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	ParticleVertex* pBuffer = (ParticleVertex*)mappedResource.pData;

	for (Particle* pParticle : m_Particles)
	{
		pParticle->Update(gameContext);

		if (pParticle->IsActive())
		{
			pBuffer[m_ActiveParticles] = pParticle->GetVertexInfo();
			++m_ActiveParticles;
		}
		else if (m_LastParticleInit >= particleInterval)
		{
			pParticle->Init(GetTransform()->GetWorldPosition());
			pBuffer[m_ActiveParticles] = pParticle->GetVertexInfo();
			++m_ActiveParticles;
			m_LastParticleInit = 0;
		}
		if (!m_HasSpawnedBurst)
		{
			m_HasSpawnedBurst = true;
		}
	}
			for (int i = 0; i < m_Settings.startParticleBurst; i++)
			{
				m_Particles[i]->Init(GetTransform()->GetWorldPosition());
				pBuffer[m_ActiveParticles] = m_Particles[i]->GetVertexInfo();
				++m_ActiveParticles;
				m_LastParticleInit = 0;
			}
	gameContext.pDeviceContext->Unmap(m_pVertexBuffer, 0);
}

void ParticleEmitterComponent::Draw(const GameContext&)
{}

void ParticleEmitterComponent::PostDraw(const GameContext& gameContext)
{
	//set shader variables
	if (m_pWvpVariable) m_pWvpVariable->SetMatrix(&gameContext.pCamera->GetViewProjection().m[0][0]);
	if (m_pViewInverseVariable) m_pViewInverseVariable->SetMatrix(&gameContext.pCamera->GetViewInverse().m[0][0]);
	if (m_pTextureVariable) m_pTextureVariable->SetResource(m_pParticleTexture->GetShaderResourceView());

	//set device context
	gameContext.pDeviceContext->IASetInputLayout(m_pInputLayout);
	gameContext.pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	unsigned int offset = 0;
	unsigned int stride = sizeof(ParticleVertex);
	gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//loop over the passes
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pDefaultTechnique->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pDefaultTechnique->GetPassByIndex(p)->Apply(0, gameContext.pDeviceContext);
		gameContext.pDeviceContext->Draw(m_ActiveParticles, 0);
	}
}
