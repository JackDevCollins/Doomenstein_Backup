#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerController.hpp"
#include "Game/AIController.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/Vertex.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Image.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"



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

	g_engine->m_render->SetLightingConstants(Vec3(2,1,-1), .85f, .35f);



	//Startup();	// startup called by game so it creates a map reference
}

Map::~Map()	// #ToDo need to figure out what to release / delete
{
	for (PlayerController* player : m_game->m_players)
	{
		if (player == nullptr) continue;
		player->m_actorHandle = ActorHandle::INVALID;
	}
	m_vertexes.clear();
	m_vertexBuffer->~VertexBuffer();
	m_vertexBuffer = nullptr;
	m_indexBuffer->~IndexBuffer();
	m_indexBuffer = nullptr;
	for (Actor* actor : m_actors)
	{
		delete actor;
		actor = nullptr;
	}
	m_spawnpoints.clear();
	m_actors.clear();
	m_tiles.clear();
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

			if (currentDef->m_mapImagePixelColor.r == tileColorAtImageIndex.r &&
				currentDef->m_mapImagePixelColor.g == tileColorAtImageIndex.g &&
				currentDef->m_mapImagePixelColor.b == tileColorAtImageIndex.b)
			{
				m_tiles[tileIndex].m_tileDefinition = currentDef;				// this seems like a way easier method of determining type type
				if (tileColorAtImageIndex.a != 255)
				{
					if (tileColorAtImageIndex.a == 100) // 1 tall
					{
						m_tiles[tileIndex].m_height = 1.00f;
					}
					if (tileColorAtImageIndex.a == 200) // 2 tall
					{
						m_tiles[tileIndex].m_height = 2.00f;
					}
					
					for (int heightIndex = 0; heightIndex < m_tiles[tileIndex].m_height; ++heightIndex)
					{
						m_tiles[tileIndex].m_bounds.m_mins.z += (1.f * heightIndex);
						m_tiles[tileIndex].m_bounds.m_maxs.z += (1.f * heightIndex);

						if (currentDef->m_isSolid)
						{
							int wallSprite = (currentDef->m_wallSpriteCoords.y * m_definition->m_spriteSheetCellCount.x) + currentDef->m_wallSpriteCoords.x;
							AddGeometryForWall(m_tiles[tileIndex].m_bounds, m_terrain->GetUVsForSprite(wallSprite));
						}

						if (currentDef->m_hasTop)
						{
							int ceilingSprite = (currentDef->m_ceilingSpriteCoords.y * m_definition->m_spriteSheetCellCount.x) + currentDef->m_ceilingSpriteCoords.x;
							AddGeometryForCeiling(m_tiles[tileIndex].m_bounds, m_terrain->GetUVsForSprite(ceilingSprite));
						}

						if (currentDef->m_hasBottom)
						{
							int floorSprite = (currentDef->m_floorSpriteCoords.y * m_definition->m_spriteSheetCellCount.x) + currentDef->m_floorSpriteCoords.x;
							AddGeometryForFloor(m_tiles[tileIndex].m_bounds, m_terrain->GetUVsForSprite(floorSprite));
						}
					}
				}
				else
				{
					if (currentDef->m_isSolid)
					{
						int wallSprite = (currentDef->m_wallSpriteCoords.y * m_definition->m_spriteSheetCellCount.x) + currentDef->m_wallSpriteCoords.x;
						AddGeometryForWall(m_tiles[tileIndex].m_bounds, m_terrain->GetUVsForSprite(wallSprite));
					}

					if (currentDef->m_hasTop)
					{
						int ceilingSprite = (currentDef->m_ceilingSpriteCoords.y * m_definition->m_spriteSheetCellCount.x) + currentDef->m_ceilingSpriteCoords.x;
						AddGeometryForCeiling(m_tiles[tileIndex].m_bounds, m_terrain->GetUVsForSprite(ceilingSprite));
					}

					if (currentDef->m_hasBottom)
					{
						int floorSprite = (currentDef->m_floorSpriteCoords.y * m_definition->m_spriteSheetCellCount.x) + currentDef->m_floorSpriteCoords.x;
						AddGeometryForFloor(m_tiles[tileIndex].m_bounds, m_terrain->GetUVsForSprite(floorSprite));
					}
				}
			}
		}	
	}
}

void Map::AddGeometryForWall(const AABB3& bounds, const AABB2& UVs)
{
	AddVertsForIndexedAABB3D(m_vertexes, m_indexes, bounds, Rgba8::WHITE, UVs);
}

