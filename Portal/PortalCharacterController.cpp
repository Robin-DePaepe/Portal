#include "stdafx.h"
#include "PortalCharacterController.h"
#include "Components.h"
#include "Prefabs.h"
#include "GameScene.h"
#include "PhysxManager.h"
#include "PhysxProxy.h"
#include "SkinnedDiffuseMaterial_Shadow.h"
#include "DiffuseMaterial.h"
#include "ModelAnimator.h"
#include "SoundManager.h"
#include "CenterGlow.h"
#include "PostBlur.h"
#include "CombineResources.h"
#include "PortalPrefab.h"

using namespace physx;
using namespace DirectX;

PortalCharacterController::PortalCharacterController(GameObject* pCrossHair, GameObject* pBluePortalHud, GameObject* pOrangePortalHud, float radius, float height, float moveSpeed) :
	m_Radius(radius),
	m_Height(height),
	m_MoveSpeed(moveSpeed),
	m_pCamera(nullptr),
	m_pController(nullptr),
	m_TotalPitch(0),
	m_TotalYaw(0),
	m_YawBeforeDance(0),
	m_RotationSpeedMouse(30.f),
	m_RotationSpeedController{ 100.f },
	m_pAnimator{ nullptr },
	m_pPickupSocket{ nullptr },
	m_pPortalGunModel{ nullptr },
	m_pPortalGun{ nullptr },
	m_pCharacterModel{ nullptr },
	m_pCharacterModelXAxisInverse{ nullptr },
	m_pRedPortal{ nullptr },
	m_pBluePortal{ nullptr },
	m_pCharacterModelComp{ nullptr },
	m_pCrossHair{ pCrossHair },
	m_pBluePortalShotHud{ pBluePortalHud },
	m_pOrangePortalShotHud{ pOrangePortalHud },
	m_IsDancing{ false },
	m_ModelRotationOffset{ 150.f },
	m_CameraPos{ 0.f, 0.f, 0.f },
	m_CameraRot{ 0.f,0.f,0.f },
	m_CameraPosDancing{ 0.f, 7.5f, -20.f },
	m_CameraRotDancing{ 20.f,0.f,0.f },
	m_CameraLerpAccTime{ 0 },
	m_CameraLerpDuration{ 1.f },
	m_IsLerpingToFirstPerson{ false },
	m_IsLerpingToDance{ false },
	m_MinPitch{ -35 },
	m_MaxPitch{ 35 },
	m_IdleChatterAccumTime{ 0.f },
	m_IdleChatterInterval{ randF(45,90) },
	//PP
	m_pGlowPPEffect{ new CenterGlow{} },
	m_BallGlowRatio{ 0.2f },
	m_CubeGlowRatio{ 0.26f },
	//Running
	m_MaxRunVelocity(50.0f),
	m_TerminalVelocity(2),
	m_Gravity(9.81f),
	m_RunAccelerationTime(0.6f),
	m_JumpAccelerationTime(0.4f),
	m_RunAcceleration(m_MaxRunVelocity / m_RunAccelerationTime),
	m_JumpAcceleration(m_Gravity / m_JumpAccelerationTime),
	m_RunVelocity(0),
	m_JumpVelocity(0),
	m_Velocity(0, 0, 0),
	//sound
	m_pPortalGunShotSucces{ nullptr },
	m_pPortalGunShotFailed{ nullptr },
	m_pDanceTalk{ nullptr },
	m_pIdleTalk{ nullptr },
	m_pHoldingItem{ nullptr }
{}

PortalCharacterController::~PortalCharacterController()
{
	//prevent memory leak when shut down during a dance
	if (m_IsDancing) m_pPortalGun->AddComponent(m_pPortalGunModel);
}

