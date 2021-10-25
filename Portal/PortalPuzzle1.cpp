#include "stdafx.h"
#include "PortalPuzzle1.h"
#include "PhysxProxy.h"
#include "PhysxManager.h"
#include "Components.h"
#include "DiffuseMaterial_Shadow.h"
#include "ContentManager.h"
#include "PortalCharacterController.h"
#include "OverlordGame.h"
#include "SoundManager.h"
#include "WallPrefab.h"
#include "SceneManager.h"
#include "PauseMenu.h"

bool PortalPuzzle1::m_FirstInit = false;

PortalPuzzle1::PortalPuzzle1(std::wstring sceneName)
	:GameScene{ sceneName }
	, m_IsDoorOpen{ false }
	, m_pDoorClosed{ nullptr }
	, m_pBallRigid{ nullptr }
	, m_pDoorOpen{ nullptr }
	, m_pCharacter{ nullptr }
	, m_enableDoorCollision{ false }
	, m_RemoveDoorCollision{ false }
	, m_PlayerInSwitchRange{ false }
	, m_pButtonCorrectItemEnteredSound{ nullptr }
	, m_pButtonCorrectItemLeftSound{ nullptr }
	, m_pWhatAreYouDoingSound{ nullptr }
	, m_pStartQuoteSound{ nullptr }
	, m_pDoorOpeningSound{ nullptr }
	, m_pSwitchSucessfullyActivatedSound{ nullptr }
	, m_pSwitchUnSucessfullyActivatedSound{ nullptr }
	, m_CubeButtonValid{ false }
	, m_BallButtonValid{ false }
	, m_PauseMenuName{ L"Pause menu L1" }
{}

PortalPuzzle1::~PortalPuzzle1()
{
	//we have to add these back to make sure we have no leaks or errors when trying to delete 
	if (m_IsDoorOpen)
	{
		GetPhysxProxy()->GetPhysxScene()->addActor(*m_pDoorClosed->GetComponent<RigidBodyComponent>()->GetPxRigidActor());
		AddChild(m_pDoorClosed);
	}
	else AddChild(m_pDoorOpen);
}

void PortalPuzzle1::Initialize()
{
	const auto gameContext = GetGameContext();
	GameScene::Initialize();

	//setup settings
	GetPhysxProxy()->EnablePhysxDebugRendering(false);

	//more initializations
	InitializeBuildingBlocks(gameContext);
	InitializePlayer();
	InitializetriggerObjects(gameContext);
	InitializeProps();
	InitializeSounds();

	//make the pause menu
	SceneManager::GetInstance()->AddGameScene(new PauseMenu(GetSceneName(), m_pChannel, m_pBackGroundChannel, m_PauseMenuName));

	//input
	InputAction pause{ GameScene::GameInput::PAUSE,InputTriggerState::Pressed, VK_ESCAPE,0 ,XINPUT_GAMEPAD_START,{} };

	GetGameContext().pInput->AddInputAction(pause);

	m_FirstInit = true;
}

void PortalPuzzle1::Update()
{
	GameScene::Update();

	//update sound
	FMOD_VECTOR posItem{}, vel{ 0.f,0.f,0.f };
	posItem.x = m_pCharacter->GetPickupSocket()->GetTransform()->GetWorldPosition().x;
	posItem.y = m_pCharacter->GetPickupSocket()->GetTransform()->GetWorldPosition().y;
	posItem.z = m_pCharacter->GetPickupSocket()->GetTransform()->GetWorldPosition().z;
	m_pChannelHoldingItem->set3DAttributes(&posItem, &vel);

	//handle inputs
	InputManager* pInput = GetGameContext().pInput;

	if (pInput->IsActionTriggered(GameScene::GameInput::PAUSE)) SceneManager::GetInstance()->SetActiveGameScene(m_PauseMenuName);

	//check if the switch is pressed
	if (m_PlayerInSwitchRange && pInput->IsActionTriggered(GameScene::GameInput::INTERACT))
	{
		if (m_pBallRigid->IsKinematic())
		{
			PlaySoundIn2D(m_pSwitchSucessfullyActivatedSound);

			//let the cube drop
			m_pBallRigid->SetKinematic(false);
			m_pBallRigid->AddTorque(PxVec3{ 0.5f,0.3f,0.7f }, PxForceMode::eIMPULSE); //to awake and give a spin to it
		}
		else PlaySoundIn2D(m_pSwitchUnSucessfullyActivatedSound);
	}
	//we do this with booleans in the update because we can't change the physX in a lambda expression during a trigger event
	if (m_RemoveDoorCollision)
	{
		m_RemoveDoorCollision = false;
		GetPhysxProxy()->GetPhysxScene()->removeActor(*m_pDoorClosed->GetComponent<RigidBodyComponent>()->GetPxRigidActor(), false);
	}
	if (m_enableDoorCollision)
	{
		m_enableDoorCollision = false;
		GetPhysxProxy()->GetPhysxScene()->addActor(*m_pDoorClosed->GetComponent<RigidBodyComponent>()->GetPxRigidActor());
	}
}

