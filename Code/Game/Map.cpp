#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Image.hpp"



Map::Map(Game* game, const MapDefinition* definition)
	:m_game(game)
	, m_definition(definition)
{
	m_dimensions = definition->m_spriteSheetCellCount;
	m_texture = definition->m_spriteSheetTexture;
	m_shader = definition->m_shader;
}

Map::~Map()	// #ToDo need to figure out what to release / delete
{
	m_vertexes.clear();
}

void Map::CreateTiles()
{
	int numTiles = m_dimensions.x * m_dimensions.y;
	m_tiles.resize(numTiles);

	for (int index = 0; index < numTiles; ++index)
	{
		m_tiles.emplace_back( Tile(AABB3(Vec3((index % m_dimensions.x),(index / m_dimensions.x), 0), Vec3((index % m_dimensions.x) + 1, (index % m_dimensions.x) + 1, 1) ) ) );	// fill vector with index
	}
	
}

void Map::CreateGeometry()
{
	for (int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex)
	{
		const void* texeldata = m_definition->m_image->GetRawData();

		for (int index = 0; index < TileDefinition::s_definitions.size(); ++index)
		{
			if (TileDefinition::s_definitions[index]->m_mapImagePixelColor == )
		}
		
		
	}
}

void Map::AddGeometryForWall(const AABB3& bounds, const AABB2& UVs)
{
	AddVertsForAABB3D(m_vertexes, m_indexes, bounds, Rgba8::WHITE, UVs);
}

void Map::AddGeometryForFloor(const AABB3& bounds, const AABB2& UVs)
{
	AddVertsForQuad3D(m_vertexes, m_indexes, Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z),
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Rgba8::RED, UVs);
}

void Map::AddGeometryForCeiling(const AABB3& bounds, const AABB2& UVs)
{
	AddVertsForQuad3D(m_vertexes, m_indexes, Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), Rgba8::RED, UVs);
}

void Map::CreateBuffers()
{
	//m_vertexBuffer = g_engine->m_render->CreateVertexBuffer( 1, sizeof(Vertex_PCUTBN));
}

const Tile* Map::GetTile(int x, int y) const
{
	
	
}