void PortalCharacterController::Initialize(const GameContext& gameContext)
{
	SetTag(L"Player");

	// controller
	m_pController = new ControllerComponent{ PhysxManager::GetInstance()->GetPhysics()->createMaterial(0.3f,0.4f,0.f) ,4.5f,7.f,L"Player character controller", physx::PxCapsuleClimbingMode::eCONSTRAINED };
	m_pController->SetCollisionGroup(CollisionGroupFlag::Group9);
	AddComponent(m_pController);

	// fixed camera as child
	AddChild(new FixedCamera{});

	// Input Actions 
	InputAction left{ GameScene::GameInput::LEFT,InputTriggerState::Down, 'A',0 ,XINPUT_GAMEPAD_DPAD_LEFT,{} };
	InputAction right{ GameScene::GameInput::RIGHT,InputTriggerState::Down, 'D',0 ,XINPUT_GAMEPAD_DPAD_RIGHT,{} };
	InputAction backward{ GameScene::GameInput::BACKWARD,InputTriggerState::Down, 'S',0 ,XINPUT_GAMEPAD_DPAD_DOWN,{} };
	InputAction forward{ GameScene::GameInput::FORWARD,InputTriggerState::Down, 'W' ,0,XINPUT_GAMEPAD_DPAD_UP,{} };
	InputAction jump{ GameScene::GameInput::JUMP,InputTriggerState::Pressed, VK_SPACE ,0 ,XINPUT_GAMEPAD_A,{} };
	InputAction dance{ GameScene::GameInput::DANCE,InputTriggerState::Pressed, 'Q' ,0, XINPUT_GAMEPAD_Y,{} };
	InputAction interact{ GameScene::GameInput::INTERACT,InputTriggerState::Pressed, 'E' , 0, XINPUT_GAMEPAD_X,{} };
	InputAction fireLeft{ GameScene::GameInput::FIRELEFT,InputTriggerState::Released, 0, VK_LBUTTON,XINPUT_GAMEPAD_LEFT_SHOULDER,{} };
	InputAction fireRight{ GameScene::GameInput::FIRERIGHT,InputTriggerState::Released,0, VK_RBUTTON ,XINPUT_GAMEPAD_RIGHT_SHOULDER,{} };

	InputManager* pInput = gameContext.pInput;

	pInput->AddInputAction(left);
	pInput->AddInputAction(backward);
	pInput->AddInputAction(right);
	pInput->AddInputAction(jump);
	pInput->AddInputAction(forward);
	pInput->AddInputAction(dance);
	pInput->AddInputAction(interact);
	pInput->AddInputAction(fireLeft);
	pInput->AddInputAction(fireRight);

	//Character model
	auto skinnedDiffuseMaterial = new SkinnedDiffuseMaterial_Shadow();
	skinnedDiffuseMaterial->SetDiffuseTexture(L"./Resources/Textures/Atlas.png");
	skinnedDiffuseMaterial->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

	gameContext.pMaterialManager->AddMaterial(skinnedDiffuseMaterial, gameContext.pMaterialManager->GetMaterialId());

	m_pCharacterModelComp = new ModelComponent(L"./Resources/Meshes/Atlas.ovm");
	m_pCharacterModelComp->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();
	m_pCharacterModelComp->SetIsActive(false);

	m_pCharacterModel = new GameObject();
	m_pCharacterModelXAxisInverse = new GameObject();

	m_pCharacterModel->AddComponent(m_pCharacterModelComp);

	AddChild(m_pCharacterModelXAxisInverse);
	m_pCharacterModelXAxisInverse->AddChild(m_pCharacterModel);

	m_pCharacterModel->GetTransform()->Translate(0.f, -6.f, 0.f);
	m_pCharacterModel->GetTransform()->Scale(0.15f, 0.15f, 0.15f);

	//Portal gun
	m_pPortalGun = new GameObject();

	auto portalGunDiffuseMaterial = new DiffuseMaterial();
	portalGunDiffuseMaterial->SetDiffuseTexture(L"./Resources/Textures/PortalGun.png");
	gameContext.pMaterialManager->AddMaterial(portalGunDiffuseMaterial, gameContext.pMaterialManager->GetMaterialId());

	m_pPortalGunModel = new ModelComponent(L"./Resources/Meshes/PortalGun.ovm");
	m_pPortalGunModel->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();

	m_pPortalGun->AddComponent(m_pPortalGunModel);

	const float portalGunScale{ -0.1f };
	m_pPortalGun->GetTransform()->Translate(0.4f, -0.6f, 0.8f);
	m_pPortalGun->GetTransform()->Rotate(-30.f, -40.f, 0.f);
	m_pPortalGun->GetTransform()->Scale(portalGunScale, portalGunScale, portalGunScale);

	AddChild(m_pPortalGun);

	//create socket
	m_pPickupSocket = new GameObject();
	m_pPickupSocket->GetTransform()->Translate(-0.1f, -5.f, 9.5f);
	AddChild(m_pPickupSocket);

	//sounds
	auto pFmodSystem = SoundManager::GetInstance()->GetSystem();
	FMOD_RESULT fmodResult;

	fmodResult = pFmodSystem->createStream("Resources/Sounds/ballbot_short_idle_chatter_81.wav", FMOD_LOOP_OFF, 0, &m_pIdleTalk);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->createStream("Resources/Sounds/Groove Jam Emote.mp3", FMOD_LOOP_OFF, 0, &m_pDancing);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->createStream("Resources/Sounds/ballbot_dance_spin_vo_01.wav", FMOD_LOOP_OFF, 0, &m_pDanceTalk);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->createStream("Resources/Sounds/wpn_portalgun_activation_01.wav", FMOD_LOOP_OFF, 0, &m_pPortalGunShotSucces);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->createStream("Resources/Sounds/portal_invalid_surface3.wav", FMOD_LOOP_OFF, 0, &m_pPortalGunShotFailed);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->createStream("Resources/Sounds/hold_loop.wav", FMOD_LOOP_NORMAL, 0, &m_pHoldingItem);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = m_pHoldingItem->setMode(FMOD_LOOP_NORMAL);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = m_pHoldingItem->set3DMinMaxDistance(0.f, 50.f);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	//post processing
	GetScene()->AddPostProcessingEffect(new CombineResources{});
	GetScene()->AddPostProcessingEffect(m_pGlowPPEffect);

	//we want to blur 2x 
	auto pBlur{ new PostBlur() };
	pBlur->SetBlurStrength(10.f);
	GetScene()->AddPostProcessingEffect(pBlur);
	GetScene()->AddPostProcessingEffect(pBlur);

	GetScene()->UsePostProcessingEffects(false);

	//disable portals shot hud
	m_pBluePortalShotHud->GetComponent<SpriteComponent>()->SetIsActive(false);
	m_pOrangePortalShotHud->GetComponent<SpriteComponent>()->SetIsActive(false);
}

