#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Rgba8.hpp"

class Game;
class Actor;

class Player 
{
public:
	Player(Game* owner);
	virtual ~Player();

	void Update(float deltaSeconds);
	void Render() const;
	void DebugRender() const;

	bool		UpdateFromInput(float deltaSeconds);
	Mat44		GetModelToWorldTransform() const;

public:
	Game*		m_game = nullptr;
	Vec3		m_position;
	Vec3		m_velocity;
	Rgba8		m_color = Rgba8::WHITE;
	EulerAngles m_orientation;
	EulerAngles m_angularVelocity;
	float		m_moveSpeed = 1.f;
	float		m_rotationSpeed = 1.f;
	float		m_mouseLookSens = 50.f;

	Actor*		m_testProjectile = nullptr;
	bool		m_controlProjectile = false;
};