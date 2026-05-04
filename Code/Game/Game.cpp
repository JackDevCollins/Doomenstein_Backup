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
#include "Game/Actor.hpp"
#include "Game/Weapon.hpp"
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
#include "Engine/Renderer/SimpleTriangleFont.hpp"
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
	m_players.reserve(2);
	m_players.push_back(nullptr);
	m_players.push_back(nullptr);
	AABB2 screenBounds = AABB2(m_screenCamera->GetOrthoBottomLeft(), m_screenCamera->GetOrthoTopRight());
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
	ActorDefinition::InitializeDefinitions("Data/Definitions/ProjectileActorDefinitions.xml");
	WeaponDefinition::InitializeDefinitions("Data/Definitions/WeaponDefinitions.xml");
	ActorDefinition::InitializeDefinitions("Data/Definitions/ActorDefinitions.xml");

// 	PlayerController* playerOne = new PlayerController();
// 	playerOne->m_playerNum = 1;
//	m_players.push_back(playerOne);
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

	if (m_currentGameState == GAMESTATE_ATTRACT)
	{
		UpdateAttractMode( deltaSeconds );
	}
	
	if (m_currentGameState == GAMESTATE_LOBBY)
	{
		UpdateLobbyMode(deltaSeconds);
	}
	
	CheckInputs();

	UpdateEntities(deltaSeconds);

	if (m_currentMap)
	{
		m_currentMap->Update(static_cast<float>(m_gameClock->GetDeltaSeconds()));
	}

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

	

	if (m_currentGameState == GameState::GAMESTATE_ATTRACT)
	{
 		RenderAttractMode();
	}
	if (m_currentGameState == GAMESTATE_LOBBY)
	{
		RenderLobbyMode();
	}

	if (m_currentGameState == GameState::GAMESTATE_GAME)
	{
		for (PlayerController* player : m_players)	// for each range based loop
		{
			if (player == nullptr) continue;
			g_engine->m_render->BeginCamera(*player->m_camera);

			m_currentMap->Render();

			RenderEntities();

			RenderText();

			DebugRenderWorld(*player->m_camera);

			g_engine->m_render->EndCamera(*player->m_camera);
		}

	}

	//	For UI elements
	g_engine->m_render->BeginCamera(*m_screenCamera);

		AABB2 screenBounds = AABB2(m_screenCamera->GetOrthoBottomLeft(), m_screenCamera->GetOrthoTopRight());

		g_engine->m_devConsole->Render(AABB2(Vec2(m_screenCamera->GetOrthoBottomLeft()), Vec2(m_screenCamera->GetOrthoTopRight())));
		Vec3 position;
		EulerAngles orientation;
	
		char buffer[32];

		sprintf_s(buffer, "%.2f", m_gameClock->GetTotalSeconds());
		std::string displayTime = buffer;

		sprintf_s(buffer, "%.2f", g_systemClock->GetFrameRate());
		std::string displayFPS = buffer;

		sprintf_s(buffer, "%.2f", g_systemClock->GetTimeScale());
		std::string displayTimeScale = buffer;
		if (m_currentGameState == GAMESTATE_GAME)
		{
			RenderWeapon();
			RenderHUD();
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
		//DebugAddScreenText(controlModeString, AABB2(Vec2(screenBounds.m_mins.x + 300.f ,screenBounds.m_maxs.y - 20.f),screenBounds.m_maxs), 20.f, Vec2(0.f,1.f), 0.f);

		DebugAddScreenText("GameTime: " + displayTime + " FPS: " + displayFPS + " Scale: " + displayTimeScale, 
				AABB2(Vec2(screenBounds.m_mins.x,screenBounds.m_maxs.y - 20.f),screenBounds.m_maxs), 20.f, Vec2(1.f,0), 0.f);
		DebugRenderScreen(*m_screenCamera);

	g_engine->m_render->EndCamera(*m_screenCamera);
}

void Game::Shutdown()
{

}

