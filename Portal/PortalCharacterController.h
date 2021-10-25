#pragma once
#include "GameObject.h"

class ControllerComponent;
class CameraComponent;
class ModelAnimator;
class ModelComponent;
class CenterGlow;
class PortalPrefab;

class PortalCharacterController final : public GameObject
{
public:
	//rule of 5
	PortalCharacterController(GameObject* pCrossHair, GameObject* pBluePortalHud, GameObject* pOrangePortalHud, float radius = 2, float height = 5, float moveSpeed = 100);
	~PortalCharacterController();

	PortalCharacterController(const PortalCharacterController& other) = delete;
	PortalCharacterController(PortalCharacterController&& other) noexcept = delete;
	PortalCharacterController& operator=(const PortalCharacterController& other) = delete;
	PortalCharacterController& operator=(PortalCharacterController&& other) noexcept = delete;

	//functions
	void Initialize(const GameContext& gameContext) override;
	void PostInitialize(const GameContext& gameContext) override;
	void Update(const GameContext& gameContext) override;

	GameObject* GetPickupSocket() const { return m_pPickupSocket; }
	bool IsDancing() const { return m_IsDancing; }
	const CameraComponent* GetCamera();

	void SetCameraActive();
	void AddRotation(float pitch, float yaw);

private:
	//datamembers
	CameraComponent* m_pCamera;
	ControllerComponent* m_pController;
	ModelComponent* m_pPortalGunModel;
	ModelComponent* m_pCharacterModelComp;

	GameObject* m_pPickupSocket, * m_pPortalGun;
	GameObject* m_pCharacterModel, * m_pCharacterModelXAxisInverse; //I can't rotate the character model seperately over both the x and y axis so I need a dummy parent to get the correct rotations
	ModelAnimator* m_pAnimator;

	float m_TotalPitch, m_TotalYaw, m_YawBeforeDance;
	float m_Radius, m_Height;
	const float m_MinPitch, m_MaxPitch;
	const float m_ModelRotationOffset;
	const float m_MoveSpeed, m_RotationSpeedMouse, m_RotationSpeedController;
	bool m_IsDancing;

	//PP
	CenterGlow* m_pGlowPPEffect;
	const float m_BallGlowRatio, m_CubeGlowRatio;

	//lerping
	const DirectX::XMFLOAT3 m_CameraPos, m_CameraRot, m_CameraPosDancing, m_CameraRotDancing;
	float m_CameraLerpDuration, m_CameraLerpAccTime;
	bool m_IsLerpingToDance, m_IsLerpingToFirstPerson;

	//hud 
	GameObject* m_pCrossHair, * m_pBluePortalShotHud, * m_pOrangePortalShotHud;

	//Running
	DirectX::XMFLOAT3 m_Velocity;

	float m_MaxRunVelocity,
		m_TerminalVelocity,
		m_Gravity,
		m_RunAccelerationTime,
		m_JumpAccelerationTime,
		m_RunAcceleration,
		m_JumpAcceleration,
		m_RunVelocity,
		m_JumpVelocity;

	//SOUND
	FMOD::Sound* m_pDanceTalk, * m_pIdleTalk;
	FMOD::Sound* m_pHoldingItem, * m_pDancing, * m_pPortalGunShotSucces, * m_pPortalGunShotFailed;

	const float m_IdleChatterInterval;
	float m_IdleChatterAccumTime;

	//Portals
	PortalPrefab* m_pBluePortal, * m_pRedPortal;

	//helper functions
	void AddPickup(const GameContext& gameContext);
	bool RemovePickup();//retuns if this went succesfull

	void FirePortal(const GameContext& gameContext, bool isBlue);
	void FailedPortalShot(const DirectX::XMFLOAT3& pos, bool isBlue);

	void LerpCamera(const GameContext& gameContext, bool& lerpingTo, const DirectX::XMFLOAT3& posGoto, const DirectX::XMFLOAT3& rotGoto, const DirectX::XMFLOAT3& posOriginal, const DirectX::XMFLOAT3& rotOriginal, bool disableModel);
	void Dance();
};


