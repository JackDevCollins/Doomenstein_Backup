#include "Game/Controller.hpp"
#include "Game/ActorHandle.hpp"
#include "Game/Map.hpp"

void Controller::Possess()
{
	// get index from actorHandle and possess the next in the list. activate
	// AI controller restoration
}

Actor* Controller::GetActor()
{
	return m_map->GetActorByHandle(*m_handle);
}
