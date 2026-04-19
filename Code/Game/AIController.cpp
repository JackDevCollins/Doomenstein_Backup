#include "Game/AIController.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"

void AIController::DamagedBy(Actor* damageSource)
{
	m_targetActorHandle = damageSource->m_handle;
}

void AIController::Update(float deltaSeconds)
{
	if (m_targetActorHandle == ActorHandle::INVALID)
	{
		for (int index = 0; index < m_map->m_actors.size(); ++index)
		{
			if (m_map->m_actors[index]->m_definition->m_name == "Marine")
			{
				m_targetActorHandle = m_map->m_actors[index]->m_handle;
			}
		}
	}

	if (m_targetActorHandle != ActorHandle::INVALID)
	{
		Actor* target = m_map->GetActorByHandle(m_targetActorHandle);
		
		GetActor()->TurnInDirection(target->m_position, deltaSeconds);

	}
}
