#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Map.hpp"
#include "Game/Tile.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/ProjectileActorDefinition.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include <string>
#include <math.h>
#include <cmath>
#include <iomanip>
#include <sstream>

// include order is high-level to low level			app-game-playership-engine-input-renderer-rgba  maybe include your own at the top 
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
Game::Game(App* owner)
	: m_app(owner)
{
	
	m_screenCamera = new Camera();

	m_gameClock = new Clock(*g_systemClock);
	g_systemClock->AddChild(m_gameClock);
	m_gameTimer01 = new Timer(.01, m_gameClock);
	m_gameTimer01->Start();

	
	//m_screenCamera->SetCameraToRenderTransform(Mat44::CAMERA_TO_RENDER);
	
	
	// i could createorgetshader here or just whenever i need it for the first time (render something lit)

	Startup();
	owner->m_game = this;
}

Game::~Game()
{
}

void Game::Startup()
{
	g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);				// #ToDo what other modes do i need to set?

	TileDefinition::InitializeDefinitions("Data/Definitions/TileDefinitions.xml");
	MapDefinition::InitializeDefinitions("Data/Definitions/MapDefinitions.xml");
	ActorDefinition::InitializeDefinitions("Data/Definitions/ActorDefinitions.xml");
	//ProjectileActorDefinition::InitializeDefinitions("Data/Definitions/ProjectileActorDefinitions.xml");
	WeaponDefinition::InitializeDefinitions("Data/Definitions/WeaponDefinitions.xml");
}


void Game::Update(float deltaSeconds)			
{
	// check if a state change was requested at any time during the previous frame
	if (m_currentGameState != m_nextGameState)
	{
		// change the game state variable
		ExitState(m_currentGameState);
		m_currentGameState = m_nextGameState;
		m_timeStartedCurrentState = static_cast<float>(GetCurrentTimeSeconds());
		EnterState(m_currentGameState);
	}

	if (m_currentGameState == GAMESTATE_ATTRACT || m_currentGameState == GAMESTATE_MENU)
	{
		UpdateAttractMode( deltaSeconds );
	}
	if (m_currentMap)
	{
		m_currentMap->Update(static_cast<float>(m_gameClock->GetDeltaSeconds()));
	}
	
	
	CheckInputs();

	UpdateEntities(deltaSeconds);

	UpdateCameras(deltaSeconds);

	DeleteGarbageEntities();
}

//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	// set in every render function
// 	g_engine->m_render->SetModelConstants(GetModelToWorldTransform(), m_color);
// 	g_engine->m_render->SetBlendMode(BlendMode::OPAQUE);
// 	g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
// 	g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
// 	g_engine->m_render->BindTexture(m_texture);


	if (m_currentGameState == GameState::GAMESTATE_ATTRACT || m_currentGameState == GameState::GAMESTATE_MENU)
	{
 		RenderAttractMode();
	}

	if (m_currentGameState == GameState::GAMESTATE_GAME)
	{
		g_engine->m_render->BeginCamera(*m_currentMap->m_playerCamera);

		m_currentMap->Render();

		g_engine->m_render->BindShader(nullptr);

		RenderEntities();

		RenderText();

		DebugRenderWorld(*m_currentMap->m_playerCamera);
	
		g_engine->m_render->EndCamera(*m_currentMap->m_playerCamera);
	}

	//	For UI elements
	g_engine->m_render->BeginCamera(*m_screenCamera);

		g_engine->m_devConsole->Render(AABB2(Vec2(m_screenCamera->GetOrthoBottomLeft()), Vec2(m_screenCamera->GetOrthoTopRight())));
		Vec3 position;
		EulerAngles orientation;
		bool controlMode = false;
		std::string controlModeString = "empty";
		if (m_player)
		{
// 			position = m_player->m_position;
// 			orientation = m_player->m_orientation;
//			controlMode = m_player->m_controlProjectile;
		}
		
		AABB2 screenBounds = AABB2(m_screenCamera->GetOrthoBottomLeft(), m_screenCamera->GetOrthoTopRight());

		char buffer[32];

		sprintf_s(buffer, "%.2f", g_systemClock->GetTotalSeconds());
		std::string displayTime = buffer;

		sprintf_s(buffer, "%.2f", g_systemClock->GetFrameRate());
		std::string displayFPS = buffer;

		sprintf_s(buffer, "%.2f", g_systemClock->GetTimeScale());
		std::string displayTimeScale = buffer;

		if (controlMode)
		{
			controlModeString = "[F1] Control Mode: Projectile";
		}
		else
		{
			controlModeString = "[F1] Control Mode: Camera";
		}

