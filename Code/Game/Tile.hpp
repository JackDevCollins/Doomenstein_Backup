#pragma once
#include "Engine/Math/AABB3.hpp"

class Tile
{
public:

	Tile(AABB3 bounds);

	AABB3	m_bounds;
};