void PortalCharacterController::PostInitialize(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);

	//camera
	m_pCamera = GetChild<FixedCamera>()->GetComponent<CameraComponent>();
	m_pCamera->GetTransform()->Translate(m_CameraPos);
	m_pCamera->GetTransform()->Rotate(m_CameraRot);

	//animator
	m_pAnimator = m_pCharacterModel->GetComponent<ModelComponent>()->GetAnimator();
	m_pAnimator->SetAnimation(L"Idle");
	m_pAnimator->Play();
}

auto createFmodVector = [](DirectX::XMFLOAT3 vec)
{
	auto fVec = FMOD_VECTOR();
	fVec.x = vec.x;
	fVec.y = vec.y;
	fVec.z = vec.z;

	return fVec;
};
void PortalCharacterController::Update(const GameContext& gameContext)
{
	//sound
	m_IdleChatterAccumTime += gameContext.pGameTime->GetElapsed();

	if (m_IdleChatterAccumTime >= m_IdleChatterInterval)
	{
		m_IdleChatterAccumTime -= m_IdleChatterInterval;

		GetScene()->PlaySoundIn2D(m_pIdleTalk);
	}

	//GLOBAL LISTENER SETTINGS
	auto forwardFmodV = createFmodVector(m_pCamera->GetTransform()->GetForward());
	auto upFmodV = createFmodVector(m_pCamera->GetTransform()->GetUp());
	auto posFmodV = createFmodVector(m_pCamera->GetTransform()->GetWorldPosition());
	auto velFmodV = createFmodVector(m_Velocity);

	SoundManager::GetInstance()->GetSystem()->set3DListenerAttributes(0, &posFmodV, &velFmodV, &forwardFmodV, &upFmodV);

	//input
	InputManager* pInput = gameContext.pInput;

	if (pInput->IsActionTriggered(GameScene::GameInput::FIRELEFT) && !m_IsDancing)		FirePortal(gameContext, true);
	if (pInput->IsActionTriggered(GameScene::GameInput::FIRERIGHT) && !m_IsDancing)		FirePortal(gameContext, false);

	if (pInput->IsActionTriggered(GameScene::GameInput::INTERACT) && !m_IsDancing)
	{
		if (!RemovePickup())	AddPickup(gameContext);//if the removed failed we weren't holding anything so we try to pick up a new item
	}

	//check if dance just ended
	if (m_IsDancing && m_pAnimator->GetClipName() != L"DanceHipHop")
	{
		m_IsDancing = false;
		m_IsLerpingToFirstPerson = true;

		m_TotalYaw = m_YawBeforeDance;

		//show crosshair and portal gun again
		m_pPortalGun->AddComponent(m_pPortalGunModel);
		m_pCrossHair->GetComponent<SpriteComponent>()->SetIsActive(true);
		m_pBluePortalShotHud->GetComponent<SpriteComponent>()->SetIsActive(true);
		m_pOrangePortalShotHud->GetComponent<SpriteComponent>()->SetIsActive(true);
	}

	//	Lerping the camera for smooth transitions
	if (m_IsLerpingToDance)	LerpCamera(gameContext, m_IsLerpingToDance, m_CameraPosDancing, m_CameraRotDancing, m_CameraPos, m_CameraRot, false);
	if (m_IsLerpingToFirstPerson) LerpCamera(gameContext, m_IsLerpingToFirstPerson, m_CameraPos, m_CameraRot, m_CameraPosDancing, m_CameraRotDancing, true);

	//MOVEMENT
#pragma region 

	//if the player is on the ground and isn't dancing we apply movement
	if (m_pController->GetCollisionFlags().isSet(physx::PxControllerFlag::eCOLLISION_DOWN) && !m_IsDancing)
	{
		const auto forward = XMLoadFloat3(&GetTransform()->GetForward());
		const auto right = XMLoadFloat3(&GetTransform()->GetRight());
		auto move = DirectX::XMFLOAT2(0, 0);

		move = InputManager::GetThumbstickPosition(true);
		const float moveTriggerValue{ 0.25f };

		//check for movement input and set the correct data and animation
		if (pInput->IsActionTriggered(GameScene::GameInput::FORWARD) || move.y > moveTriggerValue)
		{
			move.y = 1.f;
			m_pAnimator->SetAnimation(L"WalkForward", false);
		}
		if (pInput->IsActionTriggered(GameScene::GameInput::BACKWARD) || move.y < -moveTriggerValue)
		{
			move.y = -1.f;
			m_pAnimator->SetAnimation(L"WalkBackward", false);
		}
		if (pInput->IsActionTriggered(GameScene::GameInput::RIGHT) || move.x > moveTriggerValue)
		{
			move.x = 1.f;
			m_pAnimator->SetAnimation(L"WalkRight", false);
		}
		if (pInput->IsActionTriggered(GameScene::GameInput::LEFT) || move.x < -moveTriggerValue)
		{
			move.x = -1.f;
			m_pAnimator->SetAnimation(L"WalkLeft", false);
		}

		if (move.y > moveTriggerValue && move.x < -moveTriggerValue)	m_pAnimator->SetAnimation(L"WalkForwardLeft", false);
		if (move.y > moveTriggerValue && move.x > moveTriggerValue)	m_pAnimator->SetAnimation(L"WalkForwardRight", false);
		if (move.y < -moveTriggerValue && move.x < -moveTriggerValue)	m_pAnimator->SetAnimation(L"WalkBackwardLeft", false);
		if (move.y < -moveTriggerValue && move.x > moveTriggerValue)	m_pAnimator->SetAnimation(L"WalkBackwardRight", false);

		//setup movement velocity
		if (abs(move.x) > 0.1f || abs(move.y) > 0.1f)
		{
			XMVECTOR direction{ forward * move.y + right * move.x };
			direction = XMVector3Normalize(direction);

			float velocityY{ m_Velocity.y };

			m_RunVelocity += m_RunAcceleration * gameContext.pGameTime->GetElapsed();

			if (m_RunVelocity > m_MaxRunVelocity)  m_RunVelocity = m_MaxRunVelocity;

			XMStoreFloat3(&m_Velocity, direction * m_RunVelocity);
			m_Velocity.y = velocityY;
		}
		//reset velocity and movement
		else
		{
			if (m_pAnimator->GetClipName() != L"InAir") m_pAnimator->SetAnimation(L"Idle");

			m_Velocity.x = m_Velocity.z = 0.f;
			m_RunVelocity = 0.f;
		}
	}

	//jumping and gravity 

	//not on the ground 
	if (!m_pController->GetCollisionFlags().isSet(physx::PxControllerFlag::eCOLLISION_DOWN))
	{
		//sent a raycast for objects
		PxQueryFilterData filterData;
		PxRaycastBuffer hit{};
		PxVec3 pos{ m_pController->GetFootPosition().x ,m_pController->GetFootPosition().y,m_pController->GetFootPosition().z };
		PxVec3 dir{ 0,-1,0 };

		//we only want to switch the animation if he will actually fall a significant distance (eg: not when going down a stairs)
		if (GetScene()->GetPhysxProxy()->Raycast(pos, dir, PX_MAX_F32, hit, PxHitFlag::eDEFAULT, filterData))
		{
			if (hit.block.distance > 15.f)		m_pAnimator->SetAnimation(L"InAir");
		}

		//apply gravity
		m_JumpVelocity -= m_JumpAcceleration * gameContext.pGameTime->GetElapsed();

		if (m_JumpVelocity < -m_TerminalVelocity)
		{
			m_JumpVelocity = -m_TerminalVelocity;
		}
	}
	//jumping
	else if (pInput->IsActionTriggered(GameScene::GameInput::JUMP) && !m_IsDancing)
	{
		m_pAnimator->SetAnimation(L"JumpStart");
		m_pAnimator->PlayNewClipOnEnd(L"InAir");

		m_Velocity.y = 50.f;
		m_JumpVelocity = 0.f;
	}
	//on the ground
	else
	{
		//check if we just landed
		if (m_pAnimator->GetClipName() == L"InAir")
		{
			m_pAnimator->SetAnimation(L"JumpLanding");
			m_pAnimator->PlayNewClipOnEnd(L"Idle");
		}
		//check if we the player wants to dance
		else if (pInput->IsActionTriggered(GameScene::GameInput::DANCE) && !m_IsDancing && !m_pAnimator->IsAnimationReserved())	Dance();

		m_Velocity.y = 0.f;
	}

	m_Velocity.y += m_JumpVelocity;

	//move the controller
	auto displacementV{ XMLoadFloat3(&m_Velocity) * gameContext.pGameTime->GetElapsed() };
	XMFLOAT3 displacement{};

	XMStoreFloat3(&displacement, displacementV);
	m_pController->Move(displacement);

#pragma endregion movement

	//ROTATION
#pragma region 
	//check mouse changes for rotation
	auto look = DirectX::XMFLOAT2(0, 0);
	const auto mouseMove = InputManager::GetMouseMovement();

	look.x = static_cast<float>(mouseMove.x);
	look.y = static_cast<float>(mouseMove.y);

	if (look.x == 0 && look.y == 0) //check for controller input
	{
		look = InputManager::GetThumbstickPosition(false);
		look.y *= -1;

		m_TotalYaw += look.x * m_RotationSpeedController * gameContext.pGameTime->GetElapsed();
		m_TotalPitch += look.y * m_RotationSpeedController * gameContext.pGameTime->GetElapsed();
	}
	else //apply the mouse rotation
	{
		m_TotalYaw += look.x * m_RotationSpeedMouse * gameContext.pGameTime->GetElapsed();
		m_TotalPitch += look.y * m_RotationSpeedMouse * gameContext.pGameTime->GetElapsed();
	}

	if (m_TotalPitch > m_MaxPitch) m_TotalPitch = m_MaxPitch;
	if (m_TotalPitch < m_MinPitch) m_TotalPitch = m_MinPitch;

	//seperate for camera behaviour for dancing so that you can spin around and have a better look
	if (m_IsDancing)
	{
		GetTransform()->Rotate(0, m_TotalYaw, 0);
		m_pCharacterModelXAxisInverse->GetTransform()->Rotate(-0, -m_TotalYaw, 0);
		m_pCharacterModel->GetTransform()->Rotate(-0, m_ModelRotationOffset, 0);

		if (m_pBluePortal != nullptr) m_pBluePortal->GetCamera()->GetChild<FixedCamera>()->GetTransform()->Rotate(m_TotalPitch + 180.f, -m_TotalYaw, 0.f);
		if (m_pRedPortal != nullptr) m_pRedPortal->GetCamera()->GetChild<FixedCamera>()->GetTransform()->Rotate(m_TotalPitch + 180.f, -m_TotalYaw, 0.f);
	}
	else
	{
		GetTransform()->Rotate(m_TotalPitch, m_TotalYaw, 0);
		m_pCharacterModelXAxisInverse->GetTransform()->Rotate(-m_TotalPitch, 0, 0);
		m_pCharacterModel->GetTransform()->Rotate(0, m_ModelRotationOffset, 0.f);

		if (m_pBluePortal != nullptr) m_pBluePortal->GetCamera()->GetChild<FixedCamera>()->GetTransform()->Rotate(m_TotalPitch + 180.f, -m_TotalYaw, 0.f);
		if (m_pRedPortal != nullptr) m_pRedPortal->GetCamera()->GetChild<FixedCamera>()->GetTransform()->Rotate(m_TotalPitch + 180.f, -m_TotalYaw, 0.f);
	}
#pragma endregion rotation
}

