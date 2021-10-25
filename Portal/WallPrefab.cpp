#include "stdafx.h"
#include "WallPrefab.h"
#include  "Components.h"
#include "DiffuseMaterial_Shadow.h"
#include "PhysxManager.h"
#include "GameScene.h"
#include "PhysxProxy.h"

WallPrefab::WallPrefab(const DirectX::XMVECTOR& rotation, const DirectX::XMFLOAT3& wallPos, const physx::PxVec3& wallSize, const float textureScale, bool isPortalWall)
	:m_TextureScale{ textureScale },
	m_WallSize{ wallSize },
	m_IsPortalWall{ isPortalWall }
{
	GetTransform()->Translate(wallPos);
	GetTransform()->Rotate(rotation, true);
}

void WallPrefab::RemoveRigidFromScene()
{
	GetScene()->GetPhysxProxy()->GetPhysxScene()->removeActor(*GetComponent<RigidBodyComponent>(true)->GetPxRigidActor(), false);
}

void WallPrefab::AddRigidToScene()
{
	GetScene()->GetPhysxProxy()->GetPhysxScene()->addActor(*GetComponent<RigidBodyComponent>(true)->GetPxRigidActor());
}


void WallPrefab::Initialize(const GameContext& gameContext)
{
	auto wallComponentsObj = new GameObject();

	//set model and material
	ModelComponent* pModelWall = new ModelComponent{ L"Resources/Meshes/UnitPlane.ovm" };
	wallComponentsObj->AddComponent(pModelWall);

	auto diffuseWall = new DiffuseMaterial_Shadow{};

	if (m_IsPortalWall)diffuseWall->SetDiffuseTexture(L"Resources/Textures/PortalWall.png");
	else diffuseWall->SetDiffuseTexture(L"Resources/Textures/NormalWall.jpg");

	diffuseWall->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());
	diffuseWall->SetTextureScale(m_TextureScale);
	gameContext.pMaterialManager->AddMaterial(diffuseWall, gameContext.pMaterialManager->GetMaterialId());

	pModelWall->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();

	//physx
	auto physX = PhysxManager::GetInstance()->GetPhysics();
	auto pMatDefault = physX->createMaterial(0.f, 0.f, 0.5f);

	wallComponentsObj->AddComponent(new RigidBodyComponent{ true });
	std::shared_ptr<physx::PxGeometry> sp_WallGeometry(new physx::PxBoxGeometry{ m_WallSize });

	physx::PxTransform wallPose = physx::PxTransform::createIdentity();
	wallPose.p = ToPxVec3(GetTransform()->GetPosition());
	wallPose.q = ToPxQuat(GetTransform()->GetRotation());

	wallComponentsObj->AddComponent(new ColliderComponent(sp_WallGeometry, *pMatDefault, wallPose));

	if (m_IsPortalWall)	wallComponentsObj->SetPortalWall();
	const float sizeRatio{ 5.f };
	wallComponentsObj->GetTransform()->Scale(m_WallSize.x / sizeRatio, m_WallSize.y, m_WallSize.z / sizeRatio);

	AddChild(wallComponentsObj);
}