void Game::CheckInputs()
{
	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_ESC))			// exit to attract screen or close app
	{
		if (m_currentGameState == GAMESTATE_ATTRACT)
		{
			g_app->SetIsQuitting();
		}
		if (m_currentGameState == GAMESTATE_LOBBY)
		{
			if (m_joinedPlayers == 1)
			{
				for (PlayerController* player : m_players)
				{
					LeaveLobby(player);
					m_nextGameState = GameState::GAMESTATE_ATTRACT;
				}
			}
			else
			{
				for (PlayerController* player : m_players)
				{
					if (player != nullptr && player->m_isKeyboardPlayer)
					{
						LeaveLobby(player);
					}
				}
			}
		}
		if (m_currentGameState == GAMESTATE_GAME)
		{
			m_gameClock->Pause();
			m_nextGameState = GAMESTATE_LOBBY;
		}
	}

	if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::Select))
	{
		if (m_currentGameState == GAMESTATE_ATTRACT)
		{
			g_app->SetIsQuitting();
		}
		if (m_currentGameState == GAMESTATE_LOBBY)
		{
			if (m_players.size() == 1)
			{
				for (PlayerController* player : m_players)
				{
					LeaveLobby(player);
					m_nextGameState = GameState::GAMESTATE_ATTRACT;
				}
			}
			else
			{
				for (PlayerController* player : m_players)
				{
					if (!player->m_isKeyboardPlayer)
					{
						LeaveLobby(player);
					}
				}
			}
		}
		if (m_currentGameState == GAMESTATE_GAME)
		{
			m_gameClock->Pause();
			m_nextGameState = GAMESTATE_LOBBY;
		}
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_SPACE))
	{
		if (m_currentGameState == GAMESTATE_ATTRACT)								// enter the lobby as player one with keyboard controls
		{
			m_nextGameState = GAMESTATE_LOBBY;
			JoinLobby(1);
		}
		if (m_currentGameState == GAMESTATE_LOBBY)
		{
			if (m_joinedPlayers > 1)
			{
				m_nextGameState = GAMESTATE_GAME;
			}
			if (m_joinedPlayers == 1)
			{
				bool keyboardPresent = false;
				for (PlayerController* player : m_players)
				{
					if (player != nullptr && player->m_isKeyboardPlayer == true)
					{
						keyboardPresent = true;
					}
				}
				if (keyboardPresent == true)
				{
					m_nextGameState = GAMESTATE_GAME;
				}
				else
				{
					JoinLobby(1);
				}
			}

// 			for (PlayerController* player : m_players)
// 			{
// 				if (!player->m_isKeyboardPlayer && (m_players[0] == nullptr || m_players[1] == nullptr))
// 				{
// 					JoinLobby(1);
// 				}
// 			}
		}
	}
	if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::Start))
	{
		if (m_currentGameState == GAMESTATE_ATTRACT)								// enter the lobby as player one with keyboard controls
		{
			m_nextGameState = GAMESTATE_LOBBY;
			JoinLobby(2);
		}
		if (m_currentGameState == GAMESTATE_LOBBY)
		{
			if (m_joinedPlayers > 1 )
			{
				m_nextGameState = GAMESTATE_GAME;
			}
			if (m_joinedPlayers == 1)
			{
				bool gamepadPresent = false;
				for (PlayerController* player : m_players)
				{
					if (player != nullptr && player->m_isKeyboardPlayer == false)
					{
						gamepadPresent = true;
					}
				}
				if (gamepadPresent == true)
				{
					m_nextGameState = GAMESTATE_GAME;
				}
				else
				{
					JoinLobby(2);
				}
			}
			if (m_players[0]->m_isKeyboardPlayer && m_players.size() < 2)			// add a 2nd player with keyboard controls
			{
				JoinLobby(2);
			}
		}
	}

	if (g_engine->m_input->WasKeyJustPressed('P') || (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::Start)))					// p to pause
	{
		//g_app->m_isPaused = !g_app->m_isPaused;
		m_gameClock->TogglePause();
		DebugAddMessage("Paused", 5);
	}

	if (g_engine->m_input->WasKeyJustPressed('O'))
	{
		m_gameClock->StepSingleFrame();
		DebugAddMessage("Step", .5f);
		//g_app->m_pauseAfterNextUpdate = true;
		//g_app->m_isPaused = false;
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_TILDE))		// ~ toggle dev console
	{
		g_engine->m_devConsole->ToggleOpen();
	}
	if (!m_players.empty())
	{
		if (g_engine->m_input->WasKeyJustPressed('F'))			// toggle possession camera
		{
			m_players[0]->ToggleCameraMode();
		}
		if (g_engine->m_input->WasKeyJustPressed('N'))			// Possess Next Actor
		{
			m_players[0]->PossessNextActor();
		}
		if (g_engine->m_input->WasKeyJustPressed('K'))			// kill yourself NOW
		{
			m_players[0]->GetActor()->Damage(nullptr, 200.f);
		}
	}
	
	///////// Lighting Controls /////////
	if (m_currentGameState == GAMESTATE_GAME)
	{
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
	}
}
	

