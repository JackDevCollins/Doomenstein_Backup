#pragma once
#include "Game/Entity.hpp"


class Game;


class Player : public Entity
{
public:
	Player(Game* owner);
	virtual ~Player();

	void Update(float deltaSeconds);
	void Render() const;
	void DebugRender() const;

	bool		 UpdateFromInput(float deltaSeconds);

public:

	float		m_moveSpeed = 1.f;
	float		m_rotationSpeed = 1.f;
	float		m_mouseLookSens = 50.f;
};