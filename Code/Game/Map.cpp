#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Image.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Math/MathUtils.hpp"



Map::Map(Game* game, const MapDefinition* definition)
	:m_game(game)
	, m_definition(definition)
{
	m_dimensions = definition->m_image->GetDimensions();
	m_texture = definition->m_spriteSheetTexture;
	m_shader = definition->m_shader;
	m_terrain = new SpriteSheet(*m_texture, definition->m_spriteSheetCellCount);
	CreateTiles();
	CreateGeometry();
	CreateBuffers();

	CreateTestActors();

	g_engine->m_render->SetLightingConstants(Vec3(2,1,-1), .85f, .35f);
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
				m_tiles[tileIndex].m_tileDefinition = currentDef;				// this seems like a way easier method of determining type type

				if (currentDef->m_isSolid)
				{
					int wallSprite = (currentDef->m_wallSpriteCoords.y * m_definition->m_spriteSheetCellCount.x) + currentDef->m_wallSpriteCoords.x;
					AddGeometryForWall(m_tiles[tileIndex].m_bounds, m_terrain->GetUVsForSprite(wallSprite));
					break;
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
	AddVertsForQuad3D(m_vertexes, m_indexes,
		Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),
		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),
		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),
		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), Rgba8::WHITE, UVs);
}

void Map::AddGeometryForCeiling(const AABB3& bounds, const AABB2& UVs)
{
// 	AddVertsForQuad3D(m_vertexes, m_indexes, Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),
// 		Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),
// 		Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),
// 		Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), Rgba8::RED, UVs);

	AddVertsForQuad3D(m_vertexes, m_indexes, 
		 Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),
		 Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),
		 Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),
		 Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), Rgba8::WHITE, UVs);
}

void Map::CreateBuffers()
{
	m_vertexBuffer = g_engine->m_render->CreateVertexBuffer((const unsigned int) m_vertexes.size() * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
	m_indexBuffer = g_engine->m_render->CreateIndexBuffer((const unsigned int) m_indexes.size() * sizeof(unsigned int));
	g_engine->m_render->CopyCPUToGPU(m_vertexes.data(),(const unsigned int) m_vertexes.size() * sizeof(Vertex_PCUTBN), m_vertexBuffer );
	g_engine->m_render->CopyCPUToGPU(m_indexes.data(),(const unsigned int) m_indexes.size() * sizeof(unsigned int), m_indexBuffer);
}

bool Map::IsPositionInBounds(const Vec3& position) const
{
	if((position.x >= 0.f && position.x < (float)m_dimensions.x)&&(position.y >= 0.f && position.y < (float)m_dimensions.y))
	{
		return true;
	}
	else
	return false;
}

bool Map::AreCoordsInBounds(int x, int y) const
{
	if ((x >= 0 && x < m_dimensions.x) && (y >= 0 && y < m_dimensions.y))
	{
		return true;
	}
	else
	return false;
}

const Tile* Map::GetTile(int x, int y) const
{
	if (!AreCoordsInBounds(x, y))
	{
		return nullptr;
	}

	int index = (y * m_dimensions.x) + x;
	
	return &m_tiles[index];
}

void Map::Update()
{
	m_sunIntensity = GetClampedZeroToOne(m_sunIntensity);
	m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);
	g_engine->m_render->SetLightingConstants(m_sunDirection,m_sunIntensity,m_ambientIntensity);

	CollideActors();
	CollideActorsWithMap();
}

void Map::CollideActors()
{
	for (size_t i = 0; i < m_actors.size(); ++i) 
	{
		for (size_t j = i + 1; j < m_actors.size(); ++j) 
		{
			CollideActors(m_actors[i], m_actors[j]);
		}
	}
}

void Map::CollideActors(Actor* actorA, Actor* actorB)
{
	if (DoDiscsOverlap(Vec2(actorA->m_position.x, actorA->m_position.y), actorA->m_physicsRadius,
		Vec2(actorB->m_position.x, actorB->m_position.y), actorB->m_physicsRadius))
	{
		PushDiscOutOfEachOther3D(actorA->m_position, actorA->m_physicsRadius,
			actorB->m_position, actorB->m_physicsRadius);

	}
}

void Map::CollideActorsWithMap()
{

}

void Map::CollideActorWithMap(Actor* actor)
{

}

void Map::CreateTestActors()
{
	Actor* TestEnemy01 = new Actor(m_game, 0.75f, 0.35f, Vec3(7.5f,8.5f,0.25f), Rgba8::RED, true);
	Actor* TestEnemy02 = new Actor(m_game, 0.75f, 0.35f, Vec3(8.5f,8.5f,0.125f), Rgba8::RED, true);
	Actor* TestEnemy03 = new Actor(m_game, 0.75f, 0.35f, Vec3(9.5f,8.5f,0.0f), Rgba8::RED, true);
	Actor* TestProjectile01 = new Actor(m_game, 0.125f, 0.0625f, Vec3(5.5f,8.5f,0.0f), Rgba8::BLUE, false);
	m_actors.push_back(TestEnemy01);
	m_actors.push_back(TestEnemy02);
	m_actors.push_back(TestEnemy03);
	m_actors.push_back(TestProjectile01);

	if (!m_game->m_player)
	{
		m_game->m_player = new Player(m_game);
		m_game->m_player->m_position = Vec3(2.5f, 8.5f, 0.5f);
	}

	m_game->m_player->m_testProjectile = TestProjectile01;
}

