#pragma once
#include "Engine/Core/Engine.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/ActorDefinition.hpp"

class Game;

class Actor
{
public:
	Actor(Game* owner, float physicsHeight, float physicsRadius, Vec3 position, Rgba8 color, bool isStatic);
	Actor(ActorDefinition actorDef);
	virtual ~Actor();

	void  Update(float deltaSeconds);
	void  Render() const;
	void  AddVertsForMe();
	void  TestPojectileInput(Vec3 movement);

	virtual Mat44 GetModelToWorldTransform() const;

public:
	Game*					m_game = nullptr;

	ActorHandle				m_Handle;
	ActorDefinition*		m_Definition
	Map*					m_map = nullptr;
	Vec3					m_position;
	EulerAngles				m_orientation;
	Vec3					m_velocity;
	Vec3					m_acceleration;
	std::vector<Vertex>		m_verts;
	Weapon					m_weapons;
	Actor*					m_owner;
	float					m_corpseLifetime;
	Timer*					m_decomposeTimer = nullptr;
	float					m_health;
	Controller*             m_controller;
	Controller*				m_aiController;
	Rgba8					m_color	= Rgba8::WHITE;
	float					m_physicsHeight;
	float					m_physicsRadius;
	std::vector<Vertex>		m_physicsCylinder;
	bool					m_isStatic;

};