void PortalCharacterController::SetCameraActive()
{
	m_pCamera->SetActive();
}

void PortalCharacterController::AddRotation(float pitch, float yaw)
{
	m_TotalPitch += pitch;
	m_TotalYaw += yaw;
}

const CameraComponent* PortalCharacterController::GetCamera()
{
	return m_pCamera;
}


void PortalCharacterController::AddPickup(const GameContext& gameContext)
{
	const float maxDistance = 20.f;
	auto hit{ m_pCamera->Pick(gameContext, CollisionGroupFlag::Group9, maxDistance) };

	if (hit.block.actor != nullptr)
	{
		GameObject* pItem{ reinterpret_cast<BaseComponent*>(hit.block.actor->userData)->GetGameObject() };

		if (pItem != nullptr && pItem->IsPickup())
		{
			GetScene()->PlaySoundItemHolding(m_pHoldingItem);

			pItem->GetScene()->RemoveChild(pItem, false);
			pItem->GetComponent<RigidBodyComponent>()->SetKinematic(true);
			pItem->GetTransform()->Translate(0.f, 5.f, 0.f);

			m_pPickupSocket->AddChild(pItem);

			GetScene()->UsePostProcessingEffects(true);

			if (pItem->GetComponents<ColliderComponent>(true).size() > 1) //a ball has 2 colliders 
			{
				m_pGlowPPEffect->SetGlowSizeRatio(m_BallGlowRatio);
				m_pGlowPPEffect->UseCubeShape(false);
			}
			else
			{
				m_pGlowPPEffect->SetGlowSizeRatio(m_CubeGlowRatio);
				m_pGlowPPEffect->UseCubeShape(true);
			}
		}
	}
}

