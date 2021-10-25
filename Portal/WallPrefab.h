#pragma once
#include "GameObject.h"

class WallPrefab final : public GameObject
{
public:
	//rule of 5
	WallPrefab(const DirectX::XMVECTOR& rotation, const DirectX::XMFLOAT3& wallPos, const physx::PxVec3& wallSize, const float textureScale, bool isPortalWall = true);
	 ~WallPrefab() = default;

	 WallPrefab(const WallPrefab& other) = delete;
	 WallPrefab(WallPrefab&& other) noexcept = delete;
	 WallPrefab& operator=(const WallPrefab& other) = delete;
	 WallPrefab& operator=(WallPrefab&& other) noexcept = delete;

	 //public functions
	 void RemoveRigidFromScene();
	 void AddRigidToScene();

protected:
	void Initialize(const GameContext&) override;

private:
	//datamembers
	const float m_TextureScale;
	const physx::PxVec3 m_WallSize;
	const bool m_IsPortalWall;
};

