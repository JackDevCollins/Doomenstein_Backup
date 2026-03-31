#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class Player;
class Map;
class Clock;
class Timer;

enum GameState
{
	GAMESTATE_ATTRACT,			// = 0
	GAMESTATE_MENU,
	GAMESTATE_GAME,				// = 1				
	NUM, 
};

//-----------------------------------------------------------------------------------------------
class App;
class Entity;
class Camera;
struct Vec2;

struct Vertex;

class Game
{
public:
	Game( App* owner );
	~Game();
	void Startup();
	void Update( float deltaSeconds );
	void Render() const;
	void Shutdown();
	RandomNumberGenerator const m_rng;
 
	void CheckInputs();
	void RenderEntities() const;
	void RenderText() const;
	void UpdateEntities( float deltaSeconds );
	void UpdateCameras( float deltaSeconds);

	void EnterState(GameState state);
	void ExitState(GameState state);

public:
	GameState			m_currentGameState = GAMESTATE_ATTRACT;
	GameState			m_nextGameState = GAMESTATE_ATTRACT;
	Clock*				m_gameClock = nullptr;
	Timer*				m_gameTimer01 = nullptr;
	float				m_timeStartedCurrentState = 0.f;
	
	App*				m_app;
	
	Camera*				m_worldCamera = nullptr;
	Camera*				m_screenCamera = nullptr;
	Player*				m_player = nullptr;
	std::vector<Vertex> m_grid;
	
	//-----------------------------------------------------------------------------------------------
	Map*				m_currentMap;



private:
	
	void DeleteGarbageEntities();
	void UpdateAttractMode(float deltaSeconds);
	void RenderAttractMode() const;
};