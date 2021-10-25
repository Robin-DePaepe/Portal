#pragma once
#include "BaseComponent.h"
class SelfDestructionComponent :    public BaseComponent
{
public:
	SelfDestructionComponent(const float timeTillDestruction = 0.f, bool isActive = false);
	virtual ~SelfDestructionComponent() = default;

	SelfDestructionComponent(const SelfDestructionComponent& other) = delete;
	SelfDestructionComponent(SelfDestructionComponent&& other) noexcept = delete;
	SelfDestructionComponent& operator=(const SelfDestructionComponent& other) = delete;
	SelfDestructionComponent& operator=(SelfDestructionComponent&& other) noexcept = delete;

	void SetIsActive(const bool active) { m_IsActive = active; }
	void SetTimeTillDestruction(const float time) { m_TimeTillDestruction = time; }
protected:
	void Initialize(const GameContext& gameContext) override { UNREFERENCED_PARAMETER(gameContext); };
	void Update(const GameContext & gameContext) override;
	void Draw(const GameContext& gameContext) override { UNREFERENCED_PARAMETER(gameContext); };

private:
	 float m_TimeTillDestruction;
	 bool m_IsActive;
};



