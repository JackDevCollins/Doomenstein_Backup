#include "Game/App.hpp"
#include "Engine/Core/Engine.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Image.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"
#include "ThirdParty/stb/stb_image.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

App* g_app = nullptr;

App::App()
{
	g_app = this;

	//XmlDocument gameConfigXML;
	//gameConfigXML.LoadFile("Data/GameConfig.xml");

	//g_globalConfigBlackboard.PopulateFromXmlElementAttributes(*gameConfigXML.FirstChildElement());

 	XmlDocument engineConfigXML;
 	engineConfigXML.LoadFile("Data/EngineConfig.xml");
	
	g_globalConfigBlackboard.PopulateFromXmlElementAttributes(*engineConfigXML.FirstChildElement("AudioConfig"));
	g_globalConfigBlackboard.PopulateFromXmlElementAttributes(*engineConfigXML.FirstChildElement("RenderConfig"));
	g_globalConfigBlackboard.PopulateFromXmlElementAttributes(*engineConfigXML.FirstChildElement("WindowConfig"));
	g_globalConfigBlackboard.PopulateFromXmlElementAttributes(*engineConfigXML.FirstChildElement("InputConfig"));
	g_globalConfigBlackboard.PopulateFromXmlElementAttributes(*engineConfigXML.FirstChildElement("EventConfig"));
 	g_globalConfigBlackboard.PopulateFromXmlElementAttributes(*engineConfigXML.FirstChildElement("DevConsoleConfig"));

	EngineConfig EngineConfigure;
	EngineConfigure = CreateEngineConfig();

	g_engine = new Engine(EngineConfigure); //engineConfigXML
	g_engine->Startup();

	DebugRenderConfig DebugConfigure;
	DebugRenderSystemStartup(DebugConfigure);
	g_defaultFont = g_engine->m_render->CreateOrGetBitmapFont("Data/Image/SquirrelFixedFont");

	m_game = new Game(g_app);
	g_engine->m_eventSystem->SubscribeEventCallbackFunction("Quit", App::Event_Quit);
	
	m_lastFrameTime = GetCurrentTimeSeconds();
}

App::~App()
{
	DebugRenderSystemShutdown();
	delete m_game;
	m_game = nullptr;
	delete g_engine;
	g_engine = nullptr;
}

void App::RunFrame()
{
	float deltaSeconds = static_cast<float>(g_systemClock->GetDeltaSeconds());
// 	float timeNow = (float) GetCurrentTimeSeconds();
// 	float deltaSeconds = static_cast <float> (timeNow - m_lastFrameTime);
// 	m_lastFrameTime = timeNow;

	AABB2 testSquare = AABB2(Vec2(5,5),Vec2(10,10));

	
	if (m_isPaused)
	{
		deltaSeconds = 0.f;
	}
	if (m_pauseAfterNextUpdate)
	{
		m_isPaused = true;
		m_pauseAfterNextUpdate = false;
	}
	if (m_isSlowMo)
	{
		deltaSeconds /= 10.f ;
	}
	Rgba8 clearColor{ 50, 50,50, 255 };
	g_engine->m_render->ClearScreen(clearColor);

	g_engine->BeginFrame();
	DebugRenderBeginFrame();

	Update(deltaSeconds);
	Render();
	g_engine->EndFrame();
	DebugRenderEndFrame();

}

void App::Update(float deltaSeconds)
{
	// mouse cursor handling
	if (g_engine->m_window->GetHwnd() != GetActiveWindow() || g_engine->m_devConsole->IsOpen() == true || m_game->m_currentGameState == GAMESTATE_ATTRACT)
	{
		g_engine->m_input->m_cursorMode = CursorMode::POINTER;

		if (g_engine->m_window->GetHwnd() != GetActiveWindow())
		{
			m_wasWindowinFocusLastFrame = false;
		}
	}
	else
	{
		g_engine->m_input->m_cursorMode = CursorMode::FPS;
	}
	
	if (g_engine->m_window->GetHwnd() == GetActiveWindow() && !m_wasWindowinFocusLastFrame)
	{
		g_engine->m_input->m_cursorState.m_cursorClientDelta = Vec2(0.f,0.f);
		m_wasWindowinFocusLastFrame = true;
	}

	m_game->Update(deltaSeconds);
}

void App::Render() const
{	
	m_game->Render();
}

void App::SetIsQuitting()
{
	m_isQuitting = true;
}

bool App::IsQuitting() const
{
	return m_isQuitting;
}

bool App::Event_Quit([[maybe_unused]] EventArgs& args)
{
	g_app->SetIsQuitting();
	return false;
}

EngineConfig App::CreateEngineConfig()
{
	EngineConfig		returnEngineConfig;
	AudioConfig			audioConfig;
	RenderConfig		renderConfig;
	WindowConfig		windowConfig;
	InputConfig			inputConfig;
	EventConfig			eventSystemConfig;
	DevConsoleConfig	devConsoleConfig;

	audioConfig.m_isEnabled = g_globalConfigBlackboard.GetValue("AudioConfig_m_isEnabled", false);
	renderConfig.m_isEnabled = g_globalConfigBlackboard.GetValue("RenderConfig_m_isEnabled", false);
	windowConfig.m_isEnabled = g_globalConfigBlackboard.GetValue("WindowConfig_m_isEnabled", false);
	windowConfig.m_clientAspect = g_globalConfigBlackboard.GetValue("m_clientAspect", 2.f);
	windowConfig.m_windowTitle = g_globalConfigBlackboard.GetValue("m_windowTitle", "Unnamed SD Application");
	inputConfig.m_isEnabled = g_globalConfigBlackboard.GetValue("InputConfig_m_isEnabled", false);
	eventSystemConfig.m_isEnabled = g_globalConfigBlackboard.GetValue("EventConfig_m_isEnabled", false);
	devConsoleConfig.m_isEnabled = g_globalConfigBlackboard.GetValue("DevConsoleConfig_m_isEnabled", false);
	devConsoleConfig.m_isEnabled = g_globalConfigBlackboard.GetValue("m_linesShown", 10.f);

	returnEngineConfig.m_audioConfig = audioConfig;
	returnEngineConfig.m_renderConfig = renderConfig;
	returnEngineConfig.m_windowConfig = windowConfig;
	returnEngineConfig.m_inputConfig = inputConfig;
	returnEngineConfig.m_eventSystemConfig = eventSystemConfig;
	returnEngineConfig.m_devConsoleConfig = devConsoleConfig;

	return returnEngineConfig;
}