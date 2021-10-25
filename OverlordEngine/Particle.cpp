#include "stdafx.h"
#include "Particle.h"

// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
using namespace DirectX;

Particle::Particle(const ParticleEmitterSettings& emitterSettings):
	m_VertexInfo(ParticleVertex()),
	m_EmitterSettings(emitterSettings),
	m_IsActive(false),
	m_TotalEnergy(0),
	m_CurrentEnergy(0),
	m_SizeGrow(0),
	m_InitSize(0),
	m_RandomDir{0.f,0.f,0.f}
{}

void Particle::Update(const GameContext& gameContext)
{
	if (!m_IsActive) return;

	const float elapsedTime{ gameContext.pGameTime->GetElapsed() };

	m_CurrentEnergy -= elapsedTime;

	if (m_CurrentEnergy <= 0)
	{
		m_IsActive = false;
		return;
	}


	//update position
	XMVECTOR direction{};
	XMVECTOR position{ XMLoadFloat3(&m_VertexInfo.Position) };

	if (m_EmitterSettings.randomDirections) direction = XMLoadFloat3(&m_RandomDir);
	else direction = XMLoadFloat3(&m_EmitterSettings.Direction);

	direction = XMVector3Normalize(direction);

	position += direction * m_EmitterSettings.VelocityPower * elapsedTime;
	XMStoreFloat3(&m_VertexInfo.Position, position);

	//calculate PCP to interpolate the values 
	const float particleLifePercentage{ m_CurrentEnergy / m_TotalEnergy };

	//set alpha value
	m_VertexInfo.Color.w = particleLifePercentage * 2.f;

	//calc new size
	float difference = (m_InitSize * m_SizeGrow) - m_InitSize;
	m_VertexInfo.Size = m_InitSize + difference * (1 - particleLifePercentage);
}

void Particle::Init(XMFLOAT3 initPosition)
{
	//activate particle
	m_IsActive = true;

	//calculate random floats between intervals
	m_CurrentEnergy = m_TotalEnergy = randF(m_EmitterSettings.MinEnergy, m_EmitterSettings.MaxEnergy);
	m_VertexInfo.Size = m_InitSize = randF(m_EmitterSettings.MinSize, m_EmitterSettings.MaxSize);
	m_SizeGrow = randF(m_EmitterSettings.MinSizeGrow, m_EmitterSettings.MaxSizeGrow);
	m_VertexInfo.Rotation = randF(-XM_PI, XM_PI);

	//position initialization

	//generatre a random direction unit vector
	XMVECTOR randomDir{ 1,0,0 };
	XMVECTOR initPositionV{ XMLoadFloat3(&initPosition) };
	XMMATRIX randomRotationMatrix{ XMMatrixRotationRollPitchYaw(randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI)) };

	randomDir = XMVector3TransformNormal(randomDir, randomRotationMatrix);

	//calc distance
	const float distance = randF(m_EmitterSettings.MinEmitterRange, m_EmitterSettings.MaxEmitterRange);

	//calc position
	initPositionV += randomDir * distance;

	//set position
	XMStoreFloat3(&m_VertexInfo.Position, initPositionV);

	//set the new color
	m_VertexInfo.Color = m_EmitterSettings.Color;

	//direction
	if (m_EmitterSettings.randomDirections)
	{
		m_RandomDir.x = randF(-1.f, 1.f);
		m_RandomDir.y = randF(-1.f, 1.f);
		m_RandomDir.z = randF(-1.f, 1.f);
	}
}