void PortalPuzzle1::SceneActivated()
{
	GetGameContext().pShadowMapper->SetLight({ -40.f, 20.f, 135.f }, { 0.740129888f, -0.797205281f, -0.409117377f });

	PauseSounds(false);
	GetGameContext().pInput->ForceMouseToCenter(true);

	std::cout << "\nREAD THIS: \nUse the WASD and spacebar to control your character's movement. Your mouse will be used to control the rotation.\n";
	std::cout << "Left and right mouse clicks will shoot out the portals.\n";
	std::cout << "Press 'E' to interact with items and pickups.\n";
	std::cout << "Press ESC for the pause menu.\n";
	std::cout << "Press 'Q' to.. wel just check it out!\n";

	m_pCharacter->SetCameraActive();
}

void PortalPuzzle1::SceneDeactivated()
{
	PauseSounds(true);
}

void PortalPuzzle1::InitializePlayer()
{
	//adding crosshair
	GameObject* pCrosshair = new GameObject();
	DirectX::XMFLOAT3 crossHairLocation{ (float)OverlordGame::GetGameSettings().Window.Width / 2.f, (float)OverlordGame::GetGameSettings().Window.Height / 2.f, 0.f };

	pCrosshair->AddComponent(new SpriteComponent(L"./Resources/Textures/EmptyCrosshair.png", DirectX::XMFLOAT2(0.5f, 0.5f), DirectX::XMFLOAT4(1, 1, 1, 1.f)));

	const float crosshairScale{ 1.5f };
	pCrosshair->GetTransform()->Scale(crosshairScale, crosshairScale, crosshairScale);
	pCrosshair->GetTransform()->Translate(crossHairLocation);

	AddChild(pCrosshair);

	//adding portal shot hud 
	GameObject* pPortalShotBlue = new GameObject();
	pPortalShotBlue->AddComponent(new SpriteComponent(L"./Resources/Textures/CrossHairBlue.png", DirectX::XMFLOAT2(0.5f, 0.5f), DirectX::XMFLOAT4(1, 1, 1, 1.f)));

	pPortalShotBlue->GetTransform()->Translate(crossHairLocation);
	pPortalShotBlue->GetTransform()->Scale(crosshairScale, crosshairScale, crosshairScale);

	AddChild(pPortalShotBlue);

	GameObject* pPortalShotOrange = new GameObject();
	pPortalShotOrange->AddComponent(new SpriteComponent(L"./Resources/Textures/CrossHairOrange.png", DirectX::XMFLOAT2(0.5f, 0.5f), DirectX::XMFLOAT4(1, 1, 1, 1.f)));

	pPortalShotOrange->GetTransform()->Translate(crossHairLocation);
	pPortalShotOrange->GetTransform()->Scale(crosshairScale, crosshairScale, crosshairScale);

	AddChild(pPortalShotOrange);

	//character
	m_pCharacter = new PortalCharacterController(pCrosshair, pPortalShotBlue, pPortalShotOrange);
	m_pCharacter->GetTransform()->Translate(-40.f, 10.f, 75.f);
	AddChild(m_pCharacter);
}

void PortalPuzzle1::InitializeSounds()
{
	//SOUND
	auto pFmodSystem = SoundManager::GetInstance()->GetSystem();
	FMOD_RESULT fmodResult;
	FMOD::Sound* pSound;

	if (!m_FirstInit) //weird bug causes 3d sound to play even when old scene is removed
	{
		fmodResult = pFmodSystem->createStream("Resources/Sounds/looping_radio_mix.wav", FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &pSound);
		SoundManager::GetInstance()->ErrorCheck(fmodResult);

		fmodResult = pSound->set3DMinMaxDistance(0.f, 50.f);
		SoundManager::GetInstance()->ErrorCheck(fmodResult);

		fmodResult = pFmodSystem->playSound(pSound, 0, true, &m_pChannel);
		SoundManager::GetInstance()->ErrorCheck(fmodResult);
	}

	//audio radio
	DirectX::XMFLOAT3 deskPos{ -40.0f, 0.f, 150.f };

	FMOD_VECTOR posItem{ deskPos.x,deskPos.y,deskPos.z }, vel{ 0.f,0.f,0.f };
	m_pChannel->set3DAttributes(&posItem, &vel);

	m_pChannel->setPaused(false);//we only unpause this channel after the attributes are set to avoid sound in origin at startup.

	//other sounds
	fmodResult = pFmodSystem->createStream("Resources/Sounds/warehouse_ambience_lp_01.wav", FMOD_DEFAULT, 0, &pSound);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->playSound(pSound, 0, false, &m_pBackGroundChannel);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->createStream("Resources/Sounds/portal_button_down_01.wav", FMOD_LOOP_OFF, 0, &m_pButtonCorrectItemEnteredSound);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->createStream("Resources/Sounds/portal_button_up_01.wav", FMOD_LOOP_OFF, 0, &m_pButtonCorrectItemLeftSound);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->createStream("Resources/Sounds/button_synth_positive_01.wav", FMOD_LOOP_OFF, 0, &m_pSwitchSucessfullyActivatedSound);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->createStream("Resources/Sounds/button_synth_negative_02.wav", FMOD_LOOP_OFF, 0, &m_pSwitchUnSucessfullyActivatedSound);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->createStream("Resources/Sounds/door_open_chime_01.wav", FMOD_LOOP_OFF, 0, &m_pDoorOpeningSound);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);


	fmodResult = pFmodSystem->createStream("Resources/Sounds/StartQuote.wav", FMOD_LOOP_OFF, 0, &m_pStartQuoteSound);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->createStream("Resources/Sounds/WhatAreYouDoing.wav", FMOD_LOOP_OFF, 0, &m_pWhatAreYouDoingSound);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	PlaySoundIn2D(m_pStartQuoteSound);

	PauseSounds(true);
}

