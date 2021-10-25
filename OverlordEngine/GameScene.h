#pragma once

class PostProcessingMaterial;
class GameObject;
class SceneManager;
class CameraComponent;
class PhysxProxy;
class PortalPrefab;

class GameScene
{
public:
	enum GameInput : UINT
	{
		LEFT = 0,
		RIGHT,
		FORWARD,
		BACKWARD,
		JUMP,
		DANCE,
		INTERACT,
		FIRELEFT,
		FIRERIGHT,
		INCVOLUME,
		DECVOLUME,
		SELECTIONUP,
		SELECTIONDOWN,
		SELECT,
		BACK,
		PAUSE
	};
	GameScene(std::wstring sceneName);
	GameScene(const GameScene& other) = delete;
	GameScene(GameScene&& other) noexcept = delete;
	GameScene& operator=(const GameScene& other) = delete;
	GameScene& operator=(GameScene&& other) noexcept = delete;
	virtual ~GameScene();

	void AddChild(GameObject* obj);
	void RemoveChild(GameObject* obj, bool deleteObject = true);

	const GameContext& GetGameContext() const { return m_GameContext; }

	PhysxProxy* GetPhysxProxy() const { return m_pPhysxProxy; }
	void SetActiveCamera(CameraComponent* pCameraComponent, bool storeOldOne = true);

	void AddPostProcessingEffect(PostProcessingMaterial* pEffect);
	void RemovePostProcessingEffect(PostProcessingMaterial* pEffect);

	void UsePostProcessingEffects(bool value);

	void PauseSounds(bool value);
	void PlaySoundIn2D(FMOD::Sound* pSound);

	void PlaySoundItemHolding(FMOD::Sound* pSound);
	void StopSoundItemHolding();

	void DrawScene(PortalPrefab* pPortal = nullptr);

protected:
	virtual void Initialize();
	virtual void Update();
	virtual void Draw() = 0;

	virtual void SceneActivated() {}
	virtual void SceneDeactivated() {}

	const std::wstring& GetSceneName() const { return m_SceneName; }

	//datamembers
	std::vector< PostProcessingMaterial*> m_pPostProcessingEffects;

	//channels
	FMOD::Channel* m_pChannel, *m_pBackGroundChannel, * m_pChannelHoldingItem;
private:
	friend class SceneManager;

	void RootInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	void RootUpdate();
	void RootDraw();
	void RootSceneActivated();
	void RootSceneDeactivated();
	void RootWindowStateChanged(int state, bool active) const;
	void SetVolume();

	//datamembers
	std::vector<GameObject*> m_pChildren;
	std::vector<PortalPrefab*> m_pPortals;
	GameContext m_GameContext;
	bool m_IsInitialized,m_UsePostProcessing;
	std::wstring m_SceneName;
	CameraComponent* m_pDefaultCamera, * m_pActiveCamera;
	PhysxProxy* m_pPhysxProxy;

	//sound
	static float m_Volume;
	FMOD::System* m_pFmodSystem;
};
