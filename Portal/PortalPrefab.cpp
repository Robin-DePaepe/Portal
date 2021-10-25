#include "stdafx.h"
#include "PortalPrefab.h"
#include "Components.h"
#include "PortalMaterial.h"
#include "PhysxManager.h"
#include "FixedCamera.h"
#include <RenderTarget.h>
#include "SceneManager.h"
#include "OverlordGame.h"
#include "GameScene.h"
#include "PortalCharacterController.h"
#include "GameScene.h"
#include "PhysxProxy.h"
#include "WallPrefab.h"

using namespace DirectX;

const float PortalPrefab::m_BoxColliderDefaultSize{ 5.f }; //5 corresponds perfectly to a unitmesh scale of 1
const float PortalPrefab::m_HeightMultiplier{ 1.5f };
bool PortalPrefab::m_BluePortalLeft{ false };
bool PortalPrefab::m_BluePortalEntered{ false };
bool PortalPrefab::m_RedPortalLeft{ false };
bool PortalPrefab::m_PlayerInBluePortal{ false };
bool PortalPrefab::m_RedPortalEntered{ false };
bool PortalPrefab::m_PlayerInRedPortal{ false };

PortalPrefab::PortalPrefab(bool isBlue, const DirectX::XMFLOAT3& position, GameObject* pWall, PortalPrefab** pOtherPortal, PortalCharacterController* pPlayer, const float portalSize)
	:GameObject{},
	m_pPortalSize{ portalSize },
	m_IsBlue{ isBlue },
	m_pWall{ pWall },
	m_pPortalCameraObj{ nullptr },
	m_pOtherPortal{ pOtherPortal },
	m_pPortalMat{ nullptr },
	m_pPortalRT{ nullptr },
	m_pPlayer{ pPlayer }
{
	//setup transform 
	GetTransform()->Translate(position);
	DirectX::XMVECTOR rotationV{ DirectX::XMLoadFloat4(&pWall->GetTransform()->GetWorldRotation()) };
	GetTransform()->Rotate(rotationV, true);
	GetTransform()->Scale(portalSize, 0.1f, portalSize* m_HeightMultiplier);
}

PortalPrefab::~PortalPrefab()
{
	SafeDelete(m_pPortalRT);

	if ((m_PlayerInBluePortal && m_IsBlue) || (m_PlayerInRedPortal && !m_IsBlue)) reinterpret_cast<WallPrefab*>(m_pWall)->AddRigidToScene();
}

bool PortalPrefab::IsActive()
{
	bool result = *m_pOtherPortal != nullptr;
	m_pPortalMat->SetPortalActive(result);

	return result;
}

void PortalPrefab::BeginPortalRendering(const GameContext& gameContext)
{
	ID3D11ShaderResourceView* const pSRV[] = { nullptr };
	gameContext.pDeviceContext->PSSetShaderResources(1, 1, pSRV);

	SceneManager::GetInstance()->GetGame()->SetRenderTarget(m_pPortalRT);
	const FLOAT color[4]{};
	m_pPortalRT->Clear(gameContext, color);

	GetScene()->SetActiveCamera((*m_pOtherPortal)->GetCamera()->GetComponent<CameraComponent>(true));

	m_pPlayer->GetComponent<ModelComponent>(true)->SetIsActive(true);
}

void PortalPrefab::EndPortalRendering()
{
	SceneManager::GetInstance()->GetGame()->SetRenderTarget(nullptr);
	GetScene()->SetActiveCamera(nullptr);

	m_pPortalMat->SetPortalResourceView(m_pPortalRT->GetShaderResourceView());

	if (!m_pPlayer->IsDancing()) m_pPlayer->GetComponent<ModelComponent>(true)->SetIsActive(false);
}