// 		sprintf_s(buffer, "%.2f", position.x);
// 		std::string displayPosx = buffer;
// 		sprintf_s(buffer, "%.2f", position.y);
// 		std::string displayPosy = buffer;
// 		sprintf_s(buffer, "%.2f", position.z);
// 		std::string displayPosz = buffer;
// 
// 		sprintf_s(buffer, "%.2f", orientation.m_yawDegrees);
// 		std::string displayYaw = buffer;
// 		sprintf_s(buffer, "%.2f", orientation.m_pitchDegrees);
// 		std::string displayPitch = buffer;
// 		sprintf_s(buffer, "%.2f", orientation.m_rollDegrees);
// 		std::string displayRoll = buffer;

// 		DebugAddScreenText("Player position: " + displayPosx + " , " + displayPosy + " , " + 
// 				displayPosz, AABB2(Vec2(screenBounds.m_mins.x,screenBounds.m_maxs.y - 20.f),screenBounds.m_maxs), 20.f, Vec2(0, 1), 0);
// 		DebugAddScreenText("Camera orientation " + displayYaw + " , " + displayPitch + 
// 				" , " + displayRoll, AABB2(Vec2(screenBounds.m_mins.x,screenBounds.m_maxs.y - 40.f),Vec2(screenBounds.m_maxs.x,screenBounds.m_maxs.y - 20.f)),20.f, Vec2(0,1),0);
		DebugAddScreenText(controlModeString, AABB2(Vec2(screenBounds.m_mins.x + 300.f ,screenBounds.m_maxs.y - 20.f),screenBounds.m_maxs), 20.f, Vec2(0.f,1.f), 0.f);
		DebugAddScreenText("Time: " + displayTime + " FPS: " + displayFPS + " Scale: " + displayTimeScale, 
				AABB2(Vec2(screenBounds.m_mins.x,screenBounds.m_maxs.y - 20.f),screenBounds.m_maxs), 20.f, Vec2(1.f,0), 0.f);
		DebugRenderScreen(*m_screenCamera);

	g_engine->m_render->EndCamera(*m_screenCamera);
}

void Game::Shutdown()
{

}

