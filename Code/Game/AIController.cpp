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
		Actor* closestEnemy = nullptr;
		float distanceFromActorSQ = 1000000.f;

		for (int actorIndex = 0; actorIndex < m_map->m_actors.size(); ++actorIndex)
		{
			Actor* testActor = m_map->m_actors[actorIndex];

			if (testActor == nullptr) continue;

			if (testActor->m_definition->m_faction != "Marine") continue;

			Vec2 actor2DPos = Vec2(testActor->m_position.x, testActor->m_position.y);

			if (IsPointInsideOrientedSector2D(actor2DPos, Vec2(GetActor()->m_position.x, GetActor()->m_position.y), GetActor()->m_orientation.m_yawDegrees,
				GetActor()->m_definition->m_sightAngle, GetActor()->m_definition->m_sightRadius))
			{
				float distanceSQ = GetDistanceSquared3D(GetActor()->m_position, testActor->m_position);
				if ( distanceSQ < distanceFromActorSQ)
				{
					closestEnemy = testActor;
					distanceFromActorSQ = distanceSQ;
				}
			}
		}
		if (closestEnemy != nullptr)
		{
			target = closestEnemy;
		}
		else return;
	}

	Vec3 forward = GetActor()->m_orientation.GetForwardDir_IFwd_JLeft_KUp();
	forward.z = 0.f;
	forward = forward.GetNormalized();
	float speed = 0.f;

	// add an if statement later for if has target or is just wandering
	//{
		speed = GetActor()->m_definition->m_runSpeed;
	//}
// 	else
// 	{
// 		speed = GetActor()->m_definition->m_walkSpeed;
// 	}

	GetActor()->TurnInDirection(target->m_position, deltaSeconds);
	GetActor()->MoveInDirection(forward, speed);
}
