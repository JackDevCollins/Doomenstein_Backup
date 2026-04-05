#pragma once
#include "Engine/Math/AABB3.hpp"

class TileDefinition;

class Tile
{
public:

	Tile(AABB3 bounds = AABB3(Vec3(),Vec3()) );

	AABB3				m_bounds;
	TileDefinition*		m_tileDefinition = nullptr;
};