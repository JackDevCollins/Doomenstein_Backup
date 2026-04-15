#pragma once

struct ActorHandle;
class Map;
class Actor;

class Controller
{
public:
	ActorHandle*	m_actorHandle = nullptr;
	Map*			m_map		  = nullptr;

	void		Possess(ActorHandle HauntingActor);
	Actor*		GetActor();

};