#include "stdafx.h"
#include "SelfDestructionComponent.h"
#include "GameObject.h"
#include "GameScene.h"

SelfDestructionComponent::SelfDestructionComponent(const float timeTillDestruction, bool isActive)
	:BaseComponent{},
	m_TimeTillDestruction{timeTillDestruction},
	m_IsActive{isActive}
{
}

void SelfDestructionComponent::Update(const GameContext& gameContext)
{
	if (m_IsActive)
	{
		m_TimeTillDestruction -= gameContext.pGameTime->GetElapsed();

		if (m_TimeTillDestruction <= 0.f)
		{
			GameObject* thisObject{ GetGameObject() };
			if (thisObject->GetScene() != nullptr) thisObject->GetScene()->RemoveChild(thisObject, true);
			else if (thisObject->GetParent() != nullptr)
			{
				thisObject->GetParent()->RemoveChild(thisObject);
				delete thisObject;
			}
			else Logger::LogError(L"SelfDestructionComponent could not destroy this object properly.");
		}
	}
}