void PortalPrefab::Initialize(const GameContext& gameContext)
{
	//we need a parent for the rotations because you can't set the rotation on an object with rigidbody
	GameObject* childForComponents{ new GameObject{} };
	AddChild(childForComponents);

	//setup mesh and material
	ModelComponent* pModelPortal = new ModelComponent{ L"Resources/Meshes/UnitPlane.ovm" };
	childForComponents->AddComponent(pModelPortal);

	m_pPortalMat = new PortalMaterial{ m_IsBlue };
	gameContext.pMaterialManager->AddMaterial(m_pPortalMat, gameContext.pMaterialManager->GetMaterialId());

	pModelPortal->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();

	//setup collision trigger
	auto pRigid{ new RigidBodyComponent{  } };
	pRigid->SetKinematic(true);
	childForComponents->AddComponent(pRigid);

	std::shared_ptr<physx::PxGeometry> sp_CubeGeometry(new physx::PxBoxGeometry{ m_pPortalSize * m_BoxColliderDefaultSize,2.f,m_pPortalSize * m_BoxColliderDefaultSize * m_HeightMultiplier });

	physx::PxTransform portalPose = physx::PxTransform::createIdentity();
	portalPose.p = ToPxVec3(GetTransform()->GetPosition());
	portalPose.q = ToPxQuat(GetTransform()->GetRotation());

	auto pMat{ PhysxManager::GetInstance()->GetPhysics()->createMaterial(0.02f, 0.6f, 0.95f) };
	auto colliderPortal{ new ColliderComponent(sp_CubeGeometry, *pMat, portalPose) };
	colliderPortal->EnableTrigger(true);
	childForComponents->AddComponent(colliderPortal);

	childForComponents->SetOnTriggerCallBack([this](GameObject* triggerObject, GameObject* otherObject, GameObject::TriggerAction action)
		{
			UNREFERENCED_PARAMETER(triggerObject);

			if (otherObject->GetTag() == L"Player")
			{
				if (action == GameObject::TriggerAction::ENTER)
				{
					if (m_IsBlue)	m_BluePortalEntered = true;
					else		m_RedPortalEntered = true;
				}
				else	if (action == GameObject::TriggerAction::LEAVE)
				{
					if (m_IsBlue)	m_BluePortalLeft = true;
					else m_RedPortalLeft = true;
				}
			}
		}
	);

	//camera
	m_pPortalCameraObj = { new GameObject{} };
	FixedCamera* pFixedCam = { new FixedCamera{  } };

	m_pPortalCameraObj->AddChild(pFixedCam);
	GetScene()->AddChild(m_pPortalCameraObj);

	m_pPortalCameraObj->GetTransform()->Rotate(0.f, 0.f, 180.f);

	//render target
	m_pPortalRT = new RenderTarget{ gameContext.pDevice };

	RENDERTARGET_DESC rtDesc;
	rtDesc.Width = SceneManager::GetInstance()->GetGame()->GetGameSettings().Window.Width;
	rtDesc.Height = SceneManager::GetInstance()->GetGame()->GetGameSettings().Window.Height;
	rtDesc.EnableColorBuffer = true;
	rtDesc.EnableColorSRV = true;
	rtDesc.EnableDepthBuffer = true;
	rtDesc.EnableDepthSRV = true;
	rtDesc.GenerateMipMaps_Color = true;

	rtDesc.ColorBufferSupplied = false;
	rtDesc.DepthBufferSupplied = false;

	m_pPortalRT->Create(rtDesc);
}

