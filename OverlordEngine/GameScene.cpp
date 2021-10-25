#include "stdafx.h"
#include "GameScene.h"
#include "GameObject.h"
#include "SceneManager.h"
#include "OverlordGame.h"
#include "Prefabs.h"
#include "Components.h"
#include "DebugRenderer.h"
#include "RenderTarget.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "PhysxProxy.h"
#include "SoundManager.h"
#include <algorithm>
#include "PostProcessingMaterial.h"
#include "CombineResources.h"
#include "../Portal/PortalPrefab.h"

using namespace DirectX;

float GameScene::m_Volume = 0.15f;

GameScene::GameScene(std::wstring sceneName) :
	m_pChildren(std::vector<GameObject*>()),
	m_pPortals(std::vector<PortalPrefab*>()),
	m_GameContext(GameContext()),
	m_IsInitialized(false),
	m_UsePostProcessing{ true },
	m_SceneName(std::move(sceneName)),
	m_pDefaultCamera(nullptr),
	m_pActiveCamera(nullptr),
	m_pPhysxProxy(nullptr),
	m_pChannel(nullptr),
	m_pChannelHoldingItem(nullptr),
	m_pBackGroundChannel(nullptr),
	m_pPostProcessingEffects{},
	m_pFmodSystem{ SoundManager::GetInstance()->GetSystem() }
{
	m_pChildren.reserve(100);
}

void GameScene::Initialize()
{
	// Input Actions 
	InputAction increaseVolume{ GameScene::GameInput::INCVOLUME,InputTriggerState::Down, VK_OEM_PLUS,0 ,0,{} };
	InputAction decreaseVolume{ GameScene::GameInput::DECVOLUME,InputTriggerState::Down, VK_OEM_MINUS,0 ,0,{} };

	m_GameContext.pInput->AddInputAction(increaseVolume);
	m_GameContext.pInput->AddInputAction(decreaseVolume);
}
void GameScene::Update()
{
	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));
	const DWORD dwResult = XInputGetState(0, &state);

	float leftTrigger = (float)state.Gamepad.bLeftTrigger / 255;
	float rightTrigger = (float)state.Gamepad.bRightTrigger / 255;
	const float triggerValue{ 0.5f };

	if (m_Volume < 1.f && (m_GameContext.pInput->IsActionTriggered((int)GameInput::INCVOLUME)) || rightTrigger > triggerValue)
	{
		m_Volume += 0.5f * m_GameContext.pGameTime->GetElapsed();
		if (m_Volume > 1.f) m_Volume = 1.f;
	}

	if (m_Volume > 0.f && (m_GameContext.pInput->IsActionTriggered((int)GameInput::DECVOLUME)) || leftTrigger > triggerValue)
	{
		m_Volume -= 0.5f * m_GameContext.pGameTime->GetElapsed();
		if (m_Volume < 0.f) m_Volume = 0.f;
	}
	SetVolume();
}

void GameScene::PauseSounds(bool value)
{
	m_pChannel->setPaused(value);
	m_pChannelHoldingItem->setPaused(value);
	m_pBackGroundChannel->setPaused(value);
}
void GameScene::PlaySoundIn2D(FMOD::Sound* pSound)
{
	FMOD_RESULT fmodResult;

	fmodResult = m_pFmodSystem->playSound(pSound, 0, false, &m_pChannel);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);
}
void GameScene::PlaySoundItemHolding(FMOD::Sound* pSound)
{
	FMOD_RESULT fmodResult;

	fmodResult = m_pFmodSystem->playSound(pSound, 0, false, &m_pChannelHoldingItem);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);
}
void GameScene::StopSoundItemHolding()
{
	m_pChannelHoldingItem->setPaused(true);
}

GameScene::~GameScene()
{
	SafeDelete(m_GameContext.pGameTime);
	SafeDelete(m_GameContext.pInput);
	SafeDelete(m_GameContext.pMaterialManager);
	SafeDelete(m_GameContext.pShadowMapper);

	for (int i = m_pChildren.size() -1; i >= 0; --i)
	{
		SafeDelete(m_pChildren[i]);
	}

	SafeDelete(m_pPhysxProxy);

	for (PostProcessingMaterial* pEffect : m_pPostProcessingEffects)
	{
		SafeDelete(pEffect);
	}
}