void PortalPuzzle1::InitializeProps()
{
	const auto gameContext = GetGameContext();
	GameScene::Initialize();

	auto physX = PhysxManager::GetInstance()->GetPhysics();
	auto pMatDefault = physX->createMaterial(0.f, 0.f, 0.f);

	//desk model 
	const float scaleDesk = 0.15f;
	DirectX::XMFLOAT3 deskPos{ -40.0f, 0.f, 150.f };
	auto desk = new GameObject();

	ModelComponent* pModelDesk = new ModelComponent{ L"Resources/Meshes/Desk.ovm" };
	desk->AddComponent(pModelDesk);

	auto diffuseDesk = new DiffuseMaterial_Shadow{};
	diffuseDesk->SetDiffuseTexture(L"Resources/Textures/Desk.png");
	diffuseDesk->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

	gameContext.pMaterialManager->AddMaterial(diffuseDesk, gameContext.pMaterialManager->GetMaterialId());

	pModelDesk->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();

	desk->GetTransform()->Scale(scaleDesk, scaleDesk, scaleDesk);
	desk->GetTransform()->Translate(deskPos);

	desk->AddComponent(new RigidBodyComponent{ true });

	physx::PxConvexMesh* pDeskMesh = ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/Desk.ovpc");
	std::shared_ptr<physx::PxGeometry> sp_DeskGeometry(new physx::PxConvexMeshGeometry{ pDeskMesh });
	desk->AddComponent(new ColliderComponent(sp_DeskGeometry, *pMatDefault));

	AddChild(desk);

	desk->GetComponent<RigidBodyComponent>()->Scale(scaleDesk);

	//Radio
	const float scaleRadio = 0.15f;
	auto radio = new GameObject();

	ModelComponent* pModelRadio = new ModelComponent{ L"Resources/Meshes/Radio.ovm" };
	radio->AddComponent(pModelRadio);

	auto diffuseRadio = new DiffuseMaterial_Shadow{};
	diffuseRadio->SetDiffuseTexture(L"Resources/Textures/Radio.png");
	diffuseRadio->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

	gameContext.pMaterialManager->AddMaterial(diffuseRadio, gameContext.pMaterialManager->GetMaterialId());
	pModelRadio->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();

	radio->GetTransform()->Scale(scaleRadio, scaleRadio, scaleRadio);
	radio->GetTransform()->Translate(deskPos);

	AddChild(radio);

	//Item Dropper
	//************
	GameObject* pItemDropper = new GameObject();
	const DirectX::XMFLOAT3 itemDropperPos{ 135.f,55.f,230.f };
	ModelComponent* pModelItemDropper = new ModelComponent{ L"Resources/Meshes/ItemDropper.ovm" };
	pItemDropper->AddComponent(pModelItemDropper);

	auto diffuseItemDropper = new DiffuseMaterial_Shadow{};
	diffuseItemDropper->SetDiffuseTexture(L"Resources/Textures/ItemDropper.png");
	diffuseItemDropper->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

	gameContext.pMaterialManager->AddMaterial(diffuseItemDropper, gameContext.pMaterialManager->GetMaterialId());

	pModelItemDropper->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();

	const float scaleItemDropper = 0.1f;
	pItemDropper->GetTransform()->Scale(scaleItemDropper, scaleItemDropper, scaleItemDropper);
	pItemDropper->GetTransform()->Translate(itemDropperPos);

	AddChild(pItemDropper);

	//cube
//************
	auto cube = new GameObject();
	auto pMatCube = physX->createMaterial(0.02f, 0.6f, 0.95f);

	ModelComponent* pModelCube = new ModelComponent{ L"Resources/Meshes/Cube.ovm" };
	cube->AddComponent(pModelCube);

	auto diffuseCube = new DiffuseMaterial_Shadow{};
	diffuseCube->SetDiffuseTexture(L"Resources/Textures/Cube.png");
	diffuseCube->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

	gameContext.pMaterialManager->AddMaterial(diffuseCube, gameContext.pMaterialManager->GetMaterialId());

	pModelCube->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();

	RigidBodyComponent* pRigidCube = { new RigidBodyComponent{ } };
	pRigidCube->SetDensity(0.02f);
	cube->AddComponent(pRigidCube);

	const float size{ 2.f };
	std::shared_ptr<physx::PxGeometry> sp_CubeGeometry(new physx::PxBoxGeometry{ size,size, size });

	cube->AddComponent(new ColliderComponent(sp_CubeGeometry, *pMatCube));

	AddChild(cube);

	cube->GetTransform()->Translate(-40.0f, 10.f, 145.f);
	cube->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	cube->SetTag(L"CubeKey");
	cube->SetPickup(true);

	//Ball
//************
	auto ball = new GameObject();
	auto pMatBall = physX->createMaterial(0.5f, 0.5f, 1.f);

	ModelComponent* pModelBall = new ModelComponent{ L"Resources/Meshes/Ball.ovm" };
	ball->AddComponent(pModelBall);

	auto diffuseBall = new DiffuseMaterial_Shadow{};
	diffuseBall->SetDiffuseTexture(L"Resources/Textures/Ball.png");
	diffuseBall->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

	gameContext.pMaterialManager->AddMaterial(diffuseBall, gameContext.pMaterialManager->GetMaterialId());

	pModelBall->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();

	//we add 2 geometry's here, one is a sphere for all collisions except for ovpt, this would cause the physx to trigger an error so we use a box for those collisions
	m_pBallRigid = { new RigidBodyComponent{ } };
	m_pBallRigid->SetDensity(0.1f);
	m_pBallRigid->SetKinematic(true);

	ball->AddComponent(m_pBallRigid);

	std::shared_ptr<physx::PxGeometry> sp_BallGeometryBall(new physx::PxSphereGeometry{ 2.f });
	ColliderComponent* pBallCollider{ new ColliderComponent(sp_BallGeometryBall, *pMatBall) };
	ball->AddComponent(pBallCollider);

	std::shared_ptr<physx::PxGeometry> sp_BallGeometryCube(new physx::PxBoxGeometry{ 1.5f,1.5f,1.5f });
	ColliderComponent* pBallColliderBox{ new ColliderComponent(sp_BallGeometryCube, *pMatBall) };
	ball->AddComponent(pBallColliderBox);

	AddChild(ball);
	m_pBallRigid->SetCollisionIgnoreGroups(static_cast<CollisionGroupFlag>(uint32_t(CollisionGroupFlag::Group0) | uint32_t(CollisionGroupFlag::Group1) | uint32_t(CollisionGroupFlag::Group3)
		| uint32_t(CollisionGroupFlag::Group4) | uint32_t(CollisionGroupFlag::Group5) | uint32_t(CollisionGroupFlag::Group6) | uint32_t(CollisionGroupFlag::Group7) | uint32_t(CollisionGroupFlag::Group8) | uint32_t(CollisionGroupFlag::Group9)), pBallColliderBox);
	m_pBallRigid->SetCollisionIgnoreGroups(CollisionGroupFlag::Group2, pBallCollider);

	ball->GetTransform()->Translate(itemDropperPos.x, itemDropperPos.y - 1.f, itemDropperPos.z);

	ball->GetTransform()->Scale(0.1f, 0.1f, 0.1f);

	ball->SetPickup(true);
	ball->SetTag(L"BallKey");
}

