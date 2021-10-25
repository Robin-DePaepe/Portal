#include "stdafx.h"
#include "ModelAnimator.h"

using namespace DirectX;

ModelAnimator::ModelAnimator(MeshFilter* pMeshFilter) :
	m_pMeshFilter(pMeshFilter),
	m_Transforms(std::vector<DirectX::XMFLOAT4X4>()),
	m_IsPlaying(false),
	m_Reversed(false),
	m_ClipSet(false),
	m_TickCount(0),
	m_AnimationSpeed(1.0f),
	m_GunSocketTransform{},
	m_PlayClipOnEnd{ false }
{
	SetAnimation(0);
}

void ModelAnimator::SetAnimation(UINT clipNumber, bool reset)
{
	//TODO: complete
	m_ClipSet = false;

	if (clipNumber < m_pMeshFilter->m_AnimationClips.size())
	{
		SetAnimation(m_pMeshFilter->m_AnimationClips[clipNumber], reset);
	}
	else
	{
		Reset(true);
		Logger::LogWarning(L"Animation not found");
	}
}

void ModelAnimator::SetAnimation(std::wstring clipName, bool reset)
{
	//TODO: complete
	m_ClipSet = false;

	for (const AnimationClip& clip : m_pMeshFilter->m_AnimationClips)
	{
		if (clip.Name == clipName)
		{
			SetAnimation(clip, reset);
			return;
		}
	}
	Reset(true);
	Logger::LogWarning(L"Animation not found");
}

void ModelAnimator::SetAnimation(AnimationClip clip, bool reset)
{
	m_ClipSet = true;

	if (!m_PlayClipOnEnd) //we don't wanne disrupt this progress
	{
		if (m_CurrentClip.Name != clip.Name)
		{
			m_CurrentClip = clip;

			if (reset) Reset(false);
		}
	}
}

void ModelAnimator::Reset(bool pause)
{
	//TODO: complete
	if (pause) m_IsPlaying = false;

	m_TickCount = 0.f;
	m_AnimationSpeed = 1.f;

	//If m_ClipSet is true
	//	Retrieve the BoneTransform from the first Key from the current clip (m_CurrentClip)
	//	Refill the m_Transforms vector with the new BoneTransforms (have a look at vector::assign)
	//Else
	//	Create an IdentityMatrix 
	//	Refill the m_Transforms vector with this IdenityMatrix (Amount = BoneCount) (have a look at vector::assign)
	if (m_ClipSet)
	{
		m_Transforms.assign(m_CurrentClip.Keys[0].BoneTransforms.begin(), m_CurrentClip.Keys[0].BoneTransforms.end());
	}
	else
	{
		DirectX::XMFLOAT4X4 identityMatrix{};
		DirectX::XMStoreFloat4x4(&identityMatrix, DirectX::XMMatrixIdentity());
		m_Transforms.assign(m_pMeshFilter->m_BoneCount, identityMatrix);
	}
}

void ModelAnimator::PlayNewClipOnEnd(AnimationClip clipToPlay)
{
	m_PlayClipOnEnd = true;
	m_NextClip = clipToPlay;
}

void ModelAnimator::PlayNewClipOnEnd(UINT clipNumber)
{
	if (clipNumber < m_pMeshFilter->m_AnimationClips.size())
	{
		PlayNewClipOnEnd(m_pMeshFilter->m_AnimationClips[clipNumber]);
	}
	else
	{
		Logger::LogWarning(L"Animation not found");
	}
}

void ModelAnimator::PlayNewClipOnEnd(std::wstring clipName)
{
	for (const AnimationClip& clip : m_pMeshFilter->m_AnimationClips)
	{
		if (clip.Name == clipName)
		{
			PlayNewClipOnEnd(clip);
			return;
		}
	}
	Logger::LogWarning(L"Animation not found");
}

