#pragma once
#include "Game/ActorHandle.hpp"

struct ActorHandle;
class Map;
class Actor;

class Controller
{
public:
	Controller() = default;

	ActorHandle		m_actorHandle = ActorHandle::INVALID;
	Map*			m_map		  = nullptr;

	virtual void		DamagedBy(Actor* damageSource);
	virtual void		Possess(ActorHandle HauntingActor);
	Actor*			    GetActor();

};