void Game::RenderEntities() const
{
}

void Game::RenderText() const 
{
	for (PlayerController* player : m_players)
	{
		if (player == nullptr) return;
		g_engine->m_render->SetModelConstants(GetBillboardTransform(BillboardType::FULL_FACING, player->GetModelToWorldTransform(), Vec3(0, 0, 5)));
		g_engine->m_render->BindTexture(nullptr);
		g_engine->m_render->BindShader(nullptr);
		g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_engine->m_render->SetDepthMode(DepthMode::READ_ONLY_LESS_EQUAL);
		g_engine->m_render->SetBlendMode(BlendMode::ALPHA);

		std::vector<Vertex> textVerts;

		BitmapFont& font = *g_defaultFont;

		font.AddVertsForText3DAtOriginXForward(textVerts, 1.f, "butt");

		g_engine->m_render->BindTexture(&font.GetTexture());
		g_engine->m_render->DrawVertexArray(textVerts);
	}
}


void Game::UpdateEntities([[maybe_unused]]float deltaSeconds)
{
	for (PlayerController* player : m_players)
	{
		if (player && player->m_actorHandle != ActorHandle::INVALID)
		{
			player->UpdateInput();
			player->UpdateCamera();
 		}
	}	
}

void Game::UpdateCameras([[maybe_unused]]float deltaSeconds)
{
	Vec2 clientDimentions = Vec2(static_cast<float>(g_engine->m_window->GetClientDimensions().x), static_cast<float>(g_engine->m_window->GetClientDimensions().y));
	m_screenCamera->SetOrthoView(Vec2(0.f, 0.f), clientDimentions);

// 	float tempAspect = g_engine->m_window->m_config.m_clientAspect;										// #ToDo my client aspect from config (16/10) does not give the desired results. 2 does
//  	m_playerCamera->SetPerspectiveView(tempAspect, 60.f, 0.1f, 100.0f);
//  	
//  	if (m_player)
//  	{
//  		m_playerCamera->SetPositionAndOrientation(m_player->m_position, m_player->m_orientation);
//  		m_playerCamera->SetCameraToRenderTransform(m_playerCamera->GetCameraToRenderTransform());
//  	}
	
}


