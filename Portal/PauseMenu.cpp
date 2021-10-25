#include "stdafx.h"
#include "PauseMenu.h"
#include "FixedCamera.h"
#include "SpriteComponent.h"
#include "ControllerScheme.h"
#include "MenuButton.h"
#include "SceneManager.h"
#include "PortalPuzzle1.h"

PauseMenu::PauseMenu(const std::wstring& previousScene, FMOD::Channel* pChannel, FMOD::Channel* pBackGroundChannel, const std::wstring& sceneName)
	:Menu{ sceneName },
	m_ControllerSchemeName{ L"Controller scheme pause menu" },
	m_pCam{ nullptr },
	m_PreviousSceneName{ previousScene }
{
	m_pChannel = pChannel;
	m_pBackGroundChannel = pBackGroundChannel;
}

void PauseMenu::Initialize()
{
	Menu::Initialize();

	//setup a fixed camera
	auto pCamera{ new FixedCamera{} };
	AddChild(pCamera);
	m_pCam = pCamera->GetComponent<CameraComponent>();

	//adding the background
	GameObject* pBackgroundScreen = new GameObject();
	pBackgroundScreen->AddComponent(new SpriteComponent(L"./Resources/Textures/Menu/PauseMenuBackGround.png", DirectX::XMFLOAT2(0.f, 0.f), DirectX::XMFLOAT4(1, 1, 1, 1.f)));

	AddChild(pBackgroundScreen);

	//adding buttons
	auto continueButton{ new MenuButton{ L"./Resources/Textures/Menu/ContinueButton.png", L"./Resources/Textures/Menu/ContinueButtonSelected.png" } };
	continueButton->SetOnButtonClicked([this]() {	SceneManager::GetInstance()->SetActiveGameScene(m_PreviousSceneName); });
	AddButton(continueButton);

	auto controlsButton{ new MenuButton{ L"./Resources/Textures/Menu/ControlsButtonPauseMenu.png", L"./Resources/Textures/Menu/ControlsButtonSelectedPauseMenu.png" } };
	controlsButton->SetOnButtonClicked([this]() { SceneManager::GetInstance()->SetActiveGameScene(m_ControllerSchemeName); });
	AddButton(controlsButton);

	auto restartButton{ new MenuButton{ L"./Resources/Textures/Menu/RestartButton.png", L"./Resources/Textures/Menu/RestartButtonSelected.png" } };
	restartButton->SetOnButtonClicked([this]() {
		SceneManager::GetInstance()->RemoveGameScene(m_PreviousSceneName);
		SceneManager::GetInstance()->AddGameScene(new PortalPuzzle1());
		SceneManager::GetInstance()->SetActiveGameScene(L"Level1");
		SceneManager::GetInstance()->RemoveGameScene(this);
		});
	AddButton(restartButton);

	auto exitButton{ new MenuButton{ L"./Resources/Textures/Menu/ExitButton.png", L"./Resources/Textures/Menu/ExitButtonSelected.png" } };
	exitButton->SetOnButtonClicked([this]() {
		SceneManager::GetInstance()->SetActiveGameScene(L"Main menu");
		SceneManager::GetInstance()->RemoveGameScene(m_PreviousSceneName);
		SceneManager::GetInstance()->RemoveGameScene(this);
		});
	AddButton(exitButton);

	//add the controller scheme
	SceneManager::GetInstance()->AddGameScene(new ControllerScheme(GetSceneName(), m_pChannel, m_pBackGroundChannel, m_ControllerSchemeName));
}

void PauseMenu::SceneActivated()
{
	GetGameContext().pInput->ForceMouseToCenter(false);
	m_pCam->SetActive();
}