void Game::CheckInputs()
{

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_ESC) || (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::Select)))			// exit to attract screen or close app
	{
		m_nextGameState = GameState::GAMESTATE_ATTRACT;
	}	
	if (g_engine->m_input->WasKeyJustPressed('P') || (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::Start)))					// p to pause
	{
		g_app->m_isPaused = !g_app->m_isPaused;
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_TILDE))		// ~ toggle dev console
	{
		g_engine->m_devConsole->ToggleOpen();
	}
	if (m_player != nullptr)
	{
		if (g_engine->m_input->WasKeyJustPressed('F'))		// toggle possession camera
		{
			m_player->m_cameraMode = !m_player->m_cameraMode;
		}
	}
	


	///////// Lighting Controls /////////

	char buffer[32];

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F2))
	{
		m_currentMap->m_sunDirection.x -= 1;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunDirection.x);
		std::string sunDirectionX = buffer;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunDirection.y);
		std::string sunDirectionY = buffer;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunDirection.z);
		std::string sunDirectionZ = buffer;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunIntensity);
		
		DebugAddMessage("Sun Direction Decreased to: " + sunDirectionX + ", " + sunDirectionY + ", " + sunDirectionZ, 2.f);
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F3))
	{
		m_currentMap->m_sunDirection.x += 1;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunDirection.x);
		std::string sunDirectionX = buffer;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunDirection.y);
		std::string sunDirectionY = buffer;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunDirection.z);
		std::string sunDirectionZ = buffer;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunIntensity);

		DebugAddMessage("Sun Direction Increased to: " + sunDirectionX + ", " + sunDirectionY + ", " + sunDirectionZ, 2.f);
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F4))
	{
		m_currentMap->m_sunDirection.y -= 1;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunDirection.x);
		std::string sunDirectionX = buffer;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunDirection.y);
		std::string sunDirectionY = buffer;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunDirection.z);
		std::string sunDirectionZ = buffer;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunIntensity);

		DebugAddMessage("Sun Direction Decreased to: " + sunDirectionX + ", " + sunDirectionY + ", " + sunDirectionZ, 2.f);
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F5))
	{
		m_currentMap->m_sunDirection.y += 1;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunDirection.x);
		std::string sunDirectionX = buffer;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunDirection.y);
		std::string sunDirectionY = buffer;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunDirection.z);
		std::string sunDirectionZ = buffer;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunIntensity);

		DebugAddMessage("Sun Direction Increased to: " + sunDirectionX + ", " + sunDirectionY + ", " + sunDirectionZ, 2.f);
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F6))
	{
		m_currentMap->m_sunIntensity -= 0.05f;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunIntensity);
		std::string sunIntensity = buffer;

		DebugAddMessage("Sun Intensity Decreased to: " + sunIntensity, 2.f);
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F7))
	{
		m_currentMap->m_sunIntensity += 0.05f;
		sprintf_s(buffer, "%.2f", m_currentMap->m_sunIntensity);
		std::string sunIntensity = buffer;

		DebugAddMessage("Sun Intensity Increased to: " + sunIntensity, 2.f);
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F8))
	{
		m_currentMap->m_sunIntensity -= 0.05f;
		sprintf_s(buffer, "%.2f", m_currentMap->m_ambientIntensity);
		std::string ambientIntensity = buffer;

		DebugAddMessage("Ambient Intensity Decreased to: " + ambientIntensity, 2.f);
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F9))
	{
		m_currentMap->m_ambientIntensity += 0.05f;
		sprintf_s(buffer, "%.2f", m_currentMap->m_ambientIntensity);
		std::string ambientIntensity = buffer;

		DebugAddMessage("Ambient Intensity Increased to: " + ambientIntensity, 2.f);
	}

	///////// debug render controls ///////
	if (g_engine->m_input->WasKeyJustPressed('1'))				// Spawn a line from player along their fwd 20units long dur 10, radius .0625, xray yellow
	{
		DebugAddWorldCylinder(m_player->m_position, m_player->m_position + (m_player->m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetNormalized() * 20.f),
				0.0625, -1.f, Rgba8::YELLOW,Rgba8::YELLOW,DebugRenderMode::X_RAY);
	}
	if (g_engine->m_input->IsKeyDown('2'))
	{
		DebugAddWorldSphere(Vec3(m_player->m_position.x, m_player->m_position.y, 0.f), .5f, 60, Rgba8(150,75,0), Rgba8(150,75,0));
	}
	if (g_engine->m_input->WasKeyJustPressed('3'))				// Spawn a wire sphere 2u player fwd, duration 5, radius 1, w/ depth, green -> red
	{
		DebugAddWorldWireSphere(m_player->m_position + (m_player->m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetNormalized() * 2.f), 1.f, 5.f, Rgba8::GREEN, Rgba8::RED);
	}
	if (g_engine->m_input->WasKeyJustPressed('4'))				// spawn a basis using the current player model matrix, duration 20, w/ depth, white
	{
		DebugAddBasis(m_player->GetModelToWorldTransform().MakeTranslation3D(m_player->m_position), 20.f, 1.f, 0.25f, -5.f);
	}
	if (g_engine->m_input->WasKeyJustPressed('5'))				// spawn a full opposing billboarded 3d text showing player pos and orien. duration 10, height 0.125, w/ depth color white -> red
	{

	}
	if (g_engine->m_input->WasKeyJustPressed('6'))		// spawn wireframe cylinder @ player, duration 10, radius 0.5, height 1, w/ depth, color white -> red
	{
		DebugAddWorldWireCylinder(m_player->m_position, m_player->m_position + (m_player->m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetKBasis3D().GetNormalized() * 1.f), 0.5f, 10.f, Rgba8::WHITE, Rgba8::RED);
	}
	if (g_engine->m_input->WasKeyJustPressed('7'))
	{
		DebugAddMessage("Camera orientation " + std::to_string(m_player->m_orientation.m_yawDegrees) + " , " + std::to_string(m_player->m_orientation.m_pitchDegrees) +
			" , " + std::to_string(m_player->m_orientation.m_rollDegrees), 5.f, Rgba8::MAGENTA);
	}

}