void ModelAnimator::Update(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);
	//TODO: complete
	//We only update the transforms if the animation is running and the clip is set
	if (m_IsPlaying && m_ClipSet)
	{
		//1. 
		//Calculate the passedTicks (see the lab document)
		//auto passedTicks = ...
		//Make sure that the passedTicks stay between the m_CurrentClip.Duration bounds (fmod)
		float passedTicks = gameContext.pGameTime->GetElapsed() * m_AnimationSpeed * m_CurrentClip.TicksPerSecond;
		if (passedTicks > m_CurrentClip.Duration) passedTicks = m_CurrentClip.Duration;

		//2. 
		//IF m_Reversed is true
		//	Subtract passedTicks from m_TickCount
		//	If m_TickCount is smaller than zero, add m_CurrentClip.Duration to m_TickCount
		//ELSE
		//	Add passedTicks to m_TickCount
		//	if m_TickCount is bigger than the clip duration, subtract the duration from m_TickCount
		if (m_Reversed)
		{
			m_TickCount -= passedTicks;
			if (m_TickCount < 0) m_TickCount += m_CurrentClip.Duration;
		}
		else
		{
			m_TickCount += passedTicks;

			if (m_TickCount > m_CurrentClip.Duration)
			{
				m_TickCount -= m_CurrentClip.Duration;
				if (m_PlayClipOnEnd)
				{
					m_PlayClipOnEnd = false;
					m_CurrentClip = m_NextClip;
				}
			}
		}
		//3.
		//Find the enclosing keys
		AnimationKey keyA{ m_CurrentClip.Keys[0] }, keyB{ m_CurrentClip.Keys[m_CurrentClip.Keys.size() - 1] };

		float smallerDistance{ -FLT_MAX };
		float biggerDistance{ FLT_MAX };
		//Iterate all the keys of the clip and find the following keys:
		//keyA > Closest Key with Tick before/smaller than m_TickCount
		//keyB > Closest Key with Tick after/bigger than m_TickCount
		for (const AnimationKey& key : m_CurrentClip.Keys)
		{
			const float distance = key.Tick - m_TickCount;
			if (distance > smallerDistance && distance <= 0.f)
			{
				smallerDistance = distance;
				keyA = key;
			}
			if (distance < biggerDistance && distance > 0.f)
			{
				biggerDistance = distance;
				keyB = key;
			}
		}
		//4.
		//Interpolate between keys
		//Figure out the BlendFactor (See lab document)
		//Clear the m_Transforms vector
		//FOR every boneTransform in a key (So for every bone)
		//	Retrieve the transform from keyA (transformA)
		//	auto transformA = ...
		// 	Retrieve the transform from keyB (transformB)
		//	auto transformB = ...
		//	Decompose both transforms
		//	Lerp between all the transformations (Position, Scale, Rotation)
		//	Compose a transformation matrix with the lerp-results
		//	Add the resulting matrix to the m_Transforms vector
		const float totalDistance{ abs(smallerDistance) + biggerDistance };

		const float blendFactor{ abs(smallerDistance) / totalDistance };

		m_Transforms.clear();

		for (size_t i = 0; i < m_pMeshFilter->m_BoneCount; i++)
		{
			auto transformA = XMLoadFloat4x4(&keyA.BoneTransforms[i]);
			auto transformB = XMLoadFloat4x4(&keyB.BoneTransforms[i]);

			XMVECTOR rotationA{}, translationA{}, scaleA{};
			XMVECTOR rotationB{}, translationB{}, scaleB{};

			DirectX::XMMatrixDecompose(&scaleA, &rotationA, &translationA, transformA);
			DirectX::XMMatrixDecompose(&scaleB, &rotationB, &translationB, transformB);

			XMVECTOR lerpedScale{ XMVectorLerp(scaleA,scaleB, blendFactor) };
			XMVECTOR lerpedTranslation{ XMVectorLerp(translationA,translationB, blendFactor) };
			XMVECTOR lerpedRotation{ XMQuaternionSlerp(rotationA,rotationB, blendFactor) };

			XMFLOAT4X4 lerpedTransform{};
			XMStoreFloat4x4(&lerpedTransform, XMMatrixTransformation({}, {}, lerpedScale, {}, lerpedRotation, lerpedTranslation));


			m_Transforms.push_back(lerpedTransform);

			if (i == 7)	m_GunSocketTransform = lerpedTransform; //this is a bone on the right hand
		}
	}
}
