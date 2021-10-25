#pragma once
#include "Menu.h"

class CameraComponent;

class MainMenu final :  public Menu
{
public:
	//rule of 5
	MainMenu(const std::wstring& sceneName = L"Main menu");
	 ~MainMenu() = default;

	MainMenu(const MainMenu& other) = delete;
	MainMenu(MainMenu&& other) noexcept = delete;
	MainMenu& operator=(const MainMenu& other) = delete;
	MainMenu& operator=(MainMenu&& other) noexcept = delete;

protected:
	//functions
	void Initialize() override;
	void SceneActivated() override;

private:
	//datamembers
	FMOD::Sound* m_pBackgroundMusic, * m_pStartSound;
	const std::wstring m_ControllerSchemeName;

	CameraComponent* m_pCam;
};

