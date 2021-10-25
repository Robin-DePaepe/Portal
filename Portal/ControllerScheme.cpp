#include "stdafx.h"
#include "ControllerScheme.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "SpriteComponent.h"
#include "SoundManager.h"

ControllerScheme::ControllerScheme(const std::wstring& previousScene, FMOD::Channel* pChannel, FMOD::Channel* pBackGroundChannel, const std::wstring& sceneName)
	:GameScene{ sceneName },
	m_pBackSound{nullptr},
	m_PreviousSceneName{previousScene}
{
	m_pChannel = pChannel;
	m_pBackGroundChannel = pBackGroundChannel;
}

void ControllerScheme::Initialize()
{
	GameScene::Initialize();

	auto pScheme = new GameObject{};
	pScheme->AddComponent(new SpriteComponent(L"./Resources/Textures/Menu/ControllerScheme.png", DirectX::XMFLOAT2(0.f, 0.f), DirectX::XMFLOAT4(1, 1, 1, 1.f)));

	AddChild(pScheme);

	// Input Actions 
	InputAction back{ GameScene::GameInput::BACK,InputTriggerState::Pressed, VK_ESCAPE ,0 ,XINPUT_GAMEPAD_B,{} };

	GetGameContext().pInput->AddInputAction(back);

	//SOUND
	auto pFmodSystem = SoundManager::GetInstance()->GetSystem();
	FMOD_RESULT fmodResult;

	fmodResult = pFmodSystem->createStream("Resources/Sounds/menu_back.wav", FMOD_LOOP_OFF, 0, &m_pBackSound);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);
}

void ControllerScheme::Update()
{
	GameScene::Update();

	if (GetGameContext().pInput->IsActionTriggered((int)GameInput::BACK))
	{
		PlaySoundIn2D(m_pBackSound);
		SceneManager::GetInstance()->SetActiveGameScene(m_PreviousSceneName);
	}
}