void Game::RenderEntities() const
{
}

void Game::RenderText() const 
{
	
	g_engine->m_render->SetModelConstants(GetBillboardTransform(BillboardType::FULL_FACING,m_player->GetModelToWorldTransform(),Vec3(0,0,5)));
	g_engine->m_render->BindTexture(nullptr);
	g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_engine->m_render->SetDepthMode(DepthMode::READ_ONLY_LESS_EQUAL);
	g_engine->m_render->SetBlendMode(BlendMode::ALPHA);

	std::vector<Vertex> textVerts;

	BitmapFont& font = *g_defaultFont;

	font.AddVertsForText3DAtOriginXForward(textVerts, 1.f, "Dickbutt");


	g_engine->m_render->BindTexture(&font.GetTexture());
	g_engine->m_render->DrawVertexArray(textVerts);
}


void Game::UpdateEntities([[maybe_unused]]float deltaSeconds)
{
// 	if (m_player)
// 	{
// 		m_player->Update(deltaSeconds);
// 	}
}

void Game::UpdateCameras([[maybe_unused]]float deltaSeconds)
{
	Vec2 clientDimentions = Vec2(static_cast<float>(g_engine->m_window->GetClientDimensions().x), static_cast<float>(g_engine->m_window->GetClientDimensions().y));
	m_screenCamera->SetOrthoView(Vec2(0.f, 0.f), clientDimentions);

	float tempAspect = g_engine->m_window->m_config.m_clientAspect;										// #ToDo my client aspect from config (16/10) does not give the desired results. 2 does
// 	m_playerCamera->SetPerspectiveView(tempAspect, 60.f, 0.1f, 100.0f);
// 	
// 	if (m_player)
// 	{
// 		m_playerCamera->SetPositionAndOrientation(m_player->m_position, m_player->m_orientation);
// 		m_playerCamera->SetCameraToRenderTransform(m_playerCamera->GetCameraToRenderTransform());
// 	}
	
}


void Game::EnterState(GameState state)	// state is what you are entering
{
	if (state == GAMESTATE_GAME)
	{
		if (m_currentMap == nullptr)
		{
			std::string mapToLoad = g_globalConfigBlackboard.GetValue("defaultMap", "defaultMap");
			m_currentMap = new Map(this, MapDefinition::GetByName(mapToLoad));
			m_currentMap->Startup();
		}

		if (m_player == nullptr)
		{
			//m_player = new Player(this);
			//m_player->m_position = Vec3(2.5f,8.5f,0.5f);
			//m_playerCamera->SetPositionAndOrientation(m_player->m_position, m_player->m_orientation);
		}

	}
}

void Game::ExitState(GameState state)	// state is what you are exiting
{
	if (state == GAMESTATE_GAME)
	{
		m_currentMap->~Map();
		m_currentMap = nullptr;
// 		m_player->~Player();
// 		m_player = nullptr;
	}
}

void Game::DeleteGarbageEntities()
{
// 	for (int Index = 0; Index < MAX_ENTITY; ++Index)
// 	{
// 		Entity* testEntity = m_entityArray[Index];
// 		if (m_entityArray[Index] != nullptr)
// 		{
// 			if (testEntity->m_isGarbage)
// 			{
// 				delete m_entityArray[Index];
// 				m_entityArray[Index] = nullptr;
// 			}
// 		}
// 	}
}

void Game::UpdateAttractMode( [[maybe_unused]] float deltaSeconds)
{
	if (g_engine->m_input->WasKeyJustPressed(' ')|| g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::A))
	{
		m_nextGameState = GAMESTATE_GAME;
	}
	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_ESC)|| (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::Select)))
	{
		g_app->SetIsQuitting();
	}
}