void PortalPuzzle1::InitializeBuildingBlocks(const GameContext& gameContext)
{
	auto physX = PhysxManager::GetInstance()->GetPhysics();

	auto pMatDefault = physX->createMaterial(0.5f, 0.5f, 0.2f);
	auto pGround = new GameObject();
	pGround->AddComponent(new RigidBodyComponent(true));

	std::shared_ptr<physx::PxGeometry> geom(new physx::PxPlaneGeometry());
	pGround->AddComponent(new ColliderComponent(geom, *pMatDefault, physx::PxTransform(physx::PxQuat(DirectX::XM_PIDIV2, physx::PxVec3(0, 0, 1)))));
	AddChild(pGround);

	//ground mesh
	auto matGround = new DiffuseMaterial_Shadow();
	matGround->SetDiffuseTexture(L"./Resources/Textures/FloorTile.png");
	matGround->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());
	matGround->SetTextureScale(30.f);
	gameContext.pMaterialManager->AddMaterial(matGround, gameContext.pMaterialManager->GetMaterialId());

	auto pGroundObj = new GameObject();
	auto pGroundModel = new ModelComponent(L"./Resources/Meshes/UnitPlane.ovm");
	pGroundModel->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();

	pGroundObj->AddComponent(pGroundModel);
	const float floorScale{ 150.f };
	pGroundObj->GetTransform()->Scale(floorScale, floorScale, floorScale);
	pGroundObj->GetTransform()->Translate(0.f, 0.f, 51.f);

	AddChild(pGroundObj);

	//main level walls
	DirectX::XMVECTOR wallRotation{ DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(90.f), DirectX::XMConvertToRadians(0.f), DirectX::XMConvertToRadians(0.f)) };
	DirectX::XMFLOAT3 wallPos{ 50.f, 30.f, 50.f };
	PxVec3 wallSize{ 100.f, 0.1f, 30.f };
	float wallTextureScale{ 1.f };

	AddChild(new WallPrefab(wallRotation, wallPos, wallSize, wallTextureScale));

	wallPos.z += wallSize.x * 2.f;
	wallTextureScale = 3.f;
	wallSize.x -= 10.f;
	wallPos.x -= 10.f;
	wallRotation = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(90.f), DirectX::XMConvertToRadians(180.f), DirectX::XMConvertToRadians(0.f));
	AddChild(new WallPrefab(wallRotation, wallPos, wallSize, wallTextureScale, false));

	wallSize.x += 10.f;
	wallRotation = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(90.f), DirectX::XMConvertToRadians(-90.f), DirectX::XMConvertToRadians(0.f));
	wallPos.x += wallSize.x + 10.f;
	wallPos.z -= wallSize.x;
	wallTextureScale = 1.f;
	AddChild(new WallPrefab(wallRotation, wallPos, wallSize, wallTextureScale, false));

	wallRotation = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(90.f), DirectX::XMConvertToRadians(90.f), DirectX::XMConvertToRadians(0.f));
	wallPos.x -= wallSize.x * 2.f;
	AddChild(new WallPrefab(wallRotation, wallPos, wallSize, wallTextureScale, false));

	wallRotation = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(0.f), DirectX::XMConvertToRadians(0.f), DirectX::XMConvertToRadians(0.f));
	wallSize.x = wallSize.z = 100.f;
	wallPos.x = 50.f;
	wallPos.y = 60.f;
	wallPos.z = 150.f;
	wallTextureScale = 5.f;
	AddChild(new WallPrefab(wallRotation, wallPos, wallSize, wallTextureScale));

	//extra platforms for intersting gameplay
	wallPos = { 50.f, 25.f, 65.f };
	wallSize = { 30.f, 0.1f, 30.f };
	wallTextureScale = { 1.f };

	AddChild(new WallPrefab(wallRotation, wallPos, wallSize, wallTextureScale, false));

	wallPos = { 135.f, 25.f, 235.f };
	wallSize = { 15.f, 0.1f, 15.f };
	wallTextureScale = { 1.f };

	AddChild(new WallPrefab(wallRotation, wallPos, wallSize, wallTextureScale, false));

	wallRotation = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(90.f), DirectX::XMConvertToRadians(180.f), DirectX::XMConvertToRadians(0.f));
	wallPos = { 140.f, 30.f, 249.9f };
	wallSize = { 10.f, 0.1f, 30.f };
	wallTextureScale = { 0.5f };

	AddChild(new WallPrefab(wallRotation, wallPos, wallSize, wallTextureScale));


	//walls for testing
	wallRotation = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(5.f), DirectX::XMConvertToRadians(70.f), DirectX::XMConvertToRadians(15.f));
	wallSize.z = 20.f;
	wallPos.x = 20.f;
	wallPos.y = 15.f;
	wallPos.z = -100.f;
	AddChild(new WallPrefab(wallRotation, wallPos, wallSize, wallTextureScale));


	wallRotation = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(50.f), DirectX::XMConvertToRadians(16.f), DirectX::XMConvertToRadians(89.4f));
	wallSize.z = 20.f;
	wallPos.x = 20.f;
	wallPos.y = 15.f;
	wallPos.z = -65.f;
	AddChild(new WallPrefab(wallRotation, wallPos, wallSize, wallTextureScale));

	//Door open
