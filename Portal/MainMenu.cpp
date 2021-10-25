#include "stdafx.h"
#include "MainMenu.h"
#include "SpriteComponent.h"
#include "Components.h"
#include "MenuButton.h"
#include "SoundManager.h"
#include "SceneManager.h"
#include "ControllerScheme.h"
#include "PortalPuzzle1.h"
#include "FixedCamera.h"

MainMenu::MainMenu(const std::wstring& sceneName)
	:Menu(sceneName),
	m_pBackgroundMusic{ nullptr },
	m_pStartSound{ nullptr },
	m_ControllerSchemeName{ L"Controller scheme main menu" },
	m_pCam{ nullptr }
{}

void MainMenu::Initialize()
{
	Menu::Initialize();

	//setup a fixed camera
	auto pCamera{ new FixedCamera{} };
	AddChild(pCamera);
	m_pCam = pCamera->GetComponent<CameraComponent>();

	//adding the background
	GameObject* pBackgroundScreen = new GameObject();
	pBackgroundScreen->AddComponent(new SpriteComponent(L"./Resources/Textures/Menu/MainMenuBackground.png", DirectX::XMFLOAT2(0.f, 0.f), DirectX::XMFLOAT4(1, 1, 1, 1.f)));

	AddChild(pBackgroundScreen);

	//adding buttons
	auto startButton{ new MenuButton{ L"./Resources/Textures/Menu/StartButton.png", L"./Resources/Textures/Menu/StartButtonSelected.png" } };
	startButton->SetOnButtonClicked([this]() {
		PauseSounds(true);
		SceneManager::GetInstance()->SetActiveGameScene(L"Level1");
		});
	AddButton(startButton);

	auto controlsButton{ new MenuButton{ L"./Resources/Textures/Menu/ControlsButton.png", L"./Resources/Textures/Menu/ControlsButtonSelected.png" } };
	controlsButton->SetOnButtonClicked([this]() { SceneManager::GetInstance()->SetActiveGameScene(m_ControllerSchemeName); });
	AddButton(controlsButton);

	auto quitButton{ new MenuButton{ L"./Resources/Textures/Menu/QuitButton.png", L"./Resources/Textures/Menu/QuitButtonSelected.png" } };
	quitButton->SetOnButtonClicked([]() { PostQuitMessage(0); });
	AddButton(quitButton);


	//SOUND
	auto pFmodSystem = SoundManager::GetInstance()->GetSystem();
	FMOD_RESULT fmodResult;

	//background music
	fmodResult = pFmodSystem->createStream("Resources/Sounds/portal2_background01.wav", FMOD_DEFAULT, 0, &m_pBackgroundMusic);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->playSound(m_pBackgroundMusic, 0, false, &m_pBackGroundChannel);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	//start sound
	fmodResult = pFmodSystem->createStream("Resources/Sounds/startupGame.wav", FMOD_LOOP_OFF, 0, &m_pStartSound);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	PlaySoundIn2D(m_pStartSound);
	PauseSounds(true);

	//add the controller scheme
	SceneManager::GetInstance()->AddGameScene(new ControllerScheme(GetSceneName(), m_pChannel, m_pBackGroundChannel, m_ControllerSchemeName));

	PauseSounds(true);
}


void MainMenu::SceneActivated()
{
	std::cout << "\nREAD THIS: \nUse the arrows, spacebar and ESC button to naviage in the menu. \nThe + and - buttons control the volume.\n\n";

	if (!SceneManager::GetInstance()->IsScenePresent(L"Level1"))	SceneManager::GetInstance()->AddGameScene(new PortalPuzzle1());
	GetGameContext().pInput->ForceMouseToCenter(false);

	PauseSounds(false);

	m_pCam->SetActive();
}
