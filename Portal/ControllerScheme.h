#pragma once
#include "GameScene.h"

class ControllerScheme final : public GameScene
{
public:
	//rule of 5
	ControllerScheme(const std::wstring& previousScene, FMOD::Channel* pChannel, FMOD::Channel *pBackGroundChannel, const std::wstring& sceneName = L"Controller scheme");
	~ControllerScheme() = default;

	ControllerScheme(const ControllerScheme& other) = delete;
	ControllerScheme(ControllerScheme&& other) noexcept = delete;
	ControllerScheme& operator=(const ControllerScheme& other) = delete;
	ControllerScheme& operator=(ControllerScheme&& other) noexcept = delete;

protected:
	//functions
	void Initialize() override;
	void Update() override;
	void Draw() override {};

private:
//datamembers
	FMOD::Sound* m_pBackSound;
	const std::wstring m_PreviousSceneName;
};