void Game::RenderHUD() const
{
	char buffer[32];
	std::vector<Vertex> HUDVerts;

	for (PlayerController* player : m_players)
	{
		if (player != nullptr)
		{
			AABB2 playerViewport = player->m_camera->GetViewport();
			std::vector<Vertex> deathScreen;
			if (playerViewport == AABB2())
			{
				playerViewport = AABB2(m_screenCamera->GetOrthoBottomLeft(), m_screenCamera->GetOrthoTopRight());
			}
			AABB2 HUDrect = AABB2(Vec2(0.f, playerViewport.m_mins.y),Vec2(playerViewport.m_maxs.x, playerViewport.m_mins.y + (playerViewport.GetDimensions().y * .18)));
			AddVertsForAABB2D(HUDVerts, HUDrect, Rgba8::WHITE);
			
			std::string health = "0";
			if (player->GetActor() != nullptr)
			{
				sprintf_s(buffer, "%03d", (int)player->GetActor()->m_health);
				health = buffer;

				DebugAddScreenText(" " + health + " ", AABB2(Vec2(playerViewport.m_mins.x, playerViewport.m_mins.y + (playerViewport.GetDimensions().y * .05)),
				Vec2(playerViewport.m_maxs.x, playerViewport.m_mins.y + 100.f + (playerViewport.GetDimensions().y * .05))), 100.f, Vec2(.255f, .745f), 0.f, Rgba8::GREEN);

				Texture* HudTexture = g_engine->m_render->CreateOrGetTextureFromFile(player->GetActor()->m_currentWeapon->m_definition->m_baseTexture.c_str());		//g_engine->m_render->CreateOrGetTextureFromFile("Data/Images/Hud_Base.png");
				g_engine->m_render->SetModelConstants();
				g_engine->m_render->BindShader(g_engine->m_render->CreateOrGetShader(player->GetActor()->m_currentWeapon->m_definition->m_HUDshader.c_str()));
				g_engine->m_render->SetBlendMode(BlendMode::ALPHA);
				g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
				g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
				g_engine->m_render->BindTexture(HudTexture);
				g_engine->m_render->DrawVertexArray(HUDVerts);

				std::vector<Vertex> reticleVerts;
				Vec2 centerViewport = playerViewport.GetCenter();
				Vec2 reticleBL = Vec2(centerViewport.x - player->GetActor()->m_currentWeapon->m_definition->m_reticleSize.x, centerViewport.y - player->GetActor()->m_currentWeapon->m_definition->m_reticleSize.y);
				Vec2 reticleTR = Vec2(centerViewport.x + player->GetActor()->m_currentWeapon->m_definition->m_reticleSize.x, centerViewport.y + player->GetActor()->m_currentWeapon->m_definition->m_reticleSize.y);
				AABB2 reticleBox = AABB2(reticleBL,reticleTR);
				Texture* reticleTexture =  g_engine->m_render->CreateOrGetTextureFromFile(player->GetActor()->m_currentWeapon->m_definition->m_reticleTexture.c_str());
				AddVertsForAABB2D(reticleVerts, reticleBox, Rgba8::WHITE);
				//g_engine->m_render->BindShader(nullptr);
				g_engine->m_render->BindTexture(reticleTexture);
				g_engine->m_render->DrawVertexArray(reticleVerts);
			}

			if (player->GetActor()->m_isDead)
			{
				AddVertsForAABB2D(	deathScreen, playerViewport, Rgba8(110, 110, 110, 200));
				g_engine->m_render->SetModelConstants();
				g_engine->m_render->SetBlendMode(BlendMode::ALPHA);
				g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
				g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
				g_engine->m_render->BindTexture(nullptr);
				g_engine->m_render->DrawVertexArray(deathScreen);
			}
		}

	}
}