bool PortalCharacterController::RemovePickup()
{
	auto pickup = m_pPickupSocket->GetChild<GameObject>();

	if (pickup != nullptr)
	{
		GetScene()->StopSoundItemHolding();

		const XMFLOAT3& pos = pickup->GetTransform()->GetWorldPosition();

		m_pPickupSocket->RemoveChild(pickup);
		GetScene()->AddChild(pickup);

		pickup->GetTransform()->Translate(pos);

		RigidBodyComponent* pRigid = pickup->GetComponent<RigidBodyComponent>();
		pRigid->SetKinematic(false);
		pRigid->GetPxRigidBody()->setAngularVelocity(PxVec3{ 0.f,0.f,0.f });
		pRigid->GetPxRigidBody()->setLinearVelocity(PxVec3{ 0.f,0.f,0.f });

		GetScene()->UsePostProcessingEffects(false);

		return true;
	}
	return false;
}

void PortalCharacterController::LerpCamera(const GameContext& gameContext, bool& lerpingTo, const XMFLOAT3& posGoto, const XMFLOAT3& rotGoto, const XMFLOAT3& posOriginal, const XMFLOAT3& rotOriginal, bool disableModel)
{
	m_CameraLerpAccTime += gameContext.pGameTime->GetElapsed();

	if (m_CameraLerpAccTime > m_CameraLerpDuration)
	{
		lerpingTo = false;
		m_CameraLerpAccTime = 0.f;

		m_pCamera->GetTransform()->Translate(posGoto);
		m_pCamera->GetTransform()->Rotate(rotGoto);

		if (disableModel) m_pCharacterModelComp->SetIsActive(false);
	}
	else
	{
		float blendFactor = m_CameraLerpAccTime / m_CameraLerpDuration;

		XMVECTOR lerpedTranslation{ XMVectorLerp(XMLoadFloat3(&posOriginal),XMLoadFloat3(&posGoto), blendFactor) };
		XMVECTOR lerpedRotation{ XMQuaternionSlerp(XMLoadFloat3(&rotOriginal),XMLoadFloat3(&rotGoto), blendFactor) };

		XMFLOAT3 newPosition, newRotation;
		XMStoreFloat3(&newPosition, lerpedTranslation);
		XMStoreFloat3(&newRotation, lerpedRotation);

		m_pCamera->GetTransform()->Translate(newPosition);
		m_pCamera->GetTransform()->Rotate(newRotation);
	}
}

