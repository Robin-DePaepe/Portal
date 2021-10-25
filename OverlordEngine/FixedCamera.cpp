#include "stdafx.h"
#include "FixedCamera.h"
#include "TransformComponent.h"
#include "Components.h"
#include "PhysxManager.h"


void FixedCamera::Initialize(const GameContext&)
{
	AddComponent(new CameraComponent());
}