#pragma once
#include "Menu.h"

class CameraComponent;

class PauseMenu final : public Menu
{
public:
	//rule of 5
	PauseMenu(const std::wstring& previousScene, FMOD::Channel* pChannel, FMOD::Channel* pBackGroundChannel, const std::wstring& sceneName = L"Pause menu");
	~PauseMenu() = default;

	PauseMenu(const PauseMenu& other) = delete;
	PauseMenu(PauseMenu&& other) noexcept = delete;
	PauseMenu& operator=(const PauseMenu& other) = delete;
	PauseMenu& operator=(PauseMenu&& other) noexcept = delete;

protected:
	//functions
	void Initialize() override;

	void SceneActivated() override;

private:
	const std::wstring m_ControllerSchemeName;
	CameraComponent* m_pCam;

	const std::wstring m_PreviousSceneName;
};

