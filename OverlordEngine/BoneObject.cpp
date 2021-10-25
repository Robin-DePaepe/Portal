#include "stdafx.h"
#include "BoneObject.h"
#include "Components.h"

BoneObject::BoneObject(int boneId, int materialId, float length)
	:m_Length{ length }
	, m_MaterialId{ materialId }
	, m_BoneId{ boneId }
{
}

void BoneObject::AddBone(BoneObject* pBone)
{
	pBone->GetTransform()->Translate(m_Length, 0.f, 0.f);
	AddChild(pBone);
}

const DirectX::XMFLOAT4X4& BoneObject::GetBindPose() const
{
	return m_BindPose;
}

void BoneObject::CalculateBindPose()
{
		DirectX::XMStoreFloat4x4(&m_BindPose, DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&GetTransform()->GetWorld())));

	for (auto bone : GetChildren<BoneObject>())
	{
		bone->CalculateBindPose();
	}
}

void BoneObject::Initialize(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);

	auto bone = new GameObject();

	ModelComponent* pModelBone = new ModelComponent{ L"Resources/Meshes/Bone.ovm" };
	pModelBone->SetMaterial(m_MaterialId);

	bone->AddComponent(pModelBone);
	bone->GetTransform()->Scale(m_Length, m_Length, m_Length);
	bone->GetTransform()->Rotate(0, -90, 0);

	AddChild(bone);
}