void Game::RenderWeapon() const
{
	std::vector<Vertex> weaponVerts;
	for (PlayerController* player : m_players)
	{
		if (player != nullptr)
		{
			Weapon* currentweapon = player->GetActor()->m_currentWeapon;
			AABB2 playerViewport = player->m_camera->GetViewport();

			if (playerViewport == AABB2())
			{
				playerViewport = AABB2(m_screenCamera->GetOrthoBottomLeft(), m_screenCamera->GetOrthoTopRight());
			}
			float HUDTopHeight = playerViewport.m_mins.y + (playerViewport.GetDimensions().y * .18);
			Vec2 centerAboveHUD = Vec2( playerViewport.GetCenter().x, playerViewport.m_mins.y + HUDTopHeight);
			Vec2 weaponBottomLeft = Vec2(centerAboveHUD.x - currentweapon->m_definition->m_spriteSize.x, centerAboveHUD.y);
			Vec2 weaponTopRight = Vec2(centerAboveHUD.x + currentweapon->m_definition->m_spriteSize.x, centerAboveHUD.y + (weaponBottomLeft.y - playerViewport.m_mins.y) + currentweapon->m_definition->m_spriteSize.y);
			AABB2 weaponRect = AABB2(weaponBottomLeft, weaponTopRight);

			SpriteDefinition const& spriteAtTime = currentweapon->m_currentPlayingAnimation->GetSpriteDefAtTime((float)currentweapon->m_animationClock->GetTotalSeconds());
			AABB2 UVs = spriteAtTime.GetUVs();

			AddVertsForAABB2D(weaponVerts, weaponRect, Rgba8::WHITE, UVs);

				Texture* weaponTexture = spriteAtTime.GetSpriteSheet()->GetTexture();		//g_engine->m_render->CreateOrGetTextureFromFile("Data/Images/Hud_Base.png");
				g_engine->m_render->SetModelConstants();
				g_engine->m_render->BindShader(g_engine->m_render->CreateOrGetShader(player->GetActor()->m_currentWeapon->m_definition->m_HUDshader.c_str()));
				g_engine->m_render->SetBlendMode(BlendMode::ALPHA);
				g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
				g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
				g_engine->m_render->BindTexture(currentweapon->m_weaponSpriteSheet->GetTexture());
				g_engine->m_render->DrawVertexArray(weaponVerts);

				std::vector<Vertex> reticleVerts;
				Vec2 centerViewport = playerViewport.GetCenter();
				Vec2 reticleBL = Vec2(centerViewport.x - player->GetActor()->m_currentWeapon->m_definition->m_reticleSize.x, centerViewport.y - player->GetActor()->m_currentWeapon->m_definition->m_reticleSize.y);
				Vec2 reticleTR = Vec2(centerViewport.x + player->GetActor()->m_currentWeapon->m_definition->m_reticleSize.x, centerViewport.y + player->GetActor()->m_currentWeapon->m_definition->m_reticleSize.y);
				AABB2 reticleBox = AABB2(reticleBL, reticleTR);
				Texture* reticleTexture = g_engine->m_render->CreateOrGetTextureFromFile(player->GetActor()->m_currentWeapon->m_definition->m_reticleTexture.c_str());
				AddVertsForAABB2D(reticleVerts, reticleBox, Rgba8::WHITE);
				//g_engine->m_render->BindShader(nullptr);
				g_engine->m_render->BindTexture(reticleTexture);
				g_engine->m_render->DrawVertexArray(reticleVerts);
			}
	}

}

void Game::JoinLobby(int controller)
{	// 1 == keyboard 2 == gamepad
	if (m_players[0] == nullptr && m_players[1] == nullptr)
	{
		PlayerController* playerOne = new PlayerController();
		playerOne->m_playerNum = 1;
		m_joinedPlayers += 1;
		if (controller == 1)
		{
			playerOne->m_isKeyboardPlayer = true;
		}
		else
			playerOne->m_isKeyboardPlayer = false;

		m_players[0] = (playerOne);
		return;
	}
	if (m_players[0] == nullptr)
	{
		PlayerController* playerOne = new PlayerController();
		playerOne->m_playerNum = 1;
		if (controller == 1)
		{
			playerOne->m_isKeyboardPlayer = true;
		}
		else
			playerOne->m_isKeyboardPlayer = false;

		m_players[0] = playerOne;
		return;
	}
	if (m_joinedPlayers == 1 && m_players[0] != nullptr)
	{
		PlayerController* playerTwo = new PlayerController();
		playerTwo->m_playerNum = 2;
		m_joinedPlayers += 1;
		if (controller == 1)
		{
			playerTwo->m_isKeyboardPlayer = true;
		}
		else
			playerTwo->m_isKeyboardPlayer = false;

		m_players[1] = playerTwo;
		return;
	}
}