void PortalPrefab::Update(const GameContext&)
{
	if (*m_pOtherPortal == nullptr) return;

#pragma region UpdateWallCollsions

	//update the collison of the wall properly
	if (m_BluePortalEntered && m_IsBlue)
	{
		m_BluePortalEntered = false;
		m_PlayerInBluePortal = true;

		if (m_pWall != (*m_pOtherPortal)->GetWall() || !m_PlayerInRedPortal)reinterpret_cast<WallPrefab*>(m_pWall)->RemoveRigidFromScene();
	}
	else	if (m_RedPortalEntered && !m_IsBlue)
	{
		m_RedPortalEntered = false;
		m_PlayerInRedPortal = true;

		if (m_pWall != (*m_pOtherPortal)->GetWall() || !m_PlayerInBluePortal) reinterpret_cast<WallPrefab*>(m_pWall)->RemoveRigidFromScene();
	}
	if (m_BluePortalLeft && m_IsBlue)
	{
		m_BluePortalLeft = false;
		m_PlayerInBluePortal = false;

		if (m_pWall != (*m_pOtherPortal)->GetWall() || !m_PlayerInRedPortal) 	reinterpret_cast<WallPrefab*>(m_pWall)->AddRigidToScene();
	}
	if (m_RedPortalLeft && !m_IsBlue)
	{
		m_RedPortalLeft = false;
		m_PlayerInRedPortal = false;

		if (m_pWall != (*m_pOtherPortal)->GetWall() || !m_PlayerInBluePortal)reinterpret_cast<WallPrefab*>(m_pWall)->AddRigidToScene();
	}

#pragma endregion

	float anglePortals{};
#pragma region Teleportaton

	//check if the player needs to teleport
	if ((m_PlayerInBluePortal && m_IsBlue) || (m_PlayerInRedPortal && !m_IsBlue))
	{
		const float dotResult{ CalculateDotWithPlayer() };
		const float dotCutoff{- 0.1f };

		if (dotResult >= dotCutoff)
		{
			const XMVECTOR& centerOtherPortal{ XMLoadFloat3(&(*m_pOtherPortal)->GetTransform()->GetWorldPosition()) };
			const XMVECTOR& forwardOtherPortal{ XMVector3Normalize(XMLoadFloat3(&(*m_pOtherPortal)->GetTransform()->GetUp()))};
			m_pPlayer->GetTransform()->Translate(centerOtherPortal + (forwardOtherPortal * (-dotCutoff + 0.001f)));

			//rotation
			XMVECTOR otherPortalRightV{ XMLoadFloat3(&(*m_pOtherPortal)->GetTransform()->GetRight()) };
			XMVECTOR portalRightV{ XMLoadFloat3(&GetTransform()->GetRight()) };

			XMFLOAT3 dotResultAngle{}, lengthRight{}, lengthOtherRight{};

			XMStoreFloat3(&dotResultAngle, XMVector3Dot(portalRightV, otherPortalRightV));
			XMStoreFloat3(&lengthRight, XMVector3Length(portalRightV));
			XMStoreFloat3(&lengthOtherRight, XMVector3Length(otherPortalRightV));

			anglePortals  = XMConvertToDegrees(acos(dotResultAngle.x / (lengthOtherRight.x * lengthRight.x)));
			 m_pPlayer->AddRotation(0.f, anglePortals - 180.f);
		}
	}
#pragma endregion

#pragma region Camera

	//update camera transform
	XMVECTOR playerPosV{ XMLoadFloat3(&m_pPlayer->GetCamera()->GetTransform()->GetWorldPosition()) };
	XMVECTOR otherPortalPosV{ XMLoadFloat3(&(*m_pOtherPortal)->GetTransform()->GetWorldPosition()) };
	XMVECTOR portalPosV{ XMLoadFloat3(&GetTransform()->GetWorldPosition()) };

	XMVECTOR portalToPlayerV{ playerPosV - otherPortalPosV };

	//portalToPlayerV = XMVector3Rotate(portalToPlayerV, XMQuaternionRotationRollPitchYaw(0.f, anglePortals, 0.f)); //failed rotation attempt

	XMVECTOR cameraPosV{ portalPosV - portalToPlayerV };
	m_pPortalCameraObj->GetTransform()->Translate(cameraPosV);

	//FAILED ATTEMPT WITH MATRICES*/
	/*XMMATRIX playerMatrix{ XMLoadFloat4x4(&m_pPlayer->GetCamera()->GetTransform()->GetWorld()) };
	XMMATRIX portalMatrix{ XMLoadFloat4x4(&GetTransform()->GetWorld()) };
	XMMATRIX otherPortalMatrix{ XMMatrixInverse(nullptr,XMLoadFloat4x4(&(*m_pOtherPortal)->GetTransform()->GetWorld())) };

	auto rotationAdjustmentM = XMMatrixScaling(1.f, 1.f, 1.f) * XMMatrixRotationRollPitchYaw(0.f, 0.f, XM_PI) * XMMatrixTranslation(0.f, 0.f, 0.f);

	XMVECTOR translationV{}, rotationQ{}, scaleV{};

	XMMATRIX transformMatrix{ playerMatrix * otherPortalMatrix * rotationAdjustmentM * portalMatrix };

	XMMatrixDecompose(&scaleV, &rotationQ, &translationV, transformMatrix);
			
	m_pPortalCameraObj->GetTransform()->Translate(translationV);
	m_pPortalCameraObj->GetTransform()->Rotate(rotationQ);*/
#pragma endregion

}

float PortalPrefab::CalculateDotWithPlayer() const
{
	const XMVECTOR& portalUp{ XMLoadFloat3(&GetTransform()->GetUp()) };
	const XMVECTOR& centerPortal{ XMLoadFloat3(&GetTransform()->GetWorldPosition()) };
	const XMVECTOR& centerPlayer{ XMLoadFloat3(&m_pPlayer->GetTransform()->GetWorldPosition()) };

	XMVECTOR diff = centerPortal - centerPlayer;
	XMFLOAT3 dot{};

	XMStoreFloat3(&dot, XMVector3Dot(diff, portalUp));

	return dot.x;
}

