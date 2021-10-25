#pragma once
#include "GameObject.h"

class BoneObject final : public GameObject
{
public:
	//rule of 5
	BoneObject(int boneId, int materialId, float length = 5.0f);
	~BoneObject() = default;

	BoneObject(const BoneObject& other) = delete;
	BoneObject(BoneObject&& other) noexcept = delete;
	BoneObject& operator=(const BoneObject& other) = delete;
	BoneObject& operator=(BoneObject&& other) noexcept = delete;

	//functions
	void AddBone(BoneObject* pBone);

	const DirectX::XMFLOAT4X4& GetBindPose() const;
	void CalculateBindPose();
protected:
	virtual void Initialize(const GameContext& gameContext);
private:
	//datamembers
	float m_Length;
	int m_BoneId;
	int m_MaterialId;

	DirectX::XMFLOAT4X4 m_BindPose;
};