void Map::AddGeometryForFloor(const AABB3& bounds, const AABB2& UVs)
{
	AddVertsForQuad3DTaBiNo(m_vertexes, m_indexes,
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

	AddVertsForQuad3DTaBiNo(m_vertexes, m_indexes, 
		 Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),
		 Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),
		 Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z),
		 Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), Rgba8::WHITE, UVs);
}

void Map::CreateBuffers()
{
	m_vertexBuffer = g_engine->m_render->CreateVertexBuffer((const unsigned int) m_vertexes.size() * sizeof(Vertex), sizeof(Vertex));
	m_indexBuffer = g_engine->m_render->CreateIndexBuffer((const unsigned int) m_indexes.size() * sizeof(unsigned int));
	g_engine->m_render->CopyCPUToGPU(m_vertexes.data(),(const unsigned int) m_vertexes.size() * sizeof(Vertex), m_vertexBuffer );
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

void Map::Startup()
{
	CreateStartupActors();

	for (PlayerController* player : m_game->m_players)
	{
		if (player == nullptr) return;
		CreatePlayerActor(player);
	}
}

void Map::Update(float deltaSeconds)
{
	m_sunIntensity = GetClampedZeroToOne(m_sunIntensity);
	m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);
	
	g_engine->m_render->SetLightingConstants(m_sunDirection.GetNormalized(),m_sunIntensity,m_ambientIntensity);

	for (int index = 0; index < m_actors.size(); ++index)
	{
		if (m_actors[index] != nullptr)
		{
			m_actors[index]->Update(deltaSeconds);
		}
	}

	CollideActors();
	CollideActorsWithMap();

	DeleteDestroyedActors();

	for (PlayerController* player : m_game->m_players)
	{
		if (player != nullptr && player->GetActor() == nullptr)
		{
			CreatePlayerActor(player);
		}
	}

	AddMoreBoys();
}

void Map::CollideActors()
{
	for (size_t i = 0; i < m_actors.size(); ++i) 
	{
		for (size_t j = i + 1; j < m_actors.size(); ++j) 
		{
			if (m_actors[i] == nullptr || m_actors[j] == nullptr)	return;

			CollideActors(m_actors[i], m_actors[j]);
		}
	}
}

void Map::CollideActors(Actor* actorA, Actor* actorB)
{
	if (!actorA->m_definition->m_collidesWithActors || !actorB->m_definition->m_collidesWithActors) 
	{
		return;
	}
	if (actorA->m_owner == actorB || actorB->m_owner == actorA) 
	{
		return;
	}
	if (actorA->m_isDead || actorB->m_isDead)
	{
		return;
	}

	Vec2 actorAXYPosition = Vec2(actorA->m_position.x, actorA->m_position.y);
	Vec2 actorBXYPosition =  Vec2(actorB->m_position.x, actorB->m_position.y);
	FloatRange actorAzRange = FloatRange(actorA->m_position.z, actorA->m_position.z + actorA->m_physicsHeight);
	FloatRange actorBzRange = FloatRange(actorB->m_position.z, actorB->m_position.z + actorB->m_physicsHeight);

	if(DoZCylindersOverlap3D(actorAXYPosition, actorA->m_definition->m_collision_radius, actorAzRange, actorBXYPosition, actorB->m_definition->m_collision_radius, actorBzRange) )
	{
		DebuggerPrintf("COLLIDE");
		actorA->OnCollide(actorB->m_handle);
		actorB->OnCollide(actorA->m_handle);

		if ((((((static_cast<PlayerController*>(actorA->m_controller) == m_game->m_players[0]) && actorB->m_definition->m_name == "Nazi")))) || ((((static_cast<PlayerController*>(actorB->m_controller) == m_game->m_players[0]) && actorA->m_definition->m_name == "Nazi"))))
		{
			
			if (actorB->m_definition->m_name == "Nazi" && static_cast<PlayerController*>(actorA->m_controller)->m_cameraMode == SKATER)
			{
				actorB->m_health = 0;
			}
			if (actorA->m_definition->m_name == "Nazi" && static_cast<PlayerController*>(actorB->m_controller)->m_cameraMode == SKATER)
			{
				actorA->m_health = 0;
			}
		}
	}

// 	if (DoDiscsOverlap(actorAXYPosition, actorA->m_physicsRadius,
// 		actorBXYPosition, actorB->m_physicsRadius))
// 	{
// 		float aTop = actorA->m_position.z + actorA->m_physicsHeight;
// 		float aBottom = actorA->m_position.z;
// 		float bTop = actorB->m_position.z + actorB->m_physicsHeight;
// 		float bBottom = actorB->m_position.z;
// 
// 		if (aBottom < bTop && aTop > bBottom) // vertical overlap
// 		{
// 			float horizontalOverlap = (actorA->m_physicsRadius + actorB->m_physicsRadius) - GetDistance2D(actorAXYPosition,actorBXYPosition);
// 			float overlapTop = aTop - bBottom;
// 			float overlapBottom = bTop - aBottom;
// 			float zOverlapAmount = (overlapTop < overlapBottom) ? overlapTop : overlapBottom;
// 
// 			if (horizontalOverlap <= zOverlapAmount)
// 			{
// 				if (!actorA->m_definition->m_dieOnCollide && !actorB->m_definition->m_dieOnCollide)
// 				{
// 					PushDiscOutOfEachOther2D(actorAXYPosition, actorA->m_physicsRadius, actorBXYPosition, actorB->m_physicsRadius);
// 					actorA->m_position = Vec3(actorAXYPosition.x,actorAXYPosition.y, actorA->m_position.z);
// 					actorB->m_position = Vec3(actorBXYPosition.x,actorBXYPosition.y, actorB->m_position.z);
// 				}
// 			}
// 			else
// 			{
// 				float pushAmount = zOverlapAmount * 0.5f;
// 				if (aBottom < bBottom) // a below b
// 				{
// 					actorA->m_position.z -= pushAmount;
// 					actorB->m_position.z += pushAmount;
// 				}
// 				else  // b below a
// 				{
// 					actorA->m_position.z += pushAmount;
// 					actorB->m_position.z -= pushAmount;
// 				}
// 			}
// 		}
// 	}	
}

