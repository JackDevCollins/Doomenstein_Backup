#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class Player;
class Map;
class Clock;
class Timer;
class PlayerController;

enum GameState
{
	GAMESTATE_ATTRACT,			// = 0
	GAMESTATE_LOBBY,			// = 1
	GAMESTATE_GAME,				// = 2				
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
 
	void CheckInputs();
	void RenderEntities() const;
	void RenderText() const;
	void UpdateEntities( float deltaSeconds );
	void UpdateCameras( float deltaSeconds);

	void RenderHUD() const;
	void RenderWeapon() const;

	void JoinLobby(int controller);			// 1 = keyboard 2 = gamepad
	void LeaveLobby(PlayerController* playerController);

	void EnterState(GameState state);
	void ExitState(GameState state);

public:
	GameState			m_currentGameState = GAMESTATE_ATTRACT;
	GameState			m_nextGameState = GAMESTATE_ATTRACT;
	Clock*				m_gameClock = nullptr;
	Timer*				m_gameTimer01 = nullptr;
	float				m_timeStartedCurrentState = 0.f;
	
	App*				m_app;
	
	Camera*							m_screenCamera = nullptr;
	std::vector<PlayerController*>	m_players;	
	int								m_joinedPlayers = 0;
	//-----------------------------------------------------------------------------------------------
	Map*				m_currentMap = nullptr;

private:
	
	void DeleteGarbageEntities();
	void UpdateAttractMode(float deltaSeconds);
	void RenderAttractMode() const;
	void UpdateLobbyMode(float deltaSeconds);
	void RenderLobbyMode() const;
};