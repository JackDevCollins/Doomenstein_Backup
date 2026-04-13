#pragma once
#include "Game/MapDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Tile.hpp"
#include "Game/ActorHandle.hpp"

class Game;
class Actor;
struct AABB3;
struct AABB2;
struct Vec3;
class VertexBuffer;
class IndexBuffer;
struct Vertex_PCUTBN;
struct RaycastResult3D;

class Map 
{
public:

	Map(Game* game, const MapDefinition* definition);
	~Map();

	void CreateTiles();
	void CreateGeometry();
	void AddGeometryForWall(const AABB3& bounds, const AABB2& UVs);
	void AddGeometryForFloor(const AABB3& bounds, const AABB2& UVs);
	void AddGeometryForCeiling(const AABB3& bounds, const AABB2& UVs);
	void CreateBuffers();

	bool IsPositionInBounds(const Vec3& position) const;
	bool AreCoordsInBounds(int x, int y) const;
	const Tile* GetTile(int x, int y) const;

	void Update(float deltaSeconds);
	void CollideActors();
	void CollideActors(Actor* actorA, Actor* actorB);
	void CollideActorsWithMap();
	void CollideActorWithMap(Actor* actor);

	void CreateTestActors();

	Actor* SpawnActor(const SpawnInfo& spawnInfo);
	Actor* GetActorByHandle(const ActorHandle handle) const;

	void Render();

	RaycastResult3D RaycastAll(const Vec3& start, const Vec3& direciton, float distance, Actor* owner = nullptr) const;
	RaycastResult3D RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance) const;
	RaycastResult3D RaycastWorldZ(const Vec3& start, const Vec3& direction, float distance) const;
	RaycastResult3D RaycastWorldActors(const Vec3& start, const Vec3& direciton, float distance, Actor* owner = nullptr) const;
	
	Game* m_game = nullptr;

	
	const MapDefinition*		m_definition = nullptr;
	std::vector<Tile>			m_tiles;
	IntVec2						m_dimensions;
	std::vector<Vertex_PCUTBN>	m_vertexes;
	std::vector<unsigned int>	m_indexes;
	Texture*					m_texture = nullptr;
	SpriteSheet*				m_terrain = nullptr;
	Shader*						m_shader = nullptr;
	VertexBuffer*				m_vertexBuffer = nullptr;
	IndexBuffer*				m_indexBuffer = nullptr;

	std::vector<Actor*>			m_actors;
	unsigned int				m_nextActorUID = 0;

	Vec3						m_sunDirection = Vec3(2,1,-1);
	float						m_sunIntensity = 0.85f;
	float						m_ambientIntensity = 0.35f;
};