void Map::CollideActorsWithMap()
{
	for (size_t i = 0; i < m_actors.size(); ++i)
	{
		if (m_actors[i] != nullptr)
		{
			CollideActorWithMap(m_actors[i]);	
		}
	}
}

void Map::CollideActorWithMap(Actor* actor)
{
	if( !actor->m_definition->m_collidesWithWorld ) return;

	bool	collided = false;
	IntVec2 tileCoordsActorOccupies			= IntVec2(RoundDownToInt(actor->m_position.x),RoundDownToInt(actor->m_position.y));
	const Tile*	tileObjectActorOccupies		= GetTile(tileCoordsActorOccupies.x,tileCoordsActorOccupies.y);
	actor->m_tileObjActorOccupies = tileObjectActorOccupies;
	IntVec2 positionNorth	  = tileCoordsActorOccupies + IntVec2(0, 1);
	IntVec2 positionEast	  = tileCoordsActorOccupies + IntVec2(1, 0);
	IntVec2 positionSouth	  = tileCoordsActorOccupies + IntVec2(0, -1);
	IntVec2 positionWest	  = tileCoordsActorOccupies + IntVec2(-1, 0);
	IntVec2 positionNorthEast = tileCoordsActorOccupies + IntVec2(1, 1);
	IntVec2 positionSouthEast = tileCoordsActorOccupies + IntVec2(1, -1);
	IntVec2 positionSouthWest = tileCoordsActorOccupies + IntVec2(-1, 1);
	IntVec2 positionNorthWest = tileCoordsActorOccupies + IntVec2(-1, -1);
	std::vector<const Tile*> testTiles;
	testTiles.push_back(GetTile(positionNorth.x,positionNorth.y));			// North = index 0
	testTiles.push_back(GetTile(positionEast.x,positionEast.y));			// East = index 1
	testTiles.push_back(GetTile(positionSouth.x,positionSouth.y));			// South = index 2
	testTiles.push_back(GetTile(positionWest.x,positionWest.y));			// West = index 3
	testTiles.push_back(GetTile(positionNorthEast.x, positionNorthEast.y));	// NorthEast = index 4
	testTiles.push_back(GetTile(positionSouthEast.x, positionSouthEast.y));	// SouthEast = index 5
	testTiles.push_back(GetTile(positionSouthWest.x, positionSouthWest.y));	// SouthWest = index 6
	testTiles.push_back(GetTile(positionNorthWest.x, positionNorthWest.y));	// NorthWest = index 7

	for (int testNum = 0; testNum < testTiles.size(); ++testNum)
	{
		if (testTiles[testNum] == nullptr)
		{
			actor->m_isDead;
			return;
		}
		AABB2 XYTileBounds = AABB2(Vec2(testTiles[testNum]->m_bounds.m_mins.x, testTiles[testNum]->m_bounds.m_mins.y),Vec2(testTiles[testNum]->m_bounds.m_maxs.x, testTiles[testNum]->m_bounds.m_maxs.y));
		Vec2 XYPosition = Vec2(actor->m_position.x,actor->m_position.y);

		if (!AreCoordsInBounds(RoundDownToInt( testTiles[testNum]->m_bounds.m_mins.x) ,RoundDownToInt(testTiles[testNum]->m_bounds.m_mins.y) ))
		{
			if (IsPointInsideDisc2D(XYTileBounds.GetNearestPoint(XYPosition), XYPosition, actor->m_physicsRadius))
			{
				PushDiscOutOfFixedAABB2D(XYPosition, actor->m_physicsRadius, XYTileBounds);
				actor->m_position = Vec3(XYPosition.x, XYPosition.y, actor->m_position.z);
				collided = true;
			}
		}
		if (testTiles[testNum]->m_tileDefinition->m_isSolid)
		{
			if (IsPointInsideDisc2D(XYTileBounds.GetNearestPoint(XYPosition),XYPosition, actor->m_physicsRadius))
			{
				PushDiscOutOfFixedAABB2D(XYPosition, actor->m_physicsRadius, XYTileBounds);
				actor->m_position = Vec3(XYPosition.x,XYPosition.y,actor->m_position.z);
				collided = true;
			}
// 			if (IsPointInsideZCylinder3D(testTiles[testNum]->m_bounds.GetNearestPoint(actor->m_position), actor->m_position, actor->m_physicsHeight, actor->m_physicsRadius))
// 			{
// 				PushCylinderZOutOfFixedAABB3D(actor->m_position, actor->m_physicsRadius, testTiles[testNum]->m_bounds);
// 			}
			
		}
	}
	testTiles.clear();


	// z check 
	if (actor->m_position.z < actor->m_tileObjActorOccupies->m_height)
	{
		actor->m_position.z = actor->m_tileObjActorOccupies->m_height;
		collided = true;
	}
// 	if (actor->m_position.z + actor->m_physicsHeight > 1.f)
// 	{
// 		actor->m_position.z = 1.f - actor->m_physicsHeight;
// 		collided = true;
// 	}

	if (collided)
	{
		actor->m_velocity = Vec3(0.f,0.f,0.f);
	}

	if (collided && actor->m_definition->m_dieOnCollide)
	{
		actor->m_health = 0;
	}
}