void Game::LeaveLobby(PlayerController* playerController)
{
	if (m_joinedPlayers > 1)
	{
		if (m_players[1] == playerController)
		{
			m_joinedPlayers -= 1;
			delete playerController;
			m_players[1] = nullptr;
		}
		if (m_players[0] == playerController)
		{
			m_joinedPlayers -= 1;
			delete playerController;
			m_players[0] = nullptr;
		}
	}
}

void Game::EnterState(GameState state)	// state is what you are entering
{
	if (state == GAMESTATE_GAME)
	{
		if (m_players.empty())
		{
			PlayerController* playerOne = new PlayerController();
			m_players.push_back(playerOne);
		}
		if (m_currentMap == nullptr)
		{
			std::string mapToLoad = g_globalConfigBlackboard.GetValue("defaultMap", "defaultMap");
			m_currentMap = new Map(this, MapDefinition::GetByName(mapToLoad));
			m_currentMap->Startup();
		}

	}
}

void Game::ExitState(GameState state)	// state is what you are exiting
{
	if (state == GAMESTATE_GAME)
	{
		delete m_currentMap;
		m_currentMap = nullptr;

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
	
}

void Game::RenderAttractMode() const			// #todo add pulsing to start button? ship or circle? done with cosine somehow
{
	Camera attractCamera;
	g_engine->m_render->SetModelConstants();
	g_engine->m_render->BindShader(nullptr);
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

		std::vector<Vertex> AttractScreenVerts;

		AddVertsForTextTriangles2D(AttractScreenVerts, "Press SPACE to join with mouse and keyboard", Vec2(500,100), 20.f, Rgba8::WHITE);
		AddVertsForTextTriangles2D(AttractScreenVerts, "Press START to join with controller", Vec2(550,75), 20.f, Rgba8::WHITE);
		AddVertsForTextTriangles2D(AttractScreenVerts, "Press ESCAPE or BACK to exit", Vec2(600,50), 20.f, Rgba8::WHITE);

		g_engine->m_render->DrawVertexArray(AttractScreenVerts);
		//////////////////////////////////////////////////////////////////////////////////
	}

	g_engine->m_render->EndCamera( attractCamera);
}

void Game::UpdateLobbyMode(float deltaSeconds)
{

}

