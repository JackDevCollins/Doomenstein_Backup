#pragma once
#include "Game/Controller.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Rgba8.hpp"


class AIController : public Controller 
{
public:
	
ActorHandle			m_targetActorHandle = ActorHandle::INVALID;

void				DamagedBy(Actor* damageSource);
void				Update(float deltaSeconds);
};