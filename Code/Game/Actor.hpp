#pragma once
#include "Engine/Core/Engine.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"

class Game;

class Actor
{
public:
	Actor(Game* owner, float physicsHeight, float physicsRadius, Vec3 position, Rgba8 color, bool isStatic);
	virtual ~Actor();

	void  Update(float deltaSeconds);
	void  Render() const;
	void  AddVertsForMe();
	void TestPojectileInput(Vec3 movement);

	virtual Mat44 GetModelToWorldTransform() const;

public:
	Game*					m_game = nullptr;
	Vec3					m_position;
	EulerAngles				m_orientation;
	Rgba8					m_color	= Rgba8::WHITE;
	float					m_physicsHeight;
	float					m_physicsRadius;
	std::vector<Vertex>		m_physicsCylinder;
	bool					m_isStatic;

};