void Map::Render()
{
	g_engine->m_render->SetBlendMode(BlendMode::OPAQUE);
	g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_engine->m_render->BindShader(m_shader);
	
	g_engine->m_render->SetModelConstants();
	g_engine->m_render->BindTexture(m_texture);
	g_engine->m_render->DrawIndexedVertexBuffer(m_vertexBuffer, m_indexBuffer, m_indexBuffer->GetCount());

	for (int index = 0; index < m_actors.size(); ++index)
	{
		m_actors[index]->Render();
	}
}

RaycastResult3D Map::RaycastAll(const Vec3& start, const Vec3& direciton, float distance, Actor* owner /*= nullptr*/) const
{
	RaycastResult3D result;

	return result;
}

RaycastResult3D Map::RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance) const
{
	RaycastResult3D result;
	result.m_rayStartPosition = start;
	result.m_rayDirection = direction.GetNormalized();
	result.m_rayLength = distance;

	IntVec2 startTile;
	int tileX = int( floor(start.x));
	int tileY = int( floor(start.y));
	startTile = IntVec2(tileX,tileY);

	if (GetTile(tileX,tileY)->m_tileDefinition->m_isSolid)
	{
		result.m_didImpact = true;
		result.m_impactDistance = 0.f;
		result.m_impactPosition = start;
		result.m_impactNormal = - direction;
		return result;
	}

	float fwdDistPerXCrossing = 1.0f / abs(result.m_rayDirection.x);
	int tileStepDirectionX;
	if (result.m_rayDirection.x < 0)
	{
		tileStepDirectionX = -1;
	}
	else
	{
		tileStepDirectionX = +1;
	}

	float xAtFirstXCrossing = (float)tileX + ((float)tileStepDirectionX + 1.f) / 2.f;
	float xDistToFirstXCrossing = xAtFirstXCrossing - start.x;
	float fwdDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;
	
	int tileStepDirectionY;
	if (result.m_rayDirection.y < 0)
	{
		tileStepDirectionY = -1;
	}
	else
	{
		tileStepDirectionY = +1;
	}

	float yAtFirstYCrossing = (float)tileY + ((float)tileStepDirectionY + 1.f) / 2.f;
	float yDistToFirstYCrossing = yAtFirstYCrossing - start.y;
	float fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerXCrossing;

	while (result.m_didImpact == false)
	{
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing)	// x crosses first
		{
			if (fwdDistAtNextXCrossing < distance)	// missed next crossing is past ray length
			{
				result.m_didImpact = false;
				return result;
			}
			tileX += tileStepDirectionX;
			if (GetTile(tileX, tileY)->m_tileDefinition->m_isSolid) // hit
			{
				result.m_didImpact = true;
 				result.m_impactDistance = fwdDistAtNextXCrossing;
 				result.m_impactPosition = start + (result.m_rayDirection * result.m_impactDistance);
				if (tileStepDirectionX == -1)
				{
					result.m_impactNormal = Vec3(1,0,0);
				}
				else
				{
					result.m_impactNormal = Vec3(-1,0,0);
				}
 				return result;
			}
			fwdDistAtNextXCrossing += fwdDistPerXCrossing;
		}
		else if (fwdDistAtNextXCrossing > fwdDistAtNextYCrossing)	// y crosses first
		{
			if (fwdDistAtNextYCrossing < distance)	//missed next crossing is past ray length
			{
				result.m_didImpact = false;
				return result;
			}
			tileY += tileStepDirectionY;
			if (GetTile(tileX, tileY)->m_tileDefinition->m_isSolid)	// hit
			{
				result.m_didImpact = true;
 				result.m_impactDistance = fwdDistAtNextYCrossing;
 				result.m_impactPosition = start + (result.m_rayDirection * result.m_impactDistance);
				if (tileStepDirectionY == -1)
				{
					result.m_impactNormal = Vec3(0, 1, 0);
				}
				else
				{
					result.m_impactNormal = Vec3(0, -1, 0);
				}
				return result;
			}
		}
		return result;
	}
	return result;
}

RaycastResult3D Map::RaycastWorldZ(const Vec3& start, const Vec3& direction, float distance) const
{
	RaycastResult3D result;
	result.m_rayStartPosition = start;
	result.m_rayDirection = direction.GetNormalized();
	result.m_rayLength = distance;

	Vec3 ray = (start + (result.m_rayDirection * distance));

	if (ray.z > 0)
	{
		auto t = (1 - start.z) / ray.z;
		if (t > 0 && t < 1)
		{
			result.m_didImpact;
			result.m_impactDistance = (distance * t);
			result.m_impactPosition = start + (result.m_rayDirection * result.m_impactDistance);
			result.m_impactNormal = Vec3(0,0,-1);
			return result;
		}
	}
	else if (ray.z < 0)
	{
		auto t = (- start.z) / ray.z;
		if (t > 0 && t < 1)
		{
			result.m_didImpact;
			result.m_impactDistance = (distance * t);
			result.m_impactPosition = start + (result.m_rayDirection * result.m_impactDistance);
			result.m_impactNormal = Vec3(0,0,1);
			return result;
		}
	}
	return result;
}

RaycastResult3D Map::RaycastWorldActors(const Vec3& start, const Vec3& direciton, float distance, Actor* owner /*= nullptr*/) const
{
	RaycastResult3D result;

	return result;
}