//************
	const float scaleDoor = 0.25f;
	DirectX::XMFLOAT3 doorPos{ 50.0f, 10.f, 250.f };
	m_pDoorOpen = new GameObject();

	ModelComponent* pModelDoorOpen = new ModelComponent{ L"Resources/Meshes/DoorOpen.ovm" };
	m_pDoorOpen->AddComponent(pModelDoorOpen);

	auto diffuseDoor = new DiffuseMaterial_Shadow{};
	diffuseDoor->SetDiffuseTexture(L"Resources/Textures/Door.png");
	diffuseDoor->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

	const int doorId{ gameContext.pMaterialManager->GetMaterialId() };
	gameContext.pMaterialManager->AddMaterial(diffuseDoor, doorId);

	pModelDoorOpen->SetMaterial(doorId);
	gameContext.pMaterialManager->IncrementMaterialId();

	m_pDoorOpen->GetTransform()->Scale(scaleDoor, scaleDoor, scaleDoor);
	m_pDoorOpen->GetTransform()->Translate(doorPos);

	m_pDoorOpen->AddComponent(new RigidBodyComponent{ true });

	physx::PxTriangleMesh* pDoorOpenMesh = ContentManager::Load<physx::PxTriangleMesh>(L"Resources/Meshes/DoorOpen.ovpt");
	std::shared_ptr<physx::PxGeometry> sp_DoorOpenGeometry(new physx::PxTriangleMeshGeometry{ pDoorOpenMesh });

	m_pDoorOpen->AddComponent(new ColliderComponent(sp_DoorOpenGeometry, *pMatDefault));

	AddChild(m_pDoorOpen);
	m_pDoorOpen->GetComponent<RigidBodyComponent>()->Scale(scaleDoor);
	RemoveChild(m_pDoorOpen, false);

	//Door closed