void PortalCharacterController::Dance()
{
	m_pAnimator->SetAnimation(L"DanceHipHop");
	m_pAnimator->PlayNewClipOnEnd(L"Idle");

	m_IsDancing = true;
	m_IsLerpingToDance = true;

	m_YawBeforeDance = m_TotalYaw;

	m_Velocity.x = m_Velocity.z = 0.f;
	m_RunVelocity = 0.f;

	RemovePickup();

	GetScene()->PlaySoundIn2D(m_pDanceTalk);
	GetScene()->PlaySoundIn2D(m_pDancing);

	m_pCharacterModelComp->SetIsActive(true);

	//hide the portal gun and crosshair for the dance
	m_pPortalGun->RemoveComponent(m_pPortalGunModel);
	m_pCrossHair->GetComponent<SpriteComponent>()->SetIsActive(false);
	m_pBluePortalShotHud->GetComponent<SpriteComponent>()->SetIsActive(false);
	m_pOrangePortalShotHud->GetComponent<SpriteComponent>()->SetIsActive(false);
}

void PortalCharacterController::FirePortal(const GameContext& gameContext, bool isBlue)
{
	const float maxDistance = D3D11_FLOAT32_MAX;

	auto hit{ m_pCamera->Pick(gameContext, CollisionGroupFlag::Group9, maxDistance) };

	if (hit.block.actor != nullptr)
	{
		GameObject* pItem{ reinterpret_cast<BaseComponent*>(hit.block.actor->userData)->GetGameObject() };

		if (pItem != nullptr)
		{
			if (!pItem->IsPortalWall())
			{
				DirectX::XMFLOAT3 pos{ hit.block.position.x,hit.block.position.y,hit.block.position.z };

				FailedPortalShot(pos, isBlue);

				return;
			}
			const float portalSize{ 1.f };
			DirectX::XMFLOAT3 portalPos{ hit.block.position.x,hit.block.position.y, hit.block.position.z };

			//MAKE SURE THE PORTAL STAYS ON THE WALL AND DOESN 'T LEAVE ITS BOUNDARIES
			//**************

			//first setting up all the necessary variables
			PxBoxGeometry wallGeometry;
			pItem->GetComponent<ColliderComponent>()->GetShape()->getBoxGeometry(wallGeometry);

			const XMFLOAT3& centerWall{ pItem->GetTransform()->GetWorldPosition() };
			const XMFLOAT3& portalUp{ pItem->GetParent()->GetTransform()->GetForward() };
			const XMFLOAT3& portalRight{ pItem->GetParent()->GetTransform()->GetRight() };

			XMVECTOR portalUpV{ XMLoadFloat3(&portalUp) };
			XMVECTOR portalRightV{ XMLoadFloat3(&portalRight) };

			XMVECTOR portalPosV{ XMLoadFloat3(&portalPos) };
			XMVECTOR centerWallV{ XMLoadFloat3(&centerWall) };

			portalRightV = XMVector3Normalize(portalRightV);
			portalUpV = XMVector3Normalize(portalUpV);

			XMVECTOR portalBottomLeft{ portalPosV - portalRightV * (PortalPrefab::GetDefaultColliderSize() * portalSize) };
			XMVECTOR portalTopRight{ portalPosV + portalRightV * (PortalPrefab::GetDefaultColliderSize() * portalSize) };
			portalBottomLeft = { portalBottomLeft - portalUpV * (PortalPrefab::GetDefaultColliderSize() * portalSize * PortalPrefab::GetHeightMultiplier()) };
			portalTopRight = { portalTopRight + portalUpV * (PortalPrefab::GetDefaultColliderSize() * portalSize * PortalPrefab::GetHeightMultiplier()) };

			XMVECTOR wallBottomtLeft{ centerWallV - portalRightV * wallGeometry.halfExtents.x };
			XMVECTOR wallToptRight{ centerWallV + portalRightV * wallGeometry.halfExtents.x };
			wallBottomtLeft = { wallBottomtLeft - portalUpV * wallGeometry.halfExtents.z };
			wallToptRight = { wallToptRight + portalUpV * wallGeometry.halfExtents.z };

			//checking all sides and adjusting to fit on the wall

			//left
			XMVECTOR leftDiff = wallBottomtLeft - portalBottomLeft;
			XMFLOAT3 dotLeft{};

			XMStoreFloat3(&dotLeft, XMVector3Dot(leftDiff, portalRightV));

			if (dotLeft.x > 0.f)
			{
				const XMVECTOR translation{ portalRightV * dotLeft.x };

				portalPosV += translation;
				portalBottomLeft += translation;
				portalTopRight += translation;
			}

			//right
			XMVECTOR rightDiff = wallToptRight - portalTopRight;
			XMFLOAT3 dotRight{};

			XMStoreFloat3(&dotRight, XMVector3Dot(rightDiff, portalRightV));

			if (dotRight.x < 0.f)
			{
				const XMVECTOR translation{ portalRightV * dotRight.x };

				portalPosV += translation;
				portalBottomLeft += translation;
				portalTopRight += translation;
			}

			//bottom
			XMVECTOR bottomDiff = wallBottomtLeft - portalBottomLeft;
			XMFLOAT3 dotBottom{};

			XMStoreFloat3(&dotBottom, XMVector3Dot(bottomDiff, portalUpV));

			if (dotBottom.x > 0.f)
			{
				const XMVECTOR translation{ portalUpV * dotBottom.x };

				portalPosV += translation;
				portalBottomLeft += translation;
				portalTopRight += translation;
			}

			//top
			XMVECTOR topDiff = wallToptRight - portalTopRight;
			XMFLOAT3 dotTop{};

			XMStoreFloat3(&dotTop, XMVector3Dot(topDiff, portalUpV));

			if (dotTop.x < 0.f)
			{
				const XMVECTOR translation{ portalUpV * dotTop.x };

				portalPosV += translation;
				portalBottomLeft += translation;
				portalTopRight += translation;
			}

			//MAKE SURE THE PORTAL DOES NOT OVERLAP WITH THE OTHER PORTAL
			//**************
			GameObject* pOtherPortal{ nullptr };
			bool checkOverLap{ false };

			if (isBlue && m_pRedPortal != nullptr && pItem == m_pRedPortal->GetWall())
			{
				pOtherPortal = m_pRedPortal;
				checkOverLap = true;
			}
			else if (!isBlue && m_pBluePortal != nullptr && pItem == m_pBluePortal->GetWall())
			{
				pOtherPortal = m_pBluePortal;
				checkOverLap = true;
			}
			if (checkOverLap)
			{
				const XMFLOAT3& otherPortalUp{ pOtherPortal->GetTransform()->GetForward() };
				const XMFLOAT3& otherPortalRight{ pOtherPortal->GetTransform()->GetRight() };

				XMVECTOR otherPortalUpV{ XMLoadFloat3(&otherPortalUp) };
				XMVECTOR otherPortalRightV{ XMLoadFloat3(&otherPortalRight) };
				XMVECTOR otherPortalPosV{ XMLoadFloat3(&pOtherPortal->GetTransform()->GetWorldPosition()) };

				otherPortalUpV = XMVector3Normalize(otherPortalUpV);
				otherPortalRightV = XMVector3Normalize(otherPortalRightV);

				XMVECTOR otherPortalBottomLeft{ otherPortalPosV - otherPortalRightV * (PortalPrefab::GetDefaultColliderSize() * portalSize) };
				XMVECTOR otherPortalTopRight{ otherPortalPosV + otherPortalRightV * (PortalPrefab::GetDefaultColliderSize() * portalSize) };
				otherPortalBottomLeft = { otherPortalBottomLeft - otherPortalUpV * (PortalPrefab::GetDefaultColliderSize() * portalSize * PortalPrefab::GetHeightMultiplier()) };
				otherPortalTopRight = { otherPortalTopRight + otherPortalUpV * (PortalPrefab::GetDefaultColliderSize() * portalSize * PortalPrefab::GetHeightMultiplier()) };

				bool IsOverlapping = true;

				//checks recht
				rightDiff = otherPortalTopRight - portalBottomLeft;
				XMStoreFloat3(&dotRight, XMVector3Dot(rightDiff, portalRightV));
				if (dotRight.x < 0.f) IsOverlapping = false;

				//checks left
				leftDiff = otherPortalBottomLeft - portalTopRight;
				XMStoreFloat3(&dotLeft, XMVector3Dot(leftDiff, portalRightV));
				if (dotLeft.x > 0.f) IsOverlapping = false;

				//checks bottom
				topDiff = otherPortalBottomLeft - portalTopRight;
				XMStoreFloat3(&dotTop, XMVector3Dot(topDiff, portalUpV));
				if (dotTop.x > 0.f) IsOverlapping = false;

				//checks top
				bottomDiff = otherPortalTopRight - portalBottomLeft;
				XMStoreFloat3(&dotBottom, XMVector3Dot(bottomDiff, portalUpV));
				if (dotBottom.x < 0.f) IsOverlapping = false;

				if (IsOverlapping)
				{
					const DirectX::XMFLOAT3 pos{ hit.block.position.x,hit.block.position.y,hit.block.position.z };
					FailedPortalShot(pos, isBlue);

					return;
				}
			}

			//restore the changed pos 
			XMStoreFloat3(&portalPos, portalPosV);

			//Spawn Portal
			if (isBlue)
			{
				if (m_pBluePortal != nullptr)	GetScene()->RemoveChild(m_pBluePortal, true); //remove old one

				m_pBluePortal = new PortalPrefab{ isBlue, portalPos,pItem, &m_pRedPortal, this, portalSize };
				GetScene()->AddChild(m_pBluePortal);

				m_pBluePortalShotHud->GetComponent<SpriteComponent>()->SetIsActive(true);
			}
			else
			{
				if (m_pRedPortal != nullptr)	GetScene()->RemoveChild(m_pRedPortal, true); //remove old one

				m_pRedPortal = new PortalPrefab{ isBlue, portalPos,pItem, &m_pBluePortal, this, portalSize };
				GetScene()->AddChild(m_pRedPortal);

				m_pOrangePortalShotHud->GetComponent<SpriteComponent>()->SetIsActive(true);
			}
			GetScene()->PlaySoundIn2D(m_pPortalGunShotSucces);
		}
	}
}

