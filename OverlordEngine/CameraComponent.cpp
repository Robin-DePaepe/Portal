#include "stdafx.h"
#include "CameraComponent.h"
#include "OverlordGame.h"
#include "TransformComponent.h"
#include "PhysxProxy.h"
#include "GameObject.h"
#include "GameScene.h"

using namespace DirectX;
using namespace physx;

CameraComponent::CameraComponent() :
	m_FarPlane(2500.0f),
	m_NearPlane(0.1f),
	m_FOV(DirectX::XM_PIDIV4),
	m_Size(25.0f),
	m_IsActive(true),
	m_PerspectiveProjection(true)
{
	XMStoreFloat4x4(&m_Projection, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&m_View, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewInverse, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjection, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjectionInverse, DirectX::XMMatrixIdentity());
}

void CameraComponent::Initialize(const GameContext&) {}

void CameraComponent::Update(const GameContext&)
{
	// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
	using namespace DirectX;

	const auto windowSettings = OverlordGame::GetGameSettings().Window;
	DirectX::XMMATRIX projection;

	if (m_PerspectiveProjection)
	{
		projection = DirectX::XMMatrixPerspectiveFovLH(m_FOV, windowSettings.AspectRatio, m_NearPlane, m_FarPlane);
	}
	else
	{
		const float viewWidth = (m_Size > 0) ? m_Size * windowSettings.AspectRatio : windowSettings.Width;
		const float viewHeight = (m_Size > 0) ? m_Size : windowSettings.Height;
		projection = DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, m_NearPlane, m_FarPlane);
	}

	const DirectX::XMVECTOR worldPosition = XMLoadFloat3(&GetTransform()->GetWorldPosition());
	const DirectX::XMVECTOR lookAt = XMLoadFloat3(&GetTransform()->GetForward());
	const DirectX::XMVECTOR upVec = XMLoadFloat3(&GetTransform()->GetUp());

	const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(worldPosition, worldPosition + lookAt, upVec);
	const DirectX::XMMATRIX viewInv = XMMatrixInverse(nullptr, view);
	const DirectX::XMMATRIX viewProjectionInv = XMMatrixInverse(nullptr, view * projection);

	XMStoreFloat4x4(&m_Projection, projection);
	XMStoreFloat4x4(&m_View, view);
	XMStoreFloat4x4(&m_ViewInverse, viewInv);
	XMStoreFloat4x4(&m_ViewProjection, view * projection);
	XMStoreFloat4x4(&m_ViewProjectionInverse, viewProjectionInv);
}

void CameraComponent::Draw(const GameContext&) {}

void CameraComponent::SetActive()
{
	auto gameObject = GetGameObject();
	if (gameObject == nullptr)
	{
		Logger::LogError(L"[CameraComponent] Failed to set active camera. Parent game object is null");
		return;
	}

	auto gameScene = gameObject->GetScene();
	if (gameScene == nullptr)
	{
		Logger::LogError(L"[CameraComponent] Failed to set active camera. Parent game scene is null");
		return;
	}

	gameScene->SetActiveCamera(this);
}

physx::PxRaycastBuffer CameraComponent::Pick(const GameContext& gameContext, CollisionGroupFlag ignoreGroups, const float maxDistance) const
{
	UNREFERENCED_PARAMETER(ignoreGroups);

	//get window settings
	const auto windowSettings = OverlordGame::GetGameSettings().Window;
	const float halfWidth{ windowSettings.Width / 2.f };
	const float halfHeight{ windowSettings.Height / 2.f };

	//define variables
	const POINT mousePos{ gameContext.pInput->GetMousePosition() };
	const PhysxProxy* pPhysxProxy{ GetGameObject()->GetScene()->GetPhysxProxy() };
	const XMMATRIX viewProjectionInv = XMLoadFloat4x4(&m_ViewProjectionInverse);

	//transform mouse to ndc
	XMFLOAT2 ndcPos{ (float(mousePos.x) - halfWidth) / halfWidth ,(halfHeight - float(mousePos.y)) / halfHeight };

	//transform ndc to world
	XMVECTOR nearPosV4{ ndcPos.x ,ndcPos.y,0.f,0.f };
	XMVECTOR farPosV4{ ndcPos.x ,ndcPos.y,1.f,0.f };

	nearPosV4 = XMVector3TransformCoord(nearPosV4, viewProjectionInv);
	farPosV4 = XMVector3TransformCoord(farPosV4, viewProjectionInv);

	//convert to pxvec3
	XMFLOAT4 nearPosF4{}, farPosF4;

	XMStoreFloat4(&nearPosF4, nearPosV4);
	XMStoreFloat4(&farPosF4, farPosV4);

	PxVec3 nearPosV3{ nearPosF4.x ,nearPosF4.y,nearPosF4.z };
	PxVec3 farPosV3{ farPosF4.x ,farPosF4.y,farPosF4.z };

	//calc dir
	PxVec3 dir{ farPosV3 - nearPosV3 };
	dir.normalize();

	//sent a raycast for objects
	PxQueryFilterData filterData;
	filterData.data.word0 = ~int(ignoreGroups);

	PxRaycastBuffer hit{};
	if (pPhysxProxy->Raycast(nearPosV3, dir, PX_MAX_F32, hit, PxHitFlag::eDEFAULT, filterData))
	{
		if (hit.block.distance <= maxDistance)
		{
			return hit;
		}
	}

	return nullptr;
}