//************
	m_pDoorClosed = new GameObject();

	ModelComponent* pModelDoorClosed = new ModelComponent{ L"Resources/Meshes/DoorClosed.ovm" };
	m_pDoorClosed->AddComponent(pModelDoorClosed);

	pModelDoorClosed->SetMaterial(doorId);

	m_pDoorClosed->GetTransform()->Scale(scaleDoor, scaleDoor, scaleDoor);
	m_pDoorClosed->GetTransform()->Translate(doorPos);

	m_pDoorClosed->AddComponent(new RigidBodyComponent{ true });

	std::shared_ptr<physx::PxGeometry> sp_DoorClosedGeometry(new physx::PxBoxGeometry{ 60.f,40.f, 1.f });//box to save performance

	m_pDoorClosed->AddComponent(new ColliderComponent(sp_DoorClosedGeometry, *pMatDefault));

	AddChild(m_pDoorClosed);
	m_pDoorClosed->GetComponent<RigidBodyComponent>()->Scale(scaleDoor);

	//End game trigger
	GameObject* endGameTrigger = new GameObject();

	endGameTrigger->GetTransform()->Translate(doorPos);

	std::shared_ptr<physx::PxGeometry> sp_ButtonCubeCollider(new physx::PxBoxGeometry{ 15.f,10.f, 1.1f });

	auto triggerColliderButtonCube{ new ColliderComponent(sp_ButtonCubeCollider, *pMatDefault) };
	triggerColliderButtonCube->EnableTrigger(true);

	endGameTrigger->AddComponent(triggerColliderButtonCube);
	endGameTrigger->AddComponent(new RigidBodyComponent(true));

	endGameTrigger->SetOnTriggerCallBack([this](GameObject*, GameObject* otherObject, GameObject::TriggerAction action)
		{
			if (L"Player" == otherObject->GetTag())
			{
				if (action == GameObject::TriggerAction::ENTER)
				{
					SceneManager::GetInstance()->RemoveGameScene(this);
					SceneManager::GetInstance()->SetActiveGameScene(L"Main menu");
				}
			}
		}
	);
	AddChild(endGameTrigger);
}

void PortalPuzzle1::CloseDoor()
{
	RemoveChild(m_pDoorOpen, false);
	AddChild(m_pDoorClosed);

	m_IsDoorOpen = false;
	m_enableDoorCollision = true;
}

void PortalPuzzle1::OpenDoor()
{
	RemoveChild(m_pDoorClosed, false);
	AddChild(m_pDoorOpen);

	m_IsDoorOpen = true;
	m_RemoveDoorCollision = true;

	PlaySoundIn2D(m_pDoorOpeningSound);
}

