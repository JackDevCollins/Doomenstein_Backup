#pragma once
#include "Engine/Core/Engine.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/ActorHandle.hpp"


class Game;
class Map;
class Controller;
class Weapon;
class AIController;
class PlayerController;

class Actor
{
public:
	Actor(Game* owner, float physicsHeight, float physicsRadius, Vec3 position, Rgba8 color, bool isStatic);
	Actor(Map* map, Game* owner, const ActorDefinition* actorDef);
	virtual ~Actor();

	void  Update(float deltaSeconds);
	void  Render() const;
	void  AddVertsForMe();
	void  TestPojectileInput(Vec3 movement);

	void	UpdatePhysics(float deltaSeconds);
	void	Damage(Actor* damager);
	void	AddForce();
	void	AddImpulse();
	void	OnCollide();
	void	OnPossessed();
	void	OnUnPossessed();
	void	MoveInDirection();
	void	TurnInDirection();
	void	Attack();
	void	EquipWeapon();

	virtual Mat44 GetModelToWorldTransform() const;

public:
	Game*					m_game = nullptr;

	ActorHandle				m_handle;
	const ActorDefinition*	m_definition	= nullptr;
	Map*					m_map			= nullptr;
	Vec3					m_position;
	EulerAngles				m_orientation;
	Vec3					m_velocity;
	Vec3					m_acceleration;
	std::vector<Vertex>		m_verts;

	std::vector<Weapon*>	m_inventory;
	Weapon*					m_currentWeapon = nullptr;
	Actor*					m_owner			 = nullptr;
	float					m_corpseLifetime;
	Timer*					m_decomposeTimer = nullptr;
	float					m_health;
	PlayerController*       m_controller	 = nullptr;
	AIController*			m_aiController	 = nullptr;
	Rgba8					m_color	= Rgba8::WHITE;
	float					m_physicsHeight;
	float					m_physicsRadius;
	std::vector<Vertex>		m_physicsCylinder;
	bool					m_isStatic;

};