#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex.hpp"
#include <vector>

class Texture;

class Prop : public Entity
{
public:
	Prop(Game* owner);
	virtual ~Prop();

	void Update(float deltaSeconds);
	void Render() const;
	void DebugRender() const;

public:
	std::vector<Vertex>		m_vertexes;
	Texture*				m_texture = nullptr;
};