void Game::RenderLobbyMode() const
{	
	Camera LobbyCamera;
	g_engine->m_render->SetModelConstants();
	g_engine->m_render->BindShader(nullptr);
	g_engine->m_render->SetBlendMode(BlendMode::ALPHA);
	g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_engine->m_render->BindTexture(nullptr);

	AABB2 screenbounds = AABB2(m_screenCamera->GetOrthoBottomLeft(), m_screenCamera->GetOrthoTopRight());
	std::vector<Vertex> LobbyVerts;
	Vec2 center = screenbounds.GetCenter();

	Vec2 bottomCenter = screenbounds.ChopTop(.5f, 0.f).GetCenter();
	Vec2 topCenter = screenbounds.ChopBottom(.5f, 0).GetCenter();

	LobbyCamera.SetOrthoView(screenbounds.m_mins, screenbounds.m_maxs);
	g_engine->m_render->BeginCamera(LobbyCamera);

	g_engine->m_render->BindTexture(&g_defaultFont->GetTexture());

	if (m_joinedPlayers == 1 && m_players[0] != nullptr)
	{
		g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, "Player 1", AABB2(Vec2(center.x - 400, center.y -80.f),Vec2(center.x + 400.f, center.y + 80.f)), 60.f);
		if (m_players[0]->m_isKeyboardPlayer)
		{
			g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, "Mouse and Keyboard", AABB2(Vec2(center.x - 300.f, center.y -80.f),Vec2(center.x + 300.f, center.y -50.f)), 30.f);
		}
		if (!m_players[0]->m_isKeyboardPlayer)
		{
			g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, "Controller", AABB2(Vec2(center.x - 300.f, center.y - 80.f), Vec2(center.x + 300.f, center.y - 50.f)), 30.f);
		}

		if (m_players[0]->m_isKeyboardPlayer)
		{
			g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, " Press SPACE to start game\n Press ESCAPE to leave game\n Press START to join player",  AABB2(Vec2(center.x - 300.f, center.y -350.f),Vec2(center.x + 300.f, center.y -200.f)), 25.f);
		}
		if (!m_players[0]->m_isKeyboardPlayer)
		{
			g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, " Press START to start game\n Press SELECT to leave game\n Press SPACE to join player", AABB2(Vec2(center.x - 300.f, center.y - 350.f), Vec2(center.x + 300.f, center.y - 200.f)), 25.f);
		}
		
	}
	else if (m_joinedPlayers == 1 && m_players[1] != nullptr)
	{
		g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, "Player 2", AABB2(Vec2(center.x - 400, center.y - 80.f), Vec2(center.x + 400.f, center.y + 80.f)), 60.f);
		if (m_players[1]->m_isKeyboardPlayer)
		{
			g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, "Mouse and Keyboard", AABB2(Vec2(center.x - 300.f, center.y - 80.f), Vec2(center.x + 300.f, center.y - 50.f)), 30.f);
		}
		if (!m_players[1]->m_isKeyboardPlayer)
		{
			g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, "Controller", AABB2(Vec2(center.x - 300.f, center.y - 80.f), Vec2(center.x + 300.f, center.y - 50.f)), 30.f);
		}

		if (m_players[1]->m_isKeyboardPlayer)
		{
			g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, " Press SPACE to start game\n Press ESCAPE to leave game\n Press START to join player", AABB2(Vec2(center.x - 300.f, center.y - 350.f), Vec2(center.x + 300.f, center.y - 200.f)), 25.f);
		}
		if (!m_players[1]->m_isKeyboardPlayer)
		{
			g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, " Press START to start game\n Press SELECT to leave game\n Press SPACE to join player", AABB2(Vec2(center.x - 300.f, center.y - 350.f), Vec2(center.x + 300.f, center.y - 200.f)), 25.f);
		}
		
	}

	if (m_players[0] != nullptr && m_players[1] != nullptr)
	{
		g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, "Player 1", AABB2(Vec2(topCenter.x - 400, topCenter.y -80.f),Vec2(topCenter.x + 400.f, topCenter.y + 80.f)), 60.f);
		if (m_players[0]->m_isKeyboardPlayer)
		{
			g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, "Mouse and Keyboard", AABB2(Vec2(topCenter.x - 300.f, topCenter.y - 80.f), Vec2(topCenter.x + 300.f, topCenter.y - 50.f)), 30.f);
		}
		if (!m_players[0]->m_isKeyboardPlayer)
		{
			g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, "Controller", AABB2(Vec2(topCenter.x - 300.f, topCenter.y - 80.f), Vec2(topCenter.x + 300.f, topCenter.y - 50.f)), 30.f);
		}

		g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, "Player 2", AABB2(Vec2(bottomCenter.x - 400, bottomCenter.y -80.f),Vec2(bottomCenter.x + 400.f, bottomCenter.y + 80.f)), 60.f);
		if (m_players[1]->m_isKeyboardPlayer)
		{
			g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, "Mouse and Keyboard", AABB2(Vec2(bottomCenter.x - 300.f, bottomCenter.y - 80.f), Vec2(bottomCenter.x + 300.f, bottomCenter.y - 50.f)), 30.f);
		}
		if (!m_players[1]->m_isKeyboardPlayer)
		{
			g_defaultFont->AddVertsForTextInBox2D(LobbyVerts, "Controller",AABB2(Vec2(bottomCenter.x - 300.f, bottomCenter.y - 80.f), Vec2(bottomCenter.x + 300.f, bottomCenter.y - 50.f)), 30.f);
		}
	}


	g_engine->m_render->DrawVertexArray(LobbyVerts);

	g_engine->m_render->EndCamera(LobbyCamera);
}