void Map::CreateStartupActors()
{
	for (int actorIndex = 0; actorIndex < m_definition->m_spawnInfos.size(); ++actorIndex)
	{
 		Actor* newActor = SpawnActor(*m_definition->m_spawnInfos[actorIndex]);
		if (AddActorToMap(newActor))
		{
			ERROR_RECOVERABLE("ACTOR FAILED TO BE ADDED");
		}
	}
}

void Map::CreatePlayerActor( PlayerController* playerController)
{
	int randomSpawnPositionIndex = RollRandomIntInRange(0, (int)m_spawnpoints.size() - 1);
	ActorHandle* spawnPointActorHandle = &m_spawnpoints[randomSpawnPositionIndex];

	SpawnInfo playerSpawnInfo;
	playerSpawnInfo.m_actorType = "Marine";
	playerSpawnInfo.m_position = GetActorByHandle(*spawnPointActorHandle)->m_position;
	playerSpawnInfo.m_orientation = GetActorByHandle(*spawnPointActorHandle)->m_orientation;
	playerSpawnInfo.m_velocity	= GetActorByHandle(*spawnPointActorHandle)->m_velocity;

	Actor* newPlayer = SpawnActor(playerSpawnInfo);
	if (AddActorToMap(newPlayer))
	{
		ERROR_RECOVERABLE("PLAYER ACTOR FAILED TO ADD");
	}
	playerController->Possess(newPlayer->m_handle);
}

void Map::DeleteDestroyedActors()
{
	for (int garbageIndex = 0; garbageIndex < m_actors.size(); ++garbageIndex)
	{
		if (m_actors[garbageIndex] != nullptr && m_actors[garbageIndex]->m_isGarbage)
		{
			delete m_actors[garbageIndex];
			m_actors[garbageIndex] = nullptr;
		}
	}
}

