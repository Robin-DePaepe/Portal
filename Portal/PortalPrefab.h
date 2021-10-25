#pragma once
#include "GameObject.h"

class PortalMaterial;
class RenderTarget;
class PortalCharacterController;

class PortalPrefab final : public GameObject
{
public:
	//rule of 5
	PortalPrefab(bool isBlue, const DirectX::XMFLOAT3& position, GameObject* pWall, PortalPrefab** pOtherPortal, PortalCharacterController* pPlayer, const float portalSize = 1.f);
	~PortalPrefab();

	PortalPrefab(const PortalPrefab& other) = delete;
	PortalPrefab(PortalPrefab&& other) noexcept = delete;
	PortalPrefab& operator=(const PortalPrefab& other) = delete;
	PortalPrefab& operator=(PortalPrefab&& other) noexcept = delete;

	//Getters
	static const float GetDefaultColliderSize() { return m_BoxColliderDefaultSize; }
	static const float GetHeightMultiplier() { return m_HeightMultiplier; }

	GameObject* GetWall() const { return m_pWall; }
	GameObject* GetCamera() const { return m_pPortalCameraObj; }
	bool IsActive();

	//helper functions
	void BeginPortalRendering(const GameContext& gameContext);
	void EndPortalRendering();

protected:
	void Initialize(const GameContext& gameContext) override;
	void Update(const GameContext&) override;

private:
	//datamembers
	const float m_pPortalSize;
	static const float m_BoxColliderDefaultSize;
	static const float m_HeightMultiplier;
	const bool m_IsBlue;
	static	bool m_PlayerInBluePortal, m_PlayerInRedPortal;
	static bool m_BluePortalEntered, m_BluePortalLeft;
	static	bool m_RedPortalEntered, m_RedPortalLeft;

	GameObject* m_pWall;
	GameObject* m_pPortalCameraObj;
	PortalPrefab** m_pOtherPortal;
	RenderTarget* m_pPortalRT;
	PortalMaterial* m_pPortalMat;
	PortalCharacterController* m_pPlayer;

	//helper
	float CalculateDotWithPlayer() const;
};

