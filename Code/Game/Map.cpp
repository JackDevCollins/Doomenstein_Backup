#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Image.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"



Map::Map(Game* game, const MapDefinition* definition)
	:m_game(game)
	, m_definition(definition)
{
	m_dimensions = definition->m_image->GetDimensions();
	m_texture = definition->m_spriteSheetTexture;
	m_shader = definition->m_shader;
	m_terrain = new SpriteSheet(*m_texture, m_dimensions);
	CreateBuffers();
	CreateTiles();
	CreateGeometry();
}

Map::~Map()	// #ToDo need to figure out what to release / delete
{
	m_vertexes.clear();
	
}

void Map::CreateTiles()
{
	int numTiles = m_dimensions.x * m_dimensions.y;
	m_tiles.reserve(numTiles);

	for (int index = 0; index < numTiles; ++index)
	{
		int x = index % m_dimensions.x;
		int y = index / m_dimensions.x;

		m_tiles.emplace_back(Tile(AABB3(
				Vec3((float)x,(float)y,0.0f),
				Vec3((float)x + 1.0f, (float)y + 1.0f, 1.0f)
				)));
	}
	
}

void Map::CreateGeometry()
{
	const std::vector<Rgba8>& texelData = m_definition->m_image->m_texelRgba8Data;

	for (int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex)
	{
		Rgba8 tileColorAtImageIndex = texelData[tileIndex];

		for (int defIndex = 0; defIndex < TileDefinition::s_definitions.size(); ++defIndex)
		{
			TileDefinition* currentDef = TileDefinition::s_definitions[defIndex];

			if (currentDef->m_mapImagePixelColor == tileColorAtImageIndex)
			{
				if (currentDef->m_isSolid)
				{
					int wallSprite = (currentDef->m_wallSpriteCoords.y * m_definition->m_spriteSheetCellCount.x) + currentDef->m_wallSpriteCoords.x;
					AddGeometryForWall(m_tiles[tileIndex].m_bounds, m_terrain->GetUVsForSprite(wallSprite));
				}
				int floorSprite = ( currentDef->m_floorSpriteCoords.y * m_definition->m_spriteSheetCellCount.x) + currentDef->m_floorSpriteCoords.x;
				AddGeometryForFloor(m_tiles[tileIndex].m_bounds, m_terrain->GetUVsForSprite(floorSprite));

				int ceilingSprite = ( currentDef->m_ceilingSpriteCoords.y * m_definition->m_spriteSheetCellCount.x) + currentDef->m_ceilingSpriteCoords.x;
				AddGeometryForCeiling(m_tiles[tileIndex].m_bounds, m_terrain->GetUVsForSprite(ceilingSprite));

				break;
			}
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
	m_vertexBuffer = g_engine->m_render->CreateVertexBuffer( 1, sizeof(Vertex_PCUTBN));
	m_indexBuffer = g_engine->m_render->CreateIndexBuffer(sizeof(unsigned int));
}

bool Map::IsPositionInBounds(const Vec3& position) const
{
	return true;
}

bool Map::AreCoordsInBounds(int x, int y) const
{
	return true;
}

const Tile* Map::GetTile(int x, int y) const
{
	int index = (y * m_dimensions.x) + x;
	
	return &m_tiles[index];
}

void Map::Update()
{
	
}

void Map::Render()
{
	
	
	g_engine->m_render->SetModelConstants();
	g_engine->m_render->BindTexture(m_texture);
	g_engine->m_render->DrawIndexedVertexBuffer(m_vertexBuffer, m_indexBuffer, m_indexBuffer->GetCount());

}

//g_engine->m_render->SetBlendMode(BlendMode::OPAQUE);
	//g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	//g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	//g_engine->m_render->BindShader(m_shader);