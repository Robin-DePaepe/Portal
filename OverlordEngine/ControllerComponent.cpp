#include "stdafx.h"
#include "ControllerComponent.h"
#include "TransformComponent.h"
#include "PhysxProxy.h"
#include "GameObject.h"
#include "GameScene.h"
#include "ControllerComponent.h"
#include <comdef.h>

#pragma warning(disable: 4702)

using namespace physx;
#pragma warning(push)
#pragma warning(disable: 26812)
ControllerComponent::ControllerComponent(physx::PxMaterial* material, float radius, float height, std::wstring name,
	physx::PxCapsuleClimbingMode::Enum climbingMode, bool useCapsuleController)
	: m_Radius(radius),
	m_Height(height),
	m_Name(std::move(name)),
	m_Controller(nullptr),
	m_ClimbingMode(climbingMode),
	m_pMaterial(material),
	m_CollisionFlag(physx::PxControllerCollisionFlags()),
	m_CollisionGroups(physx::PxFilterData(static_cast<UINT32>(CollisionGroupFlag::Group0), 0, 0, 0)),
	m_UseCapsuleController{ useCapsuleController },
	m_pHitReport{ new MyControllerHitReport{} }

{}
ControllerComponent::ControllerComponent(physx::PxMaterial* material, float height, std::wstring name, physx::PxCapsuleClimbingMode::Enum climbingMode, bool useCapsuleController)
	:m_Height(height),
	m_Name(std::move(name)),
	m_Controller(nullptr),
	m_ClimbingMode(climbingMode),
	m_pMaterial(material),
	m_CollisionFlag(physx::PxControllerCollisionFlags()),
	m_CollisionGroups(physx::PxFilterData(static_cast<UINT32>(CollisionGroupFlag::Group0), 0, 0, 0)),
	m_UseCapsuleController{ useCapsuleController },
	m_pHitReport{ new MyControllerHitReport{} }
{
}
ControllerComponent::~ControllerComponent()
{
	delete m_pHitReport;
}
#pragma warning(pop)


void ControllerComponent::Initialize(const GameContext&)
{

	if (m_Controller != nullptr)
	{
		Logger::LogError(L"[ControllerComponent] Cannot initialize a controller twice");
		return;
	}

	if (m_UseCapsuleController) CreateCapsuleController();
	else CreateBoxController();

	//TODO: 4. Set the controller's name (use the value of m_Name) [PxController::setName]
	//   > Converting 'wstring' to 'string' > Use one of the constructor's of the string class
	//	 > Converting 'string' to 'char *' > Use one of the string's methods ;)
	_bstr_t name(m_Name.c_str());
	m_Controller->getActor()->setName(name);
	//TODO: 5. Set the controller's actor's userdata > This Component
	m_Controller->getActor()->userData = this;

	SetCollisionGroup(static_cast<CollisionGroupFlag>(m_CollisionGroups.word0));
	SetCollisionIgnoreGroups(static_cast<CollisionGroupFlag>(m_CollisionGroups.word1));
}

void ControllerComponent::Update(const GameContext&)
{
}

void ControllerComponent::Draw(const GameContext&)
{
}

void ControllerComponent::CreateBoxController()
{
	//TODO: 1. Retrieve the ControllerManager from the PhysX Proxy (PhysxProxy::GetControllerManager();)
	PxControllerManager* pControllerManager = m_pGameObject->GetScene()->GetPhysxProxy()->GetControllerManager();
	//TODO: 2. Create a PxCapsuleControllerDesc (Struct)
	PxBoxControllerDesc boxControllerDesc{};
	//  > Call the "setToDefault()" method of the PxCapsuleControllerDesc
	boxControllerDesc.setToDefault();
	//	> Fill in all the required fields
	boxControllerDesc.halfHeight = m_Height / 2.f;
	boxControllerDesc.upDirection = PxVec3(0, 1, 0);
	boxControllerDesc.contactOffset = 0.1f;
	boxControllerDesc.material = m_pMaterial;
	boxControllerDesc.slopeLimit = 0.707f;//equals to 45 degree (already default)
	boxControllerDesc.stepOffset = 0.5f;
	//  > Position -> Use the position of the parent GameObject
	boxControllerDesc.position = ToPxExtendedVec3(m_pGameObject->GetTransform()->GetPosition());
	//  > UserData -> This component
	boxControllerDesc.userData = this;
	//3. Create the controller object (m_pController), use the ControllerManager to do that (CHECK IF VALID!!)
	if (pControllerManager == nullptr)
	{
		Logger::LogError(L"[ControllerManager] is empty");
		return;
	}
	m_Controller = pControllerManager->createController(boxControllerDesc);

	if (m_Controller == nullptr)
	{
		Logger::LogError(L"[ControllerComponent] Failed to create controller");
		return;
	}
	//setup hit callbacks
	boxControllerDesc.reportCallback = m_pHitReport;
}

