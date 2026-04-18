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
	void	AddForce(Vec3 force);
	void	AddImpulse(Vec3 impulse);
	void	OnCollide();
	void	OnPossessed();
	void	OnUnPossessed();
	void	MoveInDirection();
	void	TurnInDirection();
	void	Attack();
	void	EquipWeapon();

	virtual Mat44 GetModelToWorldTransform() const;
	Mat44	GetModelToWorldTransformYawOnly() const;

public:
	Game*					m_game				= nullptr;

	ActorHandle				m_handle;
	const ActorDefinition*	m_definition		= nullptr;
	Map*					m_map				= nullptr;
	Vec3					m_position			= Vec3(0,0,0);
	EulerAngles				m_orientation		= EulerAngles(0,0,0);
	Vec3					m_velocity			= Vec3(0,0,0);
	Vec3					m_acceleration		= Vec3(0,0,0);
	bool					m_isDead			= false;
	bool					m_isGarbage			= false;
	bool					m_isFlying			= false;
	std::vector<Vertex>		m_verts;


	// values that will get modified per instance, all other values get directly from def
	std::vector<Weapon*>	m_inventory;
	Weapon*					m_currentWeapon		= nullptr;
	Actor*					m_owner				= nullptr;
	float					m_corpseLifetime	= 1.f;	
	Timer*					m_decomposeTimer	= nullptr;
	float					m_health			= 100.f;
	Controller*				m_controller		= nullptr;
	AIController*			m_aiController		= nullptr;
	Rgba8					m_color				= Rgba8::WHITE;
	float					m_physicsHeight		= 1.f;
	float					m_physicsRadius		= 1.f;
	std::vector<Vertex>		m_physicsCylinder;
	bool					m_isStatic			= false;
	bool					m_isRunning			= false;
};