Actor* Map::SpawnActor(const SpawnInfo& spawnInfo)
{
	Actor* newActor = new Actor(this, m_game, ActorDefinition::GetByName(spawnInfo.m_actorType));
	newActor->m_position = spawnInfo.m_position;
	newActor->m_orientation = spawnInfo.m_orientation;
	newActor->m_velocity = spawnInfo.m_velocity;

	int indexForNewActor = (int) m_actors.size();

	if (indexForNewActor > ActorHandle::MAX_ACTOR_INDEX) ERROR_AND_DIE("TOO MANY ACTORS");

	for (int emptyindex = 0; emptyindex < m_actors.size(); ++emptyindex)
	{
		if (m_actors[emptyindex] == nullptr)
		{
			indexForNewActor = emptyindex;
			newActor->m_handle = ActorHandle(m_nextActorUID, (unsigned int) indexForNewActor);

			if (newActor->m_aiController != nullptr)
			{
				newActor->m_aiController->m_actorHandle = newActor->m_handle;
				newActor->m_aiController->m_map = this;
			}

			return newActor;
		}
	}
	
	newActor->m_handle = ActorHandle(m_nextActorUID, (unsigned int) indexForNewActor);
	m_nextActorUID += 1;
	
	if (newActor->m_aiController != nullptr)
	{
		newActor->m_aiController->m_actorHandle = newActor->m_handle;
		newActor->m_aiController->m_map = this;
	}

	return newActor;
}

bool Map::AddActorToMap(Actor* actor)
{
	int index = actor->m_handle.GetIndex();
	if (index != m_actors.size())
	{
		m_actors[index] = actor;
	}
	else
	{
		m_actors.push_back(actor);
	}

	if (m_actors[index]->m_handle == actor->m_handle)
	{
		if (m_actors[index]->m_definition->m_name == "SpawnPoint")
		{
			m_spawnpoints.push_back(m_actors[index]->m_handle);
		}
		return false;
	}
	else
		return true;
}

Actor* Map::GetActorByHandle(const ActorHandle handle) const
{
	if (handle == ActorHandle::INVALID) return nullptr;

	int index = handle.GetIndex();

	if (m_actors[index] == nullptr) return nullptr;

	if (m_actors[index]->m_handle.m_data == handle.m_data)
	{
		return m_actors[index];
	}
	return nullptr;
}

Actor* Map::GetClosestVisibleEnemy(ActorHandle actorHandle)
{
	Actor* seekingActor = GetActorByHandle(actorHandle);

	Actor* closestEnemy = nullptr;
	float distanceFromActorSQ = 1000000.f;

	for (int actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex)
	{
		Actor* testActor = m_actors[actorIndex];

		if (testActor == nullptr) continue;

		if (testActor->m_definition->m_faction != "Marine") continue;

		Vec2 actor2DPos = Vec2(testActor->m_position.x, testActor->m_position.y);

		if (IsPointInsideOrientedSector2D(actor2DPos, Vec2(seekingActor->m_position.x, seekingActor->m_position.y), seekingActor->m_orientation.m_yawDegrees,
			seekingActor->m_definition->m_sightAngle,seekingActor->m_definition->m_sightRadius))
		{
			float distanceSQ = GetDistanceSquared3D(seekingActor->m_position, testActor->m_position);
			if (distanceSQ < distanceFromActorSQ)
			{
				closestEnemy = testActor;
				distanceFromActorSQ = distanceSQ;
			}
		}
	}
	if(closestEnemy == nullptr) return nullptr;
	RaycastResult3D sightlineRaycast = RaycastAll(seekingActor->m_position, closestEnemy->m_position - ( seekingActor->m_position) , seekingActor->m_definition->m_sightRadius) ;

	//DebugAddWorldCylinder(sightlineRaycast.m_rayStartPosition, (sightlineRaycast.m_rayStartPosition + (sightlineRaycast.m_rayDirection * sightlineRaycast.m_rayLength)), 0.01f, .3f, Rgba8::WHITE, Rgba8::WHITE);

	if (sightlineRaycast.m_didImpact && sightlineRaycast.m_impactedObjectID == "Marine")
	{
		return closestEnemy;
	}
	else
	return nullptr;
}

void Map::AddMoreBoys()
{
	int numberOfBoys = 0;

	for (int index = 0; index < m_actors.size(); ++index)
	{
		if (m_actors[index] != nullptr)
		{
			if (m_actors[index]->m_definition->m_name == "Nazi" || m_actors[index]->m_definition->m_name == "Cacodemon")
			{
				++numberOfBoys;
			}
		}
		if (numberOfBoys < 4)
		{
			SpawnInfo newSpawn;
			newSpawn.m_actorType = "Nazi";
			int randomx = RollRandomIntInRange(0 + 1, m_dimensions.x -1);
			int randomy = RollRandomIntInRange(0 + 1, m_dimensions.y -1);
			newSpawn.m_position = Vec3(randomx, randomy, .1f);
			newSpawn.m_orientation = EulerAngles((float)RollRandomIntInRange(0, 360), 0.f, 0.f);

			SpawnInfo newSpawn2;
			newSpawn2.m_actorType = "Nazi";
			newSpawn2.m_position = Vec3(25.f, 25.f, .1f);
			newSpawn2.m_orientation = EulerAngles((float)RollRandomIntInRange(0, 360), 0.f, 0.f);

			AddActorToMap(SpawnActor(newSpawn));
			AddActorToMap(SpawnActor(newSpawn2));
		}
	}
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
		if (m_actors[index] != nullptr)
		{
			m_actors[index]->Render();
		}
	}
}