void PortalPuzzle1::InitializetriggerObjects(const GameContext& gameContext)
{
	auto physX = PhysxManager::GetInstance()->GetPhysics();

	auto pMatDefault = physX->createMaterial(0.5f, 0.5f, 0.6f);

	//button cube
//************
	auto buttonCube = new GameObject();
	DirectX::XMFLOAT3 positionCubeButton{ 133.0f, 25.1f, 232.f };

	ModelComponent* pModelButtonCube = new ModelComponent{ L"Resources/Meshes/ButtonCube.ovm" };
	buttonCube->AddComponent(pModelButtonCube);

	auto diffuseButtonCube = new DiffuseMaterial_Shadow{};
	diffuseButtonCube->SetDiffuseTexture(L"Resources/Textures/ButtonShapes.png");
	diffuseButtonCube->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

	gameContext.pMaterialManager->AddMaterial(diffuseButtonCube, gameContext.pMaterialManager->GetMaterialId());
	pModelButtonCube->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();

	//add a triggers collider
	GameObject* pColliderButtonCube = new GameObject();
	const float colliderButtonCubeSize{ 2.5f };

	pColliderButtonCube->GetTransform()->Translate(positionCubeButton);

	std::shared_ptr<physx::PxGeometry> sp_ButtonCubeCollider(new physx::PxBoxGeometry{ colliderButtonCubeSize,colliderButtonCubeSize, colliderButtonCubeSize });

	auto triggerColliderButtonCube{ new ColliderComponent(sp_ButtonCubeCollider, *pMatDefault) };
	triggerColliderButtonCube->EnableTrigger(true);

	pColliderButtonCube->AddComponent(triggerColliderButtonCube);
	pColliderButtonCube->AddComponent(new RigidBodyComponent(true));

	pColliderButtonCube->SetOnTriggerCallBack([this](GameObject* triggerObject, GameObject* otherObject, GameObject::TriggerAction action)
		{
			if (triggerObject->GetTag() == otherObject->GetTag())
			{
				if (action == GameObject::TriggerAction::ENTER)
				{
					m_CubeButtonValid = true;
					PlaySoundIn2D(m_pButtonCorrectItemEnteredSound);

					if (m_CubeButtonValid && m_BallButtonValid) OpenDoor();
				}
				else if (action == GameObject::TriggerAction::LEAVE)
				{
					{
						m_CubeButtonValid = false;
						PlaySoundIn2D(m_pButtonCorrectItemLeftSound);

						if (m_IsDoorOpen) CloseDoor();
					}
				}
			}
			else if (action == GameObject::TriggerAction::ENTER)	PlaySoundIn2D(m_pWhatAreYouDoingSound);
		}
	);
	pColliderButtonCube->SetTag(L"CubeKey");
	buttonCube->AddChild(pColliderButtonCube);

	//setup transform 
	const float scaleButtonCube = 0.15f;
	buttonCube->GetTransform()->Scale(scaleButtonCube, scaleButtonCube, scaleButtonCube);
	buttonCube->GetTransform()->Translate(positionCubeButton);

	//setup collisions
	auto pRigidButtonCube{ new RigidBodyComponent{true } };
	pRigidButtonCube->SetCollisionGroup(CollisionGroupFlag::Group2);
	buttonCube->AddComponent(pRigidButtonCube);

	physx::PxTriangleMesh* pButtonCubeMesh = ContentManager::Load<physx::PxTriangleMesh>(L"Resources/Meshes/ButtonCube.ovpt");
	std::shared_ptr<physx::PxGeometry> sp_ButtonCubeGeometry(new physx::PxTriangleMeshGeometry{ pButtonCubeMesh });

	buttonCube->AddComponent(new ColliderComponent(sp_ButtonCubeGeometry, *pMatDefault));

	AddChild(buttonCube);
	pRigidButtonCube->Scale(scaleButtonCube);


	//button ball
//************
	auto buttonBall = new GameObject();
	DirectX::XMFLOAT3 positionBallButton{ 0.0f, 0.f, 200.f };

	ModelComponent* pModelButtonBall = new ModelComponent{ L"Resources/Meshes/ButtonBall.ovm" };
	buttonBall->AddComponent(pModelButtonBall);

	//material
	auto diffuseButtonBall = new DiffuseMaterial_Shadow{};
	diffuseButtonBall->SetDiffuseTexture(L"Resources/Textures/ButtonShapes.png");
	diffuseButtonBall->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

	gameContext.pMaterialManager->AddMaterial(diffuseButtonBall, gameContext.pMaterialManager->GetMaterialId());
	pModelButtonBall->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();

	//add a trigger collider
	GameObject* pColliderButtonBall = new GameObject();
	const float colliderButtonBallSize{ 3.5f };

	pColliderButtonBall->GetTransform()->Translate(positionBallButton);

	std::shared_ptr<physx::PxGeometry> sp_ButtonBallCollider(new physx::PxSphereGeometry{ colliderButtonBallSize });

	auto triggerColliderBallButton{ new ColliderComponent(sp_ButtonBallCollider, *pMatDefault) };
	triggerColliderBallButton->EnableTrigger(true);

	pColliderButtonBall->AddComponent(triggerColliderBallButton);
	pColliderButtonBall->AddComponent(new RigidBodyComponent(true));

	pColliderButtonBall->SetOnTriggerCallBack([this](GameObject* triggerObject, GameObject* otherObject, GameObject::TriggerAction action)
		{
			if (triggerObject->GetTag() == otherObject->GetTag())
			{
				if (action == GameObject::TriggerAction::ENTER)
				{
					m_BallButtonValid = true;
					PlaySoundIn2D(m_pButtonCorrectItemEnteredSound);

					if (m_CubeButtonValid && m_BallButtonValid) OpenDoor();
				}
				else if (action == GameObject::TriggerAction::LEAVE)
				{
					{
						m_BallButtonValid = false;
						PlaySoundIn2D(m_pButtonCorrectItemLeftSound);

						if (m_IsDoorOpen) CloseDoor();					}
				}
			}
			else if (action == GameObject::TriggerAction::ENTER)	PlaySoundIn2D(m_pWhatAreYouDoingSound);
		}
	);
	pColliderButtonBall->SetTag(L"BallKey");
	buttonBall->AddChild(pColliderButtonBall);

	//setup transform 
	const float scaleButtonBall = 0.15f;
	buttonBall->GetTransform()->Scale(scaleButtonBall, scaleButtonBall, scaleButtonBall);
	buttonBall->GetTransform()->Translate(positionBallButton);

	//setup collisions
	auto pRigidButtonBall{ new RigidBodyComponent{true } };
	pRigidButtonBall->SetCollisionGroup(CollisionGroupFlag::Group2);
	buttonBall->AddComponent(pRigidButtonBall);

	physx::PxTriangleMesh* pButtonBallMesh = ContentManager::Load<physx::PxTriangleMesh>(L"Resources/Meshes/ButtonBall.ovpt");
	std::shared_ptr<physx::PxGeometry> sp_ButtonBallGeometry(new physx::PxTriangleMeshGeometry{ pButtonBallMesh });

	buttonBall->AddComponent(new ColliderComponent(sp_ButtonBallGeometry, *pMatDefault));

	AddChild(buttonBall);
	pRigidButtonBall->Scale(scaleButtonBall);


	//Switch
//************
	auto switchObject = new GameObject();
	DirectX::XMFLOAT3 switchPos{ 50.f, 25.f, 80.f };
	ModelComponent* pModelSwitch = new ModelComponent{ L"Resources/Meshes/Switch.ovm" };
	switchObject->AddComponent(pModelSwitch);

	auto diffuseSwitch = new DiffuseMaterial_Shadow{};
	diffuseSwitch->SetDiffuseTexture(L"Resources/Textures/Switch.png");
	diffuseSwitch->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());

	gameContext.pMaterialManager->AddMaterial(diffuseSwitch, gameContext.pMaterialManager->GetMaterialId());

	pModelSwitch->SetMaterial(gameContext.pMaterialManager->GetMaterialId());
	gameContext.pMaterialManager->IncrementMaterialId();

	//add a triggers collider
	GameObject* pColliderSwitch = new GameObject();

	pColliderSwitch->GetTransform()->Translate(switchPos);
	const float colliderSize{ 8.f };
	std::shared_ptr<physx::PxGeometry> sp_SwitchCollider(new physx::PxBoxGeometry{ colliderSize,colliderSize , colliderSize });

	auto triggerColliderSwitch{ new ColliderComponent(sp_SwitchCollider, *pMatDefault) };
	triggerColliderSwitch->EnableTrigger(true);

	pColliderSwitch->AddComponent(triggerColliderSwitch);
	pColliderSwitch->AddComponent(new RigidBodyComponent(true));

	pColliderSwitch->SetOnTriggerCallBack([this](GameObject* triggerObject, GameObject* otherObject, GameObject::TriggerAction action)
		{
			UNREFERENCED_PARAMETER(triggerObject);

			if (otherObject->GetTag() == L"Player")
			{
				if (action == GameObject::TriggerAction::ENTER)	m_PlayerInSwitchRange = true;
				else if (action == GameObject::TriggerAction::LEAVE)	m_PlayerInSwitchRange = false;
			}
		}
	);
	pColliderSwitch->GetComponent<RigidBodyComponent>()->SetCollisionGroup(CollisionGroupFlag::Group9);
	switchObject->AddChild(pColliderSwitch);

	//setup transform
	const float scaleSwitch = 0.12f;
	switchObject->GetTransform()->Scale(scaleSwitch, scaleSwitch, scaleSwitch);
	switchObject->GetTransform()->Translate(switchPos);

	switchObject->AddComponent(new RigidBodyComponent{ true });

	std::shared_ptr<physx::PxGeometry> sp_SwitchGeometry(new physx::PxBoxGeometry{ 7.f,60.f, 7.f });//box to save performance

	switchObject->AddComponent(new ColliderComponent(sp_SwitchGeometry, *pMatDefault));

	AddChild(switchObject);
	switchObject->GetComponent<RigidBodyComponent>()->Scale(scaleSwitch);
}