void PortalCharacterController::FailedPortalShot(const DirectX::XMFLOAT3& pos, bool isBlue)
{
	GetScene()->PlaySoundIn2D(m_pPortalGunShotFailed);

	auto temp = new GameObject();

	ParticleEmitterComponent* pParticle = new ParticleEmitterComponent(L"./Resources/Textures/Spark.png", 60);
	pParticle->SetVelocity(20.f);
	pParticle->SetRandomDirections(true);
	pParticle->SetMinSize(0.1f);
	pParticle->SetMaxSize(1.0f);
	pParticle->SetMinEnergy(0.4f);
	pParticle->SetMaxEnergy(0.7f);
	pParticle->SetMinSizeGrow(0.5f);
	pParticle->SetMaxSizeGrow(1.5f);
	pParticle->SetMinEmitterRange(0.01f);
	pParticle->SetMaxEmitterRange(2.5f);
	pParticle->SetStartParticleBurst(25);

	if (isBlue) 	pParticle->SetColor(DirectX::XMFLOAT4(0.f, 0.f, 1.f, 1.f));
	else pParticle->SetColor(DirectX::XMFLOAT4(1.f, 0.f, 0.f, 1.f));

	temp->AddComponent(pParticle);
	temp->AddComponent(new SelfDestructionComponent{ 0.75f,true });

	GetScene()->AddChild(temp);

	temp->GetTransform()->Translate(pos);
}