RaycastResult3D Map::RaycastAll(const Vec3& start, const Vec3& direction, float distance, Actor* owner /*= nullptr*/) const
{
	float lowestDistance = 100000000.f;
	int lowestIndex = -1;

	if (start.x > m_dimensions.x || start.y > m_dimensions.y)
	{
		RaycastResult3D miss;
		miss.m_didImpact = false;
		return miss;
	}

	RaycastResult3D XYResult	= RaycastWorldXY(start, direction, distance);
	RaycastResult3D ZResult		= RaycastWorldZ(start, direction, distance);
	RaycastResult3D ActorResult = RaycastWorldActors(start, direction, distance, owner);
	std::vector<RaycastResult3D> results;
	results.push_back(XYResult);
	results.push_back(ZResult);
	results.push_back(ActorResult);
	
	for (int index = 0; index < results.size(); ++index)
	{
		if (results[index].m_didImpact)
		{
				if (results[index].m_impactDistance < lowestDistance)
				{
					lowestIndex = index;
					lowestDistance = results[index].m_impactDistance;
				}
		}
		else
		{
			continue;
		}
	}
	if (lowestIndex != -1)
	{
		return results[lowestIndex];
	}
	else
	{
		RaycastResult3D miss;
		miss.m_didImpact = false;
		return miss;
	}
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

	if (GetTile(tileX,tileY) != nullptr && GetTile(tileX,tileY)->m_tileDefinition->m_isSolid)
	{
		result.m_didImpact = true;
		result.m_impactDistance = 0.f;
		result.m_impactPosition = start;
		result.m_impactNormal = - result.m_rayDirection;
		result.m_impactedObjectID = "Wall";
		result.m_pointerToImpactedObject = nullptr;
		return result;
	}
	float fwdDistPerXCrossing;
	if (result.m_rayDirection.x == 0.f)
	{
		fwdDistPerXCrossing = 1.f;
	}
	else
	fwdDistPerXCrossing = 1.0f / abs(result.m_rayDirection.x);
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
	float fwdDistPerYCrossing;
	if (result.m_rayDirection.y == 0.f)
	{
		fwdDistPerYCrossing = 1.f;
	}
	else
	fwdDistPerYCrossing = 1.0f / abs(result.m_rayDirection.y);
	float yAtFirstYCrossing = (float)tileY + ((float)tileStepDirectionY + 1.f) / 2.f;
	float yDistToFirstYCrossing = yAtFirstYCrossing - start.y;
	float fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;

	while (result.m_didImpact == false)
	{
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing)	// x crosses first
		{
			if (fwdDistAtNextXCrossing > distance)	// missed next crossing is past ray length
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
				result.m_impactedObjectID = "Wall";
				result.m_pointerToImpactedObject = nullptr;
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
		else //(fwdDistAtNextXCrossing > fwdDistAtNextYCrossing)	// y crosses first
		{
			if (fwdDistAtNextYCrossing > distance)	//missed next crossing is past ray length
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
				result.m_impactedObjectID = "Wall";
				result.m_pointerToImpactedObject = nullptr;
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
			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
		}
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

	if (result.m_rayDirection.z > 0)
	{
		auto t = (10 - start.z) / (result.m_rayDirection.z * distance);
		if (t > 0 && t < 1)
		{
			result.m_didImpact = true;
			result.m_impactDistance = (distance * t);
			result.m_impactPosition = start + (result.m_rayDirection * result.m_impactDistance);
			result.m_impactNormal = Vec3(0,0,-1);
			result.m_impactedObjectID = "Ceiling";
			result.m_pointerToImpactedObject = nullptr;
			return result;
		}
	}
	else if (result.m_rayDirection.z < 0)
	{
		auto t = (- start.z) / (result.m_rayDirection.z * distance);
		if (t > 0 && t < 1)
		{
			result.m_didImpact = true;
			result.m_impactDistance = (distance * t);
			result.m_impactPosition = start + (result.m_rayDirection * result.m_impactDistance);
			result.m_impactNormal = Vec3(0,0,1);
			result.m_impactedObjectID = "Floor";
			result.m_pointerToImpactedObject = nullptr;
			return result;
		}
	}
	return result;
}

RaycastResult3D Map::RaycastWorldActors(const Vec3& start, const Vec3& direction, float distance, Actor* owner /*= nullptr*/) const
{
	RaycastResult3D closestHit;
	closestHit.m_didImpact = false;
	closestHit.m_impactDistance = distance + 1.0f;
	
	Vec3 normDirection = direction.GetNormalized();

	Vec3 ray = (start + (normDirection * distance));


	for (int index = 0; index < m_actors.size(); ++index)
	{	
		Actor* actor = m_actors[index];

		if(actor == owner) continue;
		
		if(actor == nullptr) continue;

		if (IsPointInsideDisc2D(Vec2(start.x, start.y), Vec2(actor->m_position.x, actor->m_position.y), actor->m_physicsRadius))
		{
			float aTop = actor->m_position.z + actor->m_physicsHeight;
			float aBottom = actor->m_position.z;
			
			if (start.z > aBottom && start.z < aTop)
			{
				RaycastResult3D result;
				result.m_rayStartPosition = start;
				result.m_rayDirection = direction.GetNormalized();
				result.m_rayLength = distance;
				result.m_didImpact = true;
				result.m_impactDistance = 0.0f;
				result.m_impactNormal = - result.m_rayDirection;
				result.m_impactPosition = start;
				result.m_impactedObjectID = actor->m_definition->m_name;
				result.m_pointerToImpactedObject = actor;
				return result;
			}
		}
		Vec2 cylindercenterXY = Vec2(actor->m_position.x, actor->m_position.y);
		float cylinderMinZ = m_actors[index]->m_position.z;
		float cylinderMaxZ = cylinderMinZ + actor->m_physicsHeight;
		RaycastResult3D cylinderRaycast = RaycastVsCylinderZ3D(start, direction, distance, cylindercenterXY, cylinderMinZ, cylinderMaxZ, m_actors[index]->m_physicsRadius);
		if (cylinderRaycast.m_didImpact && cylinderRaycast.m_impactDistance < closestHit.m_impactDistance && !actor->m_isDead)
		{
			closestHit = cylinderRaycast;
			closestHit.m_pointerToImpactedObject = actor;
			if (actor->m_definition)
			{
				closestHit.m_impactedObjectID = actor->m_definition->m_name;
			}
		}
	}

	if (closestHit.m_didImpact)
	{
		return closestHit;
	}

	RaycastResult3D miss;
	miss.m_didImpact = false;
	miss.m_rayStartPosition = start;
	miss.m_rayDirection = normDirection;
	miss.m_rayLength = distance;
	return miss;

}

// 		std::vector<RaycastResult3D> intersectionChecks;
// 
// 		float aTop = m_actors[index]->m_position.z + m_actors[index]->m_physicsHeight;
// 		float aBottom = m_actors[index]->m_position.z;
// 		float rayDeltaZ = normDirection.z * distance;
// 		// intersection with top of cylinder
// 
// 		
// 		float topT = ( aTop - start.z ) / rayDeltaZ;
// 		RaycastResult3D* topResult = nullptr;
// 		if (topT > 0 && topT < 1)
// 		{
// 			RaycastResult3D topOfCylinder;
// 			topOfCylinder.m_rayStartPosition = start;
// 			topOfCylinder.m_rayDirection = normDirection;
// 			topOfCylinder.m_rayLength = distance;
// 			topOfCylinder.m_didImpact = true;
// 			topOfCylinder.m_impactDistance = (distance * topT);
// 			topOfCylinder.m_impactPosition = (start + (normDirection * topOfCylinder.m_impactDistance));
// 			topOfCylinder.m_impactNormal = topOfCylinder.m_impactPosition - Vec3(m_actors[index]->m_position.x,m_actors[index]->m_position.y, topOfCylinder.m_impactPosition.z);		// might need to be reversed. impact point to center of actor for normal
// 			intersectionChecks.push_back(topOfCylinder);
// 			topResult = &topOfCylinder;
// 		}
// 		float bottomT = (aBottom - start.z) / rayDeltaZ;
// 		RaycastResult3D* bottomResult = nullptr;
// 		if (bottomT > 0 && bottomT < 1)
// 		{
// 			RaycastResult3D bottomOfCylinder;
// 			bottomOfCylinder.m_rayStartPosition = start;
// 			bottomOfCylinder.m_rayDirection = normDirection;
// 			bottomOfCylinder.m_rayLength = distance;
// 			bottomOfCylinder.m_didImpact = true;
// 			bottomOfCylinder.m_impactDistance = (distance * bottomT);
// 			bottomOfCylinder.m_impactPosition = (start + (normDirection * bottomOfCylinder.m_impactDistance));
// 			bottomOfCylinder.m_impactNormal = bottomOfCylinder.m_impactPosition - Vec3(m_actors[index]->m_position.x, m_actors[index]->m_position.y, bottomOfCylinder.m_impactPosition.z);		// might need to be reversed. impact point to center of actor for normal
// 			intersectionChecks.push_back(bottomOfCylinder);
// 			bottomResult = &bottomOfCylinder;
// 		}
// 
// 		//Vec3 rayStartToActorCenter = Vec3(m_actors[index]->m_position - start);
// 
// 		Vec2 start2D = Vec2(start.x, start.y);
// 		Vec2 dir2D = Vec2(direction.x,direction.y);
// 		
// 		Vec3 rayXYonly = Vec3(ray.x, ray.y, 0.f);
// 		float XYmagSquared = DotProduct3D(ray, rayXYonly);
// 		float lengthOfRayinXYPlane = GetDistance2D(start2D, Vec2(ray.x,ray.y));
// 		//sqrtf(XYmagSquared);
// 		Vec2 discIn2d(m_actors[index]->m_position.x,m_actors[index]->m_position.y);
// 		Vec3 fwdXYDirection = (rayXYonly / lengthOfRayinXYPlane);
// 		Vec2 fwd2d = Vec2(fwdXYDirection.x,fwdXYDirection.y);
// 
// 		RaycastResult2D discRaycast = RaycastVsDisc2D(start2D, fwd2d, lengthOfRayinXYPlane, discIn2d, m_actors[index]->m_physicsRadius);
// 		float widthT = 10000000;
// 		RaycastResult3D* widthResult = nullptr;
// 		if (discRaycast.m_didImpact)
// 		{
// 			float xydiscT = (discRaycast.m_impactDist / lengthOfRayinXYPlane);
// 			widthT = discRaycast.m_impactDist / lengthOfRayinXYPlane;
// 			//widthT = (xydiscT * distance) / lengthOfRayinXYPlane;
// 			//widthT = distance * xydiscT;
// 			float impactDistance = (distance * widthT);
// 			Vec3 impactPosition = start + (normDirection * impactDistance);
// 
// 
// 			RaycastResult3D widthOfCylinder;
// 			widthOfCylinder.m_rayStartPosition = start;
// 			widthOfCylinder.m_rayDirection = normDirection;
// 			widthOfCylinder.m_rayLength = distance;
// 			widthOfCylinder.m_didImpact = true;
// 			widthOfCylinder.m_impactDistance = impactDistance;
// 			widthOfCylinder.m_impactPosition = impactPosition;
// 			widthOfCylinder.m_impactNormal = widthOfCylinder.m_impactPosition - Vec3(m_actors[index]->m_position.x, m_actors[index]->m_position.y, widthOfCylinder.m_impactPosition.z);		// might need to be reversed. impact point to center of actor for normal
// 			intersectionChecks.push_back(widthOfCylinder);
// 			widthResult = &widthOfCylinder;
// 		}
// 
// 		if (topT < bottomT && topT < widthT)
// 		{
// 			if (topT > 0 && topT < 1)
// 			{
// 				hits.push_back(topResult);
// 			}
// 		}
// 		if (bottomT < topT && bottomT < widthT)
// 		{
// 			if (bottomT > 0 && bottomT < 1)
// 			{
// 				hits.push_back(bottomResult);
// 			}
// 		}
// 		if (widthT < topT && widthT < topT)
// 		{
// 			if (widthT > 0 && widthT < 1)
// 			{
// 				hits.push_back(widthResult);
// 			}
// 		}
// 	}
// 	float lowestImpactD = 10000000.f;
// 	int lowestIndex = -1;
// 	if (hits.size() > 0)
// 	{
// 		for (int index = 0; index < hits.size(); ++index)
// 		{
// 			if (hits[index]->m_impactDistance < lowestImpactD)
// 			{
// 				lowestImpactD = hits[index]->m_impactDistance;
// 				lowestIndex = index;
// 			}
// 		}
// 		if (lowestIndex != -1)
// 		{
// 			return *hits[lowestIndex];
// 		}
// 
// 	}
// 
// 	return miss;
// }
