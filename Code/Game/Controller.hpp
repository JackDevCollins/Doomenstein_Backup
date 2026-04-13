#pragma once
#pragma once

struct ActorHandle;
class Map;
class Actor;

class Controller
{
public:
	ActorHandle*	m_handle = nullptr;
	Map*			m_map = nullptr;

	void		Possess();
	Actor*		GetActor();
};