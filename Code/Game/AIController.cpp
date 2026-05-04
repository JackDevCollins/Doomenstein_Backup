#include "Game/AIController.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"

void AIController::DamagedBy(Actor* damageSource)
{
	m_targetActorHandle = damageSource->m_handle;
}

void AIController::Update(float deltaSeconds)
{
	Actor* target = m_map->GetActorByHandle(m_targetActorHandle);

	if (target == nullptr)
	{
		target = m_map->GetClosestVisibleEnemy(GetActor()->m_handle);
	}

	Vec3 forward = GetActor()->m_orientation.GetForwardDir_IFwd_JLeft_KUp();
	forward.z = 0.f;
	forward = forward.GetNormalized();
	float speed = 0.f;

	if (target != nullptr)
	{
		Vec2 selfVec2Pos = Vec2(GetActor()->m_position.x, GetActor()->m_position.y);
		Vec2 targetVec2Pos = Vec2(target->m_position.x,target->m_position.y);
		if (DoDiscsOverlap(selfVec2Pos, GetActor()->m_definition->m_collision_radius * 1.01f, targetVec2Pos, target->m_definition->m_collision_radius * 1.01f))
		{
			GetActor()->TurnInDirection(target->m_position, deltaSeconds);
			GetActor()->Attack();
			return;
		}
		
	// add an if statement later for if has target or is just wandering
	//{
		speed = GetActor()->m_definition->m_runSpeed;
	//}
// 	else
// 	{
// 		speed = GetActor()->m_definition->m_walkSpeed;
// 	}

		//GetActor()->TurnInDirection(target->m_position, deltaSeconds);

		//GetActor()->MoveInDirection(forward, speed);
	}
}
