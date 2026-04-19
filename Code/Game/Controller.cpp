#include "Game/Controller.hpp"
#include "Game/ActorHandle.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/App.hpp"
#include "Engine/Math/Mat44.hpp"

void Controller::Possess(ActorHandle HauntingActor)
{
	if (m_map == nullptr)
	{
		m_map = g_app->m_game->m_currentMap;
	}
	if (m_actorHandle == ActorHandle::INVALID && m_map->m_actors[HauntingActor.GetIndex()]->m_definition->m_canBePossessed)
	{
		m_actorHandle = HauntingActor;
		GetActor()->m_controller = this;
		GetActor()->OnPossessed();
	}

	// get index from actorHandle and possess the next in the list. activate
	// AI controller restoration
}

Actor* Controller::GetActor()
{
	if (m_map == nullptr)
	{
		m_map = g_app->m_game->m_currentMap;
	}

	return m_map->GetActorByHandle(m_actorHandle);
}


