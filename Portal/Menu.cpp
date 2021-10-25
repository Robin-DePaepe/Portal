#include "stdafx.h"
#include "Menu.h"
#include "MenuButton.h"
#include "SoundManager.h"

Menu::Menu(const std::wstring& sceneName)
	:GameScene{ sceneName },
	m_MenuSelectionIterator{ 0 },
	m_pSelectSound{ nullptr },
	m_pNextSound{ nullptr }
{}

void Menu::AddButton(MenuButton* pButton)
{
	m_pMenuButtons.push_back(pButton);

	AddChild(pButton);

	if (m_pMenuButtons.size() == 1) pButton->SetIsSelected(true);
}

void Menu::Initialize()
{
	GameScene::Initialize();

	// Input Actions 
	InputAction selectionUp{ GameScene::GameInput::SELECTIONUP,InputTriggerState::Pressed, VK_DOWN,0 ,XINPUT_GAMEPAD_DPAD_DOWN,{} };
	InputAction selectionDown{ GameScene::GameInput::SELECTIONDOWN,InputTriggerState::Pressed, VK_UP,0 ,XINPUT_GAMEPAD_DPAD_UP,{} };
	InputAction select{ GameScene::GameInput::SELECT,InputTriggerState::Pressed, VK_SPACE,0 ,XINPUT_GAMEPAD_A,{} };

	GetGameContext().pInput->AddInputAction(select);
	GetGameContext().pInput->AddInputAction(selectionDown);
	GetGameContext().pInput->AddInputAction(selectionUp);

	//SOUND
	auto pFmodSystem = SoundManager::GetInstance()->GetSystem();
	FMOD_RESULT fmodResult;

	fmodResult = pFmodSystem->createStream("Resources/Sounds/menu_accept.wav", FMOD_LOOP_OFF, 0, &m_pSelectSound);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);

	fmodResult = pFmodSystem->createStream("Resources/Sounds/next.wav", FMOD_LOOP_OFF, 0, &m_pNextSound);
	SoundManager::GetInstance()->ErrorCheck(fmodResult);
}

void Menu::Update()
{
	if (m_pMenuButtons.size() == 0) Logger::LogError(L"You need to add menubuttons if you are using the menu prefab.");

	GameScene::Update();

	if (GetGameContext().pInput->IsActionTriggered((int)GameInput::SELECT))
	{
		PlaySoundIn2D(m_pSelectSound);
		m_pMenuButtons[m_MenuSelectionIterator]->ButtonClicked();
	}
	if (GetGameContext().pInput->IsActionTriggered((int)GameInput::SELECTIONDOWN))
	{
		m_pMenuButtons[m_MenuSelectionIterator]->SetIsSelected(false);
		if (m_MenuSelectionIterator == 0) m_MenuSelectionIterator = m_pMenuButtons.size() - 1;
		else --m_MenuSelectionIterator;
		m_pMenuButtons[m_MenuSelectionIterator]->SetIsSelected(true);

		PlaySoundIn2D(m_pNextSound);
	}
	if (GetGameContext().pInput->IsActionTriggered((int)GameInput::SELECTIONUP))
	{
		m_pMenuButtons[m_MenuSelectionIterator]->SetIsSelected(false);
		if (m_MenuSelectionIterator == m_pMenuButtons.size() - 1) m_MenuSelectionIterator = 0;
		else ++m_MenuSelectionIterator;
		m_pMenuButtons[m_MenuSelectionIterator]->SetIsSelected(true);

		PlaySoundIn2D(m_pNextSound);
	}
}
