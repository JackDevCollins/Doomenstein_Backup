#pragma once
//#include "Engine/Core/Engine.hpp"
//#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"

//-----------------------------------------------------------------------------------------------
class App;
class Camera;
class Game;
struct EngineConfig;
//-----------------------------------------------------------------------------------------------
extern App* g_app;

//-----------------------------------------------------------------------------------------------

class App
{
public:
	App();
	~App();
	
	void RunFrame();
	void Update( float deltaSeconds );
	void Render() const;
	static bool Event_Quit( EventArgs& args );

	void SetIsQuitting();
	bool IsQuitting() const;
	EngineConfig CreateEngineConfig();
	


public:
	
	Camera*			m_gameCamera = nullptr;		// not here maybe render?
	double			m_lastFrameTime;
	bool			m_isQuitting = false;
	bool			m_isPaused = false;
	bool			m_isSlowMo = false;
	bool			m_pauseAfterNextUpdate = false;
	bool			m_wasWindowinFocusLastFrame = true;
	Game*			m_game = nullptr;
	
};