void Game::RenderAttractMode() const			// #todo add pulsing to start button? ship or circle? done with cosine somehow
{
	Camera attractCamera;
	g_engine->m_render->SetModelConstants();
	g_engine->m_render->SetBlendMode(BlendMode::ALPHA);
	g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_engine->m_render->BindTexture(nullptr);

	attractCamera.SetOrthoView(Vec2(0.f,0.f), Vec2(1600.f,800.f));
	g_engine->m_render->BeginCamera(attractCamera);
	
	if (m_currentGameState == GAMESTATE_ATTRACT)
	{
		float pulseAmount = RangeMap(static_cast<float>(cos(7 * GetCurrentTimeSeconds())), -1.f, 1.f, 100.f, 255.f);
		unsigned char pulse = static_cast<unsigned char>(pulseAmount);

		DebugDrawRing(Vec2(800.f, 400.f), 125.f, 25.f, Rgba8(255, 0, 0, pulse));
		unsigned char fakeImageData[64] =
		{
			// Row 1 (top)
			0,255,255,255,    // Cyan
			255,0,255,255,    // Magenta
			255,255,0,255,    // Yellow
			0,0,0,255,        // Black

			// Row 2 (checkerboard black / brown)
			0,0,0,255,        // Black
			200,100,50,255,   // Brown
			0,0,0,255,        // Black
			200,100,50,255,   // Brown

			// Row 3 (checkerboard brown / black)
			200,100,50,255,   // Brown
			0,0,0,255,        // Black
			200,100,50,255,   // Brown
			0,0,0,255,        // Black

			// Row 4 (bottom)
			255,0,0,255,      // Red
			0,255,0,255,      // Green
			0,0,255,255,      // Blue
			255,255,255,255   // White
		};
		 Vertex tempAABB2verts[6];

		tempAABB2verts[0].m_position = Vec3(0.f, 0.f, 0.f);
		tempAABB2verts[1].m_position = Vec3(600.f, 0.f, 0.f);
		tempAABB2verts[2].m_position = Vec3(600.f, 600.f, 0.f);
		tempAABB2verts[3].m_position = Vec3(0.f, 0.f, 0.f);
		tempAABB2verts[4].m_position = Vec3(600.f,600.f, 0.f);
		tempAABB2verts[5].m_position = Vec3(0.f, 600.f, 0.f);
		tempAABB2verts[0].m_color = Rgba8(255,255,255,255);
		tempAABB2verts[1].m_color = Rgba8(255,255,255,255);
		tempAABB2verts[2].m_color = Rgba8(255,255,255,255);
		tempAABB2verts[3].m_color = Rgba8(255,255,255,255);
		tempAABB2verts[4].m_color = Rgba8(255,255,255,255);
		tempAABB2verts[5].m_color = Rgba8(255,255,255,255);
		tempAABB2verts[0].m_uvTexCoords= Vec2(0.f,0.f);
		tempAABB2verts[1].m_uvTexCoords= Vec2(1.f,0.f);
		tempAABB2verts[2].m_uvTexCoords= Vec2(1.f,1.f);
		tempAABB2verts[3].m_uvTexCoords= Vec2(0.f,0.f);
		tempAABB2verts[4].m_uvTexCoords= Vec2(1.f,1.f);
		tempAABB2verts[5].m_uvTexCoords= Vec2(0.f,1.f);

	
		Texture* testTexture2 = g_engine->m_render->CreateOrGetTextureFromFile("Data/Images/Test_StbiFlippedAndOpenGL.png");

		//Texture* testTexture = g_engine->m_render->CreateTextureFromData("faketext",IntVec2(4,4),4,fakeImageData);
		g_engine->m_render->BindTexture(testTexture2);
		g_engine->m_render->DrawVertexArray(6, tempAABB2verts);


		//////////////////////////////////////////////////////////////////////////////////

	}

	if (m_currentGameState == GAMESTATE_MENU)
	{
		
	}

	g_engine->m_render->EndCamera( attractCamera);
}