void ControllerComponent::CreateCapsuleController()
{
	//TODO: 1. Retrieve the ControllerManager from the PhysX Proxy (PhysxProxy::GetControllerManager();)
	PxControllerManager* pControllerManager = m_pGameObject->GetScene()->GetPhysxProxy()->GetControllerManager();
	//TODO: 2. Create a PxCapsuleControllerDesc (Struct)
	PxCapsuleControllerDesc capsuleControllerDesc{};
	//  > Call the "setToDefault()" method of the PxCapsuleControllerDesc
	capsuleControllerDesc.setToDefault();
	//	> Fill in all the required fields
	//  > Radius, Height, ClimbingMode, UpDirection (PxVec3(0,1,0)), ContactOffset (0.1f), Material [See Initializer List]
	capsuleControllerDesc.height = m_Height;
	capsuleControllerDesc.radius = m_Radius;
	capsuleControllerDesc.climbingMode = m_ClimbingMode;
	capsuleControllerDesc.upDirection = PxVec3(0, 1, 0);
	capsuleControllerDesc.contactOffset = 0.1f;
	capsuleControllerDesc.material = m_pMaterial;
	capsuleControllerDesc.slopeLimit = 0.707f;//equals to 45 degree (already default)
	capsuleControllerDesc.stepOffset = 2.5f;
	//  > Position -> Use the position of the parent GameObject
	capsuleControllerDesc.position = ToPxExtendedVec3(m_pGameObject->GetTransform()->GetPosition());
	//  > UserData -> This component
	capsuleControllerDesc.userData = this;
	//3. Create the controller object (m_pController), use the ControllerManager to do that (CHECK IF VALID!!)
	if (pControllerManager == nullptr)
	{
		Logger::LogError(L"[ControllerManager] is empty");
		return;
	}
	m_Controller = pControllerManager->createController(capsuleControllerDesc);

	if (m_Controller == nullptr)
	{
		Logger::LogError(L"[ControllerComponent] Failed to create controller");
		return;
	}

	//setup hit callbacks
	capsuleControllerDesc.reportCallback = m_pHitReport;
}



void ControllerComponent::Translate(const DirectX::XMFLOAT3& position) const
{
	Translate(position.x, position.y, position.z);
}

void ControllerComponent::Translate(const float x, const float y, const float z) const
{
	if (m_Controller == nullptr)
		Logger::LogError(L"[ControllerComponent] Cannot Translate. No controller present");
	else
		m_Controller->setPosition(physx::PxExtendedVec3(x, y, z));
}

void ControllerComponent::Move(const DirectX::XMFLOAT3 displacement, const float minDist)
{
	if (m_Controller == nullptr)
		Logger::LogError(L"[ControllerComponent] Cannot Move. No controller present");
	else
		m_CollisionFlag = m_Controller->move(ToPxVec3(displacement), minDist, 0, nullptr, nullptr);
}

DirectX::XMFLOAT3 ControllerComponent::GetPosition() const
{
	if (m_Controller == nullptr)
		Logger::LogError(L"[ControllerComponent] Cannot get position. No controller present");
	else
		return ToXMFLOAT3(m_Controller->getPosition());

	return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT3 ControllerComponent::GetFootPosition() const
{
	if (m_Controller == nullptr)
		Logger::LogError(L"[ControllerComponent] Cannot get footposition. No controller present");
	else
		return ToXMFLOAT3(m_Controller->getFootPosition());

	return DirectX::XMFLOAT3();
}

void ControllerComponent::SetCollisionIgnoreGroups(const CollisionGroupFlag ignoreGroups)
{
	using namespace physx;

	m_CollisionGroups.word1 = static_cast<PxU32>(ignoreGroups);

	if (m_Controller != nullptr)
	{
		const auto actor = m_Controller->getActor();
		const auto numShapes = actor->getNbShapes();
		const auto shapes = new PxShape * [numShapes];

		const auto numPointers = actor->getShapes(shapes, numShapes);
		for (PxU32 i = 0; i < numPointers; i++)
		{
#pragma warning (push)
#pragma warning (disable: 6385)
			auto shape = shapes[i];
#pragma warning (pop)
			shape->setSimulationFilterData(m_CollisionGroups);
			//TODO: shouldn't the query filter data be set as well?
		}
		delete[] shapes;
	}
}

void ControllerComponent::SetCollisionGroup(const CollisionGroupFlag group)
{
	using namespace physx;

	m_CollisionGroups.word0 = static_cast<UINT32>(group);

	if (m_Controller != nullptr)
	{
		const auto actor = m_Controller->getActor();
		const auto numShapes = actor->getNbShapes();
		const auto shapes = new PxShape * [numShapes];

		const auto numPointers = actor->getShapes(shapes, numShapes);
		for (PxU32 i = 0; i < numPointers; i++)
		{
#pragma warning (push)
#pragma warning (disable: 6385)
			auto shape = shapes[i];
#pragma warning (pop)
			shape->setSimulationFilterData(m_CollisionGroups);
			shape->setQueryFilterData(m_CollisionGroups);
		}
		delete[] shapes;
	}
}

void MyControllerHitReport::onShapeHit(const PxControllerShapeHit& hit)
{
	UNREFERENCED_PARAMETER(hit);
	std::cout << "oh you hit me";
}

void MyControllerHitReport::onControllerHit(const PxControllersHit& hit)
{
	std::cout << "oh you hit me";

	UNREFERENCED_PARAMETER(hit);
}

void MyControllerHitReport::onObstacleHit(const PxControllerObstacleHit& hit)
{
	std::cout << "oh you hit me";

	UNREFERENCED_PARAMETER(hit);
}
