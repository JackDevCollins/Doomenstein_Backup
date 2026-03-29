#pragma once


class Game;


class Map 
{
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

	void Update();
	void CollideActors();
	void CollideActors(Actor* actorA, Actor* actorB);
	void CollideActorsWithMap();
	void CollideActorWithMAp(Actor* actor);

	void Render();

	RaycastResult RaycastAll(const Vec3& start, const Vec3& direciton, float distance, Actor* owner = nullptr) const;
	RaycastResult RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance) const;
	RaycastResult RaycastWorldZ(const Vec3& start, const Vec3& direction, float distance) const;
	RaycastResult RaycastWorldActors(const vec3& start, const Vec3& direciton, float distance, Actor* owner = null) const;
	
	Game* m_game = nullptr;

protected:
	
	const MapDefinition* m_definition = nullptr;
	std::vector<Tile> m_tiles;
	IntVec2 m_dimensions;

	std::vector<Vertex_PCUTBN> m_vertexes;
	std::vector<unsigned int> m_indexes;
	const Texture* m_texture = nullptr;
	Shader* m_shader = nullptr;
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
};