void GameScene::AddChild(GameObject* obj)
{
#if _DEBUG
	if (obj->m_pParentScene)
	{
		if (obj->m_pParentScene == this)
			Logger::LogWarning(L"GameScene::AddChild > GameObject is already attached to this GameScene");
		else
			Logger::LogWarning(
				L"GameScene::AddChild > GameObject is already attached to another GameScene. Detach it from it's current scene before attaching it to another one.");

		return;
	}

	if (obj->m_pParentObject)
	{
		Logger::LogWarning(
			L"GameScene::AddChild > GameObject is currently attached to a GameObject. Detach it from it's current parent before attaching it to another one.");
		return;
	}
#endif

	obj->m_pParentScene = this;
	obj->RootInitialize(m_GameContext);
	m_pChildren.push_back(obj);

	if (typeid(*obj) == typeid(PortalPrefab)) m_pPortals.push_back(reinterpret_cast<PortalPrefab*>(obj));
}

void GameScene::RemoveChild(GameObject* obj, bool deleteObject)
{
	const auto it = find(m_pChildren.begin(), m_pChildren.end(), obj);

#if _DEBUG
	if (it == m_pChildren.end())
	{
		Logger::LogWarning(L"GameScene::RemoveChild > GameObject to remove is not attached to this GameScene!");
		return;
	}
#endif

	m_pChildren.erase(it);
	if (typeid(*obj) == typeid(PortalPrefab))
	{
		const auto itPortals = find(m_pPortals.begin(), m_pPortals.end(), reinterpret_cast<PortalPrefab*>(obj));
		m_pPortals.erase(itPortals);
	}

	if (deleteObject)
	{
		delete obj;
		obj = nullptr;
	}
	else
		obj->m_pParentScene = nullptr;
}

void GameScene::AddPostProcessingEffect(PostProcessingMaterial* pEffect)
{
	if (std::find(m_pPostProcessingEffects.cbegin(), m_pPostProcessingEffects.cend(), pEffect) == m_pPostProcessingEffects.cend())
	{
		pEffect->Initialize(m_GameContext);

		m_pPostProcessingEffects.push_back(pEffect);
	}
}

void GameScene::RemovePostProcessingEffect(PostProcessingMaterial* pEffect)
{
	std::vector< PostProcessingMaterial*>::iterator it = std::find(m_pPostProcessingEffects.begin(), m_pPostProcessingEffects.end(), pEffect);

	if (pEffect)	delete pEffect;
	if (it != m_pPostProcessingEffects.cend()) m_pPostProcessingEffects.erase(it);
}

void GameScene::UsePostProcessingEffects(bool value)
{
	m_UsePostProcessing = value;
}

void GameScene::RootInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
	if (m_IsInitialized)
		return;

	//Create DefaultCamera
	auto freeCam = new FreeCamera();
	freeCam->SetRotation(30, 0);
	freeCam->GetTransform()->Translate(0, 50, -80);
	AddChild(freeCam);
	m_pDefaultCamera = freeCam->GetComponent<CameraComponent>();
	m_pActiveCamera = m_pDefaultCamera;
	m_GameContext.pCamera = m_pDefaultCamera;

	//Create GameContext
	m_GameContext.pGameTime = new GameTime();
	m_GameContext.pGameTime->Reset();
	m_GameContext.pGameTime->Stop();

	m_GameContext.pInput = new InputManager();
	InputManager::Initialize();

	m_GameContext.pMaterialManager = new MaterialManager();
	m_GameContext.pShadowMapper = new ShadowMapRenderer();

	m_GameContext.pDevice = pDevice;
	m_GameContext.pDeviceContext = pDeviceContext;

	//Initialize ShadowMapper
	m_GameContext.pShadowMapper->Initialize(m_GameContext);

	// Initialize Physx
	m_pPhysxProxy = new PhysxProxy();
	m_pPhysxProxy->Initialize(this);

	//User-Scene Initialize
	Initialize();

	//Root-Scene Initialize
	for (auto pChild : m_pChildren)
	{
		pChild->RootInitialize(m_GameContext);
	}

	//post processing
	for (PostProcessingMaterial* pEffect : m_pPostProcessingEffects)
	{
		pEffect->Initialize(m_GameContext);
	}

	m_IsInitialized = true;
}

void GameScene::RootUpdate()
{
	m_GameContext.pGameTime->Update();
	m_GameContext.pInput->Update();

	SoundManager::GetInstance()->GetSystem()->update();

	//User-Scene Update
	Update();

	//Root-Scene Update
	for (auto pChild : m_pChildren)
	{
		pChild->RootUpdate(m_GameContext);
	}

	//post processing
	if (m_UsePostProcessing)
	{
		std::sort(m_pPostProcessingEffects.begin(), m_pPostProcessingEffects.end(),
			[](const PostProcessingMaterial* pPPMat1, const PostProcessingMaterial* pPPMat2) { return pPPMat1->GetRenderIndex() < pPPMat2->GetRenderIndex(); });

		for (PostProcessingMaterial* pPPMat : m_pPostProcessingEffects)
		{
			pPPMat->Update(m_GameContext);
		}
	}

	m_pPhysxProxy->Update(m_GameContext);
}

