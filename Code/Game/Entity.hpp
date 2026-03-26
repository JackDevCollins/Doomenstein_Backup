#pragma once
#include "Engine/Core/Engine.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"

class Game;

class Entity
{
public:
	Entity(Game* owner);
	virtual ~Entity();

	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	virtual void DebugRender() const;

	virtual Mat44 GetModelToWorldTransform() const;

public:
	Game*			m_game = nullptr;
	Vec3			m_position;
	Vec3			m_velocity;
	Rgba8			m_color	= Rgba8::WHITE;
	EulerAngles		m_orientation;
	EulerAngles		m_angularVelocity;
};