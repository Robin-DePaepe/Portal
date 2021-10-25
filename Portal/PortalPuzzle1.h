#pragma once
#include <GameScene.h>

class RigidBodyComponent;
class PortalCharacterController;

class PortalPuzzle1 final:    public GameScene
{
public:
	//rule of 5
	PortalPuzzle1(std::wstring sceneName = L"Level1");
	virtual ~PortalPuzzle1();

	PortalPuzzle1(const PortalPuzzle1& other) = delete;
	PortalPuzzle1(PortalPuzzle1&& other) noexcept = delete;
	PortalPuzzle1& operator=(const PortalPuzzle1& other) = delete;
	PortalPuzzle1& operator=(PortalPuzzle1&& other) noexcept = delete;

protected:
	//functions
	void Initialize() override;
	void Update() override;
	void Draw()  override {};

	void SceneActivated() override;
	void SceneDeactivated() override;

private:
	//data members
	PortalCharacterController* m_pCharacter;
	RigidBodyComponent* m_pBallRigid;
	GameObject* m_pDoorClosed, * m_pDoorOpen;
	bool m_RemoveDoorCollision, m_enableDoorCollision, m_IsDoorOpen;
	bool m_PlayerInSwitchRange, m_CubeButtonValid, m_BallButtonValid;
	const std::wstring m_PauseMenuName;
	//SOUND
	FMOD::Sound* m_pButtonCorrectItemEnteredSound, * m_pButtonCorrectItemLeftSound;
	FMOD::Sound* m_pSwitchSucessfullyActivatedSound, * m_pSwitchUnSucessfullyActivatedSound;
	FMOD::Sound* m_pDoorOpeningSound, *m_pStartQuoteSound, *m_pWhatAreYouDoingSound;

	static bool m_FirstInit;

	//helpers
	void InitializetriggerObjects(const GameContext& context);
	void InitializePlayer();
	void InitializeSounds();
	void InitializeProps();
	void InitializeBuildingBlocks(const GameContext& context);

	void CloseDoor ();
	void OpenDoor();

};

