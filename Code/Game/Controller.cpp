#include "Game/Controller.hpp"
#include "Game/ActorHandle.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Engine/Math/Mat44.hpp"

void Controller::Possess(ActorHandle HauntingActor)
{
	m_actorHandle = &HauntingActor;

	GetActor()->m_controller = this;

	// get index from actorHandle and possess the next in the list. activate
	// AI controller restoration
}

Actor* Controller::GetActor()
{
	return m_map->GetActorByHandle(*m_actorHandle);
}