void GameScene::RootDraw()
{
	m_GameContext.pShadowMapper->Begin(m_GameContext);
	for (auto pChild : m_pChildren)
	{
		pChild->RootDrawShadowMap(m_GameContext);
	}
	m_GameContext.pShadowMapper->End(m_GameContext);

	for (PortalPrefab* pPortal : m_pPortals)
	{
		if (pPortal->IsActive())
		{
			pPortal->BeginPortalRendering(m_GameContext);

			DrawScene(pPortal);

			pPortal->EndPortalRendering();
		}
	}
	//draw the scene itself
	DrawScene();


	//Draw PhysX
	m_pPhysxProxy->Draw(m_GameContext);

	//Draw Debug Stuff
	DebugRenderer::Draw(m_GameContext);
	SpriteRenderer::GetInstance()->Draw(m_GameContext);
	TextRenderer::GetInstance()->Draw(m_GameContext);

	//post processing
	if (!m_pPostProcessingEffects.empty() && m_UsePostProcessing)
	{
		OverlordGame* pGame{ SceneManager::GetInstance()->GetGame() };

		RenderTarget* INIT_RT = pGame->GetRenderTarget();
		RenderTarget* PREV_RT = INIT_RT;

		for (PostProcessingMaterial* pPPMat : m_pPostProcessingEffects)
		{
			if (typeid(*pPPMat) == typeid(CombineResources))
			{
				reinterpret_cast<CombineResources*>(pPPMat)->SetSecondTexture(INIT_RT);
			}

			RenderTarget* TEMP_RT{ pPPMat->GetRenderTarget() };
			pGame->SetRenderTarget(TEMP_RT);

			pPPMat->Draw(m_GameContext, PREV_RT);
			PREV_RT = TEMP_RT;
		}
		pGame->SetRenderTarget(INIT_RT);
		SpriteRenderer::GetInstance()->DrawImmediate(m_GameContext, PREV_RT->GetShaderResourceView(), DirectX::XMFLOAT2{});
	}
}

void GameScene::RootSceneActivated()
{
	//Start Timer
	m_GameContext.pGameTime->Start();
	SceneActivated();
}

void GameScene::RootSceneDeactivated()
{
	//Stop Timer
	m_GameContext.pGameTime->Stop();
	SceneDeactivated();

	m_pChannel->stop();
	m_pBackGroundChannel->stop();
	m_pChannelHoldingItem->stop();
}

void GameScene::RootWindowStateChanged(int state, bool active) const
{
	//TIMER
	if (state == 0)
	{
		if (active)m_GameContext.pGameTime->Start();
		else m_GameContext.pGameTime->Stop();
	}
}

 void GameScene::SetVolume()
{
	m_pChannel->setVolume(m_Volume);
	m_pChannelHoldingItem->setVolume(m_Volume);
	m_pBackGroundChannel->setVolume(m_Volume / 2.5f);//we want this to be more subtle
}

void GameScene::DrawScene(PortalPrefab* pPortal)
{
	const float dotCutoffValue{ -0.1f };
	//User-Scene Draw
	Draw();

	//Object-Scene Draw
	for (auto pChild : m_pChildren)
	{
		//we don't wanna render the things that are behind the portal and block its view
		if (pPortal != nullptr)
		{
			if (typeid(*pChild) == typeid(PortalPrefab)) continue;
			if (pChild == pPortal->GetWall()->GetParent()) continue;
		}
		pChild->RootDraw(m_GameContext);
	}
	//Object-Scene Post-Draw
	for (auto pChild : m_pChildren)
	{
		//we don't wanna render the things that are behind the portal and block its view
		if (pPortal != nullptr)
		{
			if (typeid(*pChild) == typeid(PortalPrefab)) continue;
			if (pChild == pPortal->GetWall()->GetParent()) continue;
		}
		pChild->RootPostDraw(m_GameContext);
	}
}

void GameScene::SetActiveCamera(CameraComponent* pCameraComponent, bool storeOldOne)
{
	if (m_pActiveCamera != nullptr)
		m_pActiveCamera->m_IsActive = false;

	if (storeOldOne && pCameraComponent != nullptr) m_pDefaultCamera = m_pActiveCamera;
	m_pActiveCamera = (pCameraComponent) ? pCameraComponent : m_pDefaultCamera;

	m_pActiveCamera->m_IsActive = true;

	m_GameContext.pCamera = m_pActiveCamera;
}
