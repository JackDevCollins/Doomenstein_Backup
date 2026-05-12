#include "Game/PlayerController.hpp"
#include "Game/Controller.hpp"
#include "Game/App.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/Vec3.hpp"

PlayerController::PlayerController()
{
	m_camera = new Camera();
	m_camera->SetCameraToRenderTransform(Mat44::CAMERA_TO_RENDER);
}

PlayerController::~PlayerController()
{
	delete m_camera;
	m_camera = nullptr;
}

void PlayerController::UpdateInput()
{
	if (m_cameraMode == FREEFLY)	// if in fps mode
	{
		FreeFlyInput();
	}
	if (m_cameraMode == FPS)
	{
		ActorInput();
	}

	if (m_cameraMode == CameraMode::SKATER)
	{
		if (m_skateTimer == nullptr)
		{
			m_skateTimer = new Timer(SKATEMAXPUSHDURATION, m_map->m_game->m_gameClock);
			m_jumpTimer = new Timer(1.2f, m_map->m_game->m_gameClock);
		}

		SkaterInput();
	}
}

void PlayerController::UpdateCamera()
{
	AABB2 screenBounds = AABB2(m_map->m_game->m_screenCamera->GetOrthoBottomLeft(), m_map->m_game->m_screenCamera->GetOrthoTopRight());
	float aspect = g_engine->m_window->m_config.m_clientAspect;
	float screenHeight = screenBounds.m_maxs.y;
	float screenHalfHeight = screenHeight * .5f;

	if (m_map->m_game->m_joinedPlayers > 1)
	{
		if (m_playerNum == 1)
		{
			screenBounds.m_mins.y = screenHalfHeight;
			screenBounds.m_maxs.y = screenHeight;
		}
		if (m_playerNum == 2)
		{
			screenBounds.m_mins.y = 0.f;
			screenBounds.m_maxs.y = screenHalfHeight;
		}
		Vec2 dimensions = screenBounds.GetDimensions();
		aspect = dimensions.x / dimensions.y;

		m_camera->SetViewport(screenBounds);
	}

	if (GetActor()->m_definition->m_faction == "Demon")
	{
		m_camera->SetPerspectiveView(aspect, 100.f, 0.1f, 100.0f);
	}
	else
	{
		m_camera->SetPerspectiveView(aspect, 60.f, 0.1f, 100.0f);
	}

	if (m_cameraMode == FPS)
	{
		float eyeHeight;

		if (!GetActor()->m_isDead)
		{
			eyeHeight = GetActor()->m_definition->m_cameraEyeHeight;
		}
		else
		{
			eyeHeight = GetClampedZeroToOne(1.f - (GetActor()->m_decomposeTimer->GetElapsedFraction() * 2.f)) * GetActor()->m_definition->m_cameraEyeHeight;
		}
		Vec3 actorPosition = GetActor()->m_position;
		Vec3 cameraPosition = Vec3(actorPosition.x, actorPosition.y, eyeHeight); 
		m_camera->SetPosition(cameraPosition);
		m_camera->SetOrientation(GetActor()->m_orientation);
	}
	if (m_cameraMode == FREEFLY)
	{
		m_camera->SetPosition(m_position);
		m_camera->SetOrientation(m_orientation);
	}
	if (m_cameraMode == CameraMode::SKATER)
	{
		float eyeHeight = GetActor()->m_definition->m_cameraEyeHeight;

		if (GetActor()->m_isDead)
		{
			eyeHeight *= GetClampedZeroToOne(1.f - (GetActor()->m_decomposeTimer->GetElapsedFraction() * 2.f));
		}

		Vec3 focalPoint = GetActor()->m_position + Vec3(0.f, 0.f, eyeHeight);

		float cameraDistance = 2.f;

		Vec3 cameraForward = m_orientation.GetForwardDir_IFwd_JLeft_KUp();

		Vec3 cameraPosition = focalPoint - (cameraForward * cameraDistance);

		m_camera->SetPosition(cameraPosition);
		m_camera->SetOrientation(m_orientation);
	}
}

void PlayerController::ChangeCameraMode(CameraMode cameraMode)
{
	m_cameraMode = cameraMode;

	if (cameraMode == CameraMode::FPS)
	{
		m_position = GetActor()->m_position + Vec3(0.f, 0.f, GetActor()->m_definition->m_cameraEyeHeight);
		m_orientation = GetActor()->m_orientation;
	}

	if (cameraMode == CameraMode::SKATER)
	{
		GetActor()->m_orientation = m_orientation;
		GetActor()->m_orientation.m_pitchDegrees = 0.f;
	}

	if (cameraMode == CameraMode::FREEFLY)
	{
		m_position = GetActor()->m_position + Vec3(0.f, 0.f, GetActor()->m_definition->m_cameraEyeHeight);
		m_orientation = GetActor()->m_orientation;
	}

}

void PlayerController::PossessNextActor()
{
	int numActors = (int)m_map->m_actors.size();
	int currentIndex = m_actorHandle.GetIndex();
	
	for (int index = 1;  index < numActors; ++index)
	{
		int testIndex = (currentIndex + index) % numActors;
		Actor* testActor = m_map->m_actors[testIndex];
		
		if (testActor && testActor->m_definition->m_canBePossessed)
		{
			GetActor()->OnUnPossessed();
			m_actorHandle = ActorHandle::INVALID;
			Possess(testActor->m_handle);
			return;
		}
	}
}

void PlayerController::FreeFlyInput()
{
	float deltaSeconds = (float)g_systemClock->GetDeltaSeconds();

	const XboxController& controller = g_engine->m_input->GetController(0);
	Vec2 moveInputLeft = controller.GetLeftStick().GetPosition();
	Vec2 moveInputRight = controller.GetRightStick().GetPosition();

	if (g_engine->m_input->m_cursorState.m_cursorClientDelta != Vec2(0.f, 0.f))
	{
		m_orientation.m_yawDegrees	 = m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetOrientationAboutZDegrees() + (g_engine->m_input->m_cursorState.m_cursorClientDelta.x * 0.075f);
		m_orientation.m_pitchDegrees = m_orientation.m_pitchDegrees - (g_engine->m_input->m_cursorState.m_cursorClientDelta.y * 0.075f);

		if (m_orientation.m_pitchDegrees > 90.f)  m_orientation.m_pitchDegrees = 89.9f;
		if (m_orientation.m_pitchDegrees < -90.f) m_orientation.m_pitchDegrees = -89.9f;
	}

	if (g_engine->m_input->WasKeyJustPressed('P'))		// Pause the game
	{
		m_map->m_game->m_gameClock->TogglePause();
	}

	float speed = 0.f;
	if (g_engine->m_input->IsKeyDown(KEYCODE_SHIFT) || (g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::A)))	// Increase speed by a factor of 15 if held
	{
		speed = GetActor()->m_definition->m_runSpeed;
	}
	else
	{
		speed = GetActor()->m_definition->m_walkSpeed;
	}

	if (g_engine->m_input->IsKeyDown('A'))		// translate positive J							
	{
		Vec3 forward = m_orientation.GetForwardDir_IFwd_JLeft_KUp();
		forward.z = 0.f;
		forward.GetNormalized();
		m_position += forward.GetRotatedAboutZDegrees(90).GetNormalized() * speed * deltaSeconds;
	}

	if (g_engine->m_input->IsKeyDown('D'))		// translate negative J
	{
		Vec3 forward = m_orientation.GetForwardDir_IFwd_JLeft_KUp();
		forward.z = 0.f;
		forward.GetNormalized();
		m_position -= forward.GetRotatedAboutZDegrees(90).GetNormalized() * speed * deltaSeconds;
	}

	if (g_engine->m_input->IsKeyDown('W'))		// translate positive I
	{
		m_position += m_orientation.GetForwardDir_IFwd_JLeft_KUp() * speed * deltaSeconds;
	}

	if (g_engine->m_input->IsKeyDown('S'))		// translate negative I
	{
		m_position -= m_orientation.GetForwardDir_IFwd_JLeft_KUp() * speed * deltaSeconds;
	}

	if (g_engine->m_input->IsKeyDown('Z') || (g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::L_Bumper)))		// translate positive K
	{
		m_position.z += speed * deltaSeconds;
	}

	if (g_engine->m_input->IsKeyDown('C') || (g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::R_Bumper)))		// translate negative K
	{
		m_position.z -= speed * deltaSeconds;
	}
}

void PlayerController::ActorInput()
{
	const XboxController& controller = g_engine->m_input->GetController(0);
	Vec2 moveInputLeft = controller.GetLeftStick().GetPosition();
	Vec2 moveInputRight = controller.GetRightStick().GetPosition();

	if (GetActor() == nullptr) return;
	if (m_isKeyboardPlayer)		// KEYBOARD CONTROLS
	{
		if (g_engine->m_input->m_cursorState.m_cursorClientDelta != Vec2(0.f, 0.f))
		{
			GetActor()->m_orientation.m_yawDegrees = GetActor()->m_orientation.m_yawDegrees + (g_engine->m_input->m_cursorState.m_cursorClientDelta.x * 0.075f);
			GetActor()->m_orientation.m_pitchDegrees = GetActor()->m_orientation.m_pitchDegrees - (g_engine->m_input->m_cursorState.m_cursorClientDelta.y * 0.075f);

			if (GetActor()->m_orientation.m_pitchDegrees > 90.f)  GetActor()->m_orientation.m_pitchDegrees = 89.9f;
			if (GetActor()->m_orientation.m_pitchDegrees < -90.f) GetActor()->m_orientation.m_pitchDegrees = -89.9f;

			m_orientation = GetActor()->m_orientation;
		}
		if (g_engine->m_input->WasKeyJustPressed('P'))		// Pause the game
		{
			m_map->m_game->m_gameClock->TogglePause();
		}
		if (g_engine->m_input->IsKeyDown(KEYCODE_LEFT_MOUSE))	// Fire weapon
		{
			if (GetActor()->m_currentWeapon != nullptr)
			{
				GetActor()->m_currentWeapon->Fire();
			}
		}
		if (g_engine->m_input->WasKeyJustPressed('1'))			// weapon slot 1
		{
			if (GetActor()->m_inventory.size() > 1)
			{
				GetActor()->EquipWeapon(0);
			}
		}
		if (g_engine->m_input->WasKeyJustPressed('2'))			// weapon slot 2
		{
			if (GetActor()->m_inventory.size() > 1)
			{
				GetActor()->EquipWeapon(1);
			}
		}
		if (g_engine->m_input->WasKeyJustPressed('3'))			// weapon slot 2
		{
			if (GetActor()->m_inventory.size() > 1)
			{
				GetActor()->EquipWeapon(2);
			}
		}
		if (g_engine->m_input->WasKeyJustPressed('R') )
		{
			ChangeCameraMode(CameraMode::SKATER);
		}
		if (g_engine->m_input->WasKeyJustPressed(KEYCODE_LEFTARROW) )
		{
			int currentIndex = 0;
			for (int invIndex = 0; invIndex < GetActor()->m_inventory.size(); ++invIndex)
			{
				if (GetActor()->m_inventory[invIndex]->m_name == GetActor()->m_currentWeapon->m_name)
				{
					currentIndex = invIndex;
				}
			}
			int newIndex = (currentIndex - 1) % GetActor()->m_inventory.size();
			GetActor()->EquipWeapon(newIndex);
		}
		if (g_engine->m_input->WasKeyJustPressed(KEYCODE_RIGHTARROW) )
		{
			int currentIndex = 0;
			for (int invIndex = 0; invIndex < GetActor()->m_inventory.size(); ++invIndex)
			{
				if (GetActor()->m_inventory[invIndex]->m_name == GetActor()->m_currentWeapon->m_name)
				{
					currentIndex = invIndex;
				}
			}
			int newIndex = (currentIndex + 1) % GetActor()->m_inventory.size();
			GetActor()->EquipWeapon(newIndex);
		}
		///// MOVEMENT CONTROLS //////

		Vec3 forward = GetActor()->m_orientation.GetForwardDir_IFwd_JLeft_KUp();
		forward.z = 0.f;
		forward = forward.GetNormalized();
		Vec3 left = forward.GetRotatedAboutZDegrees(90.f);
		Vec3 up = CrossProduct3D(forward, left);

		Vec3 actorFwd = GetActor()->m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetNormalized();
		Vec3 actorLeft = actorFwd.GetRotatedAboutZDegrees(90.f);

		float speed = 0.f;

		if (g_engine->m_input->IsKeyDown(KEYCODE_SHIFT))	// Increase speed by a factor of 15 if held
		{
			speed = GetActor()->m_definition->m_runSpeed;
		}
		else
		{
			speed = GetActor()->m_definition->m_walkSpeed;
		}


		if (g_engine->m_input->IsKeyDown('A'))		// translate positive J							
		{
			GetActor()->MoveInDirection(left, speed);
		}

		if (g_engine->m_input->IsKeyDown('D'))		// translate negative J
		{
			GetActor()->MoveInDirection(-left, speed);
		}

		if (g_engine->m_input->IsKeyDown('W'))		// translate positive I
		{
			GetActor()->MoveInDirection(forward, speed);
		}

		if (g_engine->m_input->IsKeyDown('S'))		// translate negative I
		{
			GetActor()->MoveInDirection(-forward, speed);
		}
	}
	///////////////////////////////////////////////////////////////
	if (!m_isKeyboardPlayer)		// controller controls
	{
		if (g_engine->m_input->GetController(0).GetRightTrigger() > 0.1)
		{
			if (GetActor()->m_currentWeapon != nullptr)
			{
				GetActor()->m_currentWeapon->Fire();
			}
		}
		if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::X))
		{
			if (GetActor()->m_inventory.size() > 1)
			{
				GetActor()->EquipWeapon(0);
			}
		}
		if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::Y))
		{
			if (GetActor()->m_inventory.size() > 1)
			{
				GetActor()->EquipWeapon(1);
			}
		}

		if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::D_Pad_Down))
		{
			int currentIndex = 0;
			for (int invIndex = 0; invIndex < GetActor()->m_inventory.size(); ++invIndex)
			{
				if (GetActor()->m_inventory[invIndex]->m_name == GetActor()->m_currentWeapon->m_name)
				{
					currentIndex = invIndex;
				}
			}
			int newIndex = (currentIndex - 1) % GetActor()->m_inventory.size();
			GetActor()->EquipWeapon(newIndex);
		}
		if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::D_Pad_Up))
		{
			int currentIndex = 0;
			for (int invIndex = 0; invIndex < GetActor()->m_inventory.size(); ++invIndex)
			{
				if (GetActor()->m_inventory[invIndex]->m_name == GetActor()->m_currentWeapon->m_name)
				{
					currentIndex = invIndex;
				}
			}
			int newIndex = (currentIndex + 1) % GetActor()->m_inventory.size();
			GetActor()->EquipWeapon(newIndex);
		}
		///// MOVEMENT CONTROLS //////
		float speed = 0.f;
		if ((g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::A)))	// Increase speed by a factor of 15 if held
		{
			speed = GetActor()->m_definition->m_runSpeed;
		}
		else
		{
			speed = GetActor()->m_definition->m_walkSpeed;
		}

		Vec3 forward = GetActor()->m_orientation.GetForwardDir_IFwd_JLeft_KUp();
		forward.z = 0.f;
		forward = forward.GetNormalized();
		Vec3 left = forward.GetRotatedAboutZDegrees(90.f);
		float speedMagnitude = 0;

		float rightStickMagnitude = controller.GetRightStick().GetMagnitude();
		if (rightStickMagnitude > 0.f)
		{
			Vec2 direction = controller.GetRightStick().GetPosition();
			GetActor()->TurnInDirection(Vec3(direction.x, direction.y, direction.y),(float)m_map->m_game->m_gameClock->GetDeltaSeconds()* speedMagnitude);
			
			//Vec3 goalDirection = Vec3::MakeFromPolarDegrees(0.f, controller.GetRightStick().GetOrientationDegrees());
			//GetActor()->TurnInDirection(goalDirection, m_map->m_game->m_gameClock->GetDeltaSeconds() * speedMagnitude); 
		}

		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		if (leftStickMagnitude > 0.f)
		{
			Vec2 direction2D = controller.GetLeftStick().GetPosition();
			speedMagnitude = leftStickMagnitude;
			Vec3 direction = Vec3(direction2D.x, direction2D.y, 0.f);
			speed = speed * speedMagnitude; 
			GetActor()->MoveInDirection(-direction, speed);
		}
	}
}

void PlayerController::SkaterInput()
{
	const XboxController& controller = g_engine->m_input->GetController(0);
	Vec2 moveInputLeft = controller.GetLeftStick().GetPosition();
	Vec2 moveInputRight = controller.GetRightStick().GetPosition();

	if (GetActor() == nullptr) return;
	if (m_isKeyboardPlayer)		// KEYBOARD CONTROLS
	{

		if (m_cameraMode == SKATER)
		{
			if (GetActor()->m_currentPlayingAnimationGroup->m_name != "Skate")
			{
				GetActor()->PlayAnimationByName("Ride", 8);
			}
		}

		if (g_engine->m_input->m_cursorState.m_cursorClientDelta != Vec2(0.f, 0.f))
		{
			m_orientation.m_yawDegrees = m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetOrientationAboutZDegrees() + (g_engine->m_input->m_cursorState.m_cursorClientDelta.x * 0.075f);
			m_orientation.m_pitchDegrees = m_orientation.m_pitchDegrees - (g_engine->m_input->m_cursorState.m_cursorClientDelta.y * 0.075f);

			if (m_orientation.m_pitchDegrees > 25.f)  m_orientation.m_pitchDegrees = 24.9f;
			if (m_orientation.m_pitchDegrees < 5.f) m_orientation.m_pitchDegrees = 5.1f;
		}
		if (g_engine->m_input->WasKeyJustPressed('P'))		// Pause the game
		{
			m_map->m_game->m_gameClock->TogglePause();
		}
		if (g_engine->m_input->IsKeyDown(KEYCODE_LEFT_MOUSE))	// Fire weapon
		{
			if (GetActor()->m_currentWeapon != nullptr)
			{
				GetActor()->m_currentWeapon->Fire();
			}
		}
		if (g_engine->m_input->WasKeyJustPressed('1'))			// weapon slot 1
		{
			if (GetActor()->m_inventory.size() > 1)
			{
				GetActor()->EquipWeapon(0);
			}
		}
		if (g_engine->m_input->WasKeyJustPressed('2'))			// weapon slot 2
		{
			if (GetActor()->m_inventory.size() > 1)
			{
				GetActor()->EquipWeapon(1);
			}
		}
		if (g_engine->m_input->WasKeyJustPressed('3'))			// weapon slot 3
		{
			if (GetActor()->m_inventory.size() > 1)
			{
				GetActor()->EquipWeapon(2);
			}
		}
		if (g_engine->m_input->WasKeyJustPressed('R'))			// skate mode
		{
			ChangeCameraMode(CameraMode::SKATER);
		}
		if (g_engine->m_input->WasKeyJustPressed(KEYCODE_LEFTARROW))
		{
			int currentIndex = 0;
			for (int invIndex = 0; invIndex < GetActor()->m_inventory.size(); ++invIndex)
			{
				if (GetActor()->m_inventory[invIndex]->m_name == GetActor()->m_currentWeapon->m_name)
				{
					currentIndex = invIndex;
				}
			}
			int newIndex = (currentIndex - 1) % GetActor()->m_inventory.size();
			GetActor()->EquipWeapon(newIndex);
		}
		if (g_engine->m_input->WasKeyJustPressed(KEYCODE_RIGHTARROW))
		{
			int currentIndex = 0;
			for (int invIndex = 0; invIndex < GetActor()->m_inventory.size(); ++invIndex)
			{
				if (GetActor()->m_inventory[invIndex]->m_name == GetActor()->m_currentWeapon->m_name)
				{
					currentIndex = invIndex;
				}
			}
			int newIndex = (currentIndex + 1) % GetActor()->m_inventory.size();
			GetActor()->EquipWeapon(newIndex);
		}
		///// MOVEMENT CONTROLS //////

		Vec3 forward = GetActor()->m_orientation.GetForwardDir_IFwd_JLeft_KUp();
		forward.z = 0.f;
		forward = forward.GetNormalized();
		Vec3 left = forward.GetRotatedAboutZDegrees(90.f);
		Vec3 up = CrossProduct3D(forward, left);

		Vec3 actorFwd = GetActor()->m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetNormalized();
		Vec3 actorLeft = actorFwd.GetRotatedAboutZDegrees(90.f);

		if (g_engine->m_input->WasKeyJustPressed(KEYCODE_SHIFT))
		{
			float currentSpeed = DotProduct3D(GetActor()->m_velocity, actorFwd);
			float speedFactor = GetClamped(1.0f - (currentSpeed / MAXSKATESPEED), 0.1f, 1.0f);

			GetActor()->AddImpulse(actorFwd * SKATEPUSHFORCEVALUE * speedFactor);
			GetActor()->PlayAnimationByName("Skate", 10);
		}
		
		if (g_engine->m_input->WasKeyJustPressed(KEYCODE_SPACE))
		{
			if (GetActor()->m_velocity.z < 0.1f && GetActor()->m_velocity.z > -0.1f)
			{
				m_jumpTimer->Start();
			}
		}
		if (g_engine->m_input->IsKeyDown(KEYCODE_SPACE))
		{

		}
		if (g_engine->m_input->WasKeyJustReleased(KEYCODE_SPACE))
		{
			Actor* actor = GetActor();
			IntVec2 tileCoordsActorOccupies			= IntVec2(RoundDownToInt(actor->m_position.x),RoundDownToInt(actor->m_position.y));
			if (GetActor()->m_position.z <= m_map->GetTile(tileCoordsActorOccupies.x, tileCoordsActorOccupies.y)->m_height)
			{
				GetActor()->AddImpulse(up* SKATEMAXJUMPFORCE* (m_jumpTimer->GetElapsedFraction()));

				GetActor()->PlayAnimationByName("Jump", 10);

				for (int index = 0; index < m_map->m_actors.size(); ++index)
				{
					if (m_map->m_actors[index] != nullptr && m_map->m_actors[index]->m_definition->m_name == "Cacodemon")
					{
						m_map->m_actors[index]->PlayAnimationByName("Pog", 10);
					}
				}
			}
		}

		float speed = GetActor()->m_velocity.GetLength();

		if (g_engine->m_input->IsKeyDown('A'))		// translate positive J							
		{
			GetActor()->m_orientation.m_yawDegrees += SKATERTURNSPEEDVALUE * (float)m_map->m_game->m_gameClock->GetDeltaSeconds();
		}

		if (g_engine->m_input->IsKeyDown('D'))		// translate negative J
		{
			GetActor()->m_orientation.m_yawDegrees -= SKATERTURNSPEEDVALUE * (float)m_map->m_game->m_gameClock->GetDeltaSeconds();
		}
		if (speed > 0.5f)
		{
			actorFwd = GetActor()->m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetNormalized();
			Vec3 targetVelocity = actorFwd * speed;

			GetActor()->m_velocity = Vec3::Interpolate(GetActor()->m_velocity, targetVelocity, (float)m_map->m_game->m_gameClock->GetDeltaSeconds() * SKATETURNGRIPSTRENGTH);
		}
		// 			if (g_engine->m_input->IsKeyDown('W'))		// translate positive I
		// 			{
		// 				GetActor()->MoveInDirection(forward, 1.f );
		// 			}
		// 
		// 			if (g_engine->m_input->IsKeyDown('S'))		// translate negative I
		// 			{
		// 				GetActor()->MoveInDirection(-forward, speed);
		// 			}
		
	}

	///////////////////////////////////////////////////////////////
	if (!m_isKeyboardPlayer)		// controller controls
	{
		if (g_engine->m_input->GetController(0).GetRightTrigger() > 0.1)
		{
			if (GetActor()->m_currentWeapon != nullptr)
			{
				GetActor()->m_currentWeapon->Fire();
			}
		}
		if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::X))
		{
			if (GetActor()->m_inventory.size() > 1)
			{
				GetActor()->EquipWeapon(0);
			}
		}
		if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::Y))
		{
			if (GetActor()->m_inventory.size() > 1)
			{
				GetActor()->EquipWeapon(1);
			}
		}

		if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::D_Pad_Down))
		{
			int currentIndex = 0;
			for (int invIndex = 0; invIndex < GetActor()->m_inventory.size(); ++invIndex)
			{
				if (GetActor()->m_inventory[invIndex]->m_name == GetActor()->m_currentWeapon->m_name)
				{
					currentIndex = invIndex;
				}
			}
			int newIndex = (currentIndex - 1) % GetActor()->m_inventory.size();
			GetActor()->EquipWeapon(newIndex);
		}
		if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::D_Pad_Up))
		{
			int currentIndex = 0;
			for (int invIndex = 0; invIndex < GetActor()->m_inventory.size(); ++invIndex)
			{
				if (GetActor()->m_inventory[invIndex]->m_name == GetActor()->m_currentWeapon->m_name)
				{
					currentIndex = invIndex;
				}
			}
			int newIndex = (currentIndex + 1) % GetActor()->m_inventory.size();
			GetActor()->EquipWeapon(newIndex);
		}
		///// MOVEMENT CONTROLS //////
		float speed = 0.f;
		if ((g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::A)))	// Increase speed by a factor of 15 if held
		{
			speed = GetActor()->m_definition->m_runSpeed;
		}
		else
		{
			speed = GetActor()->m_definition->m_walkSpeed;
		}

		Vec3 forward = GetActor()->m_orientation.GetForwardDir_IFwd_JLeft_KUp();
		forward.z = 0.f;
		forward = forward.GetNormalized();
		Vec3 left = forward.GetRotatedAboutZDegrees(90.f);
		float speedMagnitude = 0;

		float rightStickMagnitude = controller.GetRightStick().GetMagnitude();
		if (rightStickMagnitude > 0.f)
		{
			Vec2 direction = controller.GetRightStick().GetPosition();
			GetActor()->TurnInDirection(Vec3(direction.x, direction.y, direction.y), (float)m_map->m_game->m_gameClock->GetDeltaSeconds() * speedMagnitude);

			//Vec3 goalDirection = Vec3::MakeFromPolarDegrees(0.f, controller.GetRightStick().GetOrientationDegrees());
			//GetActor()->TurnInDirection(goalDirection, m_map->m_game->m_gameClock->GetDeltaSeconds() * speedMagnitude); 
		}

		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		if (leftStickMagnitude > 0.f)
		{
			Vec2 direction2D = controller.GetLeftStick().GetPosition();
			speedMagnitude = leftStickMagnitude;
			Vec3 direction = Vec3(direction2D.x, direction2D.y, 0.f);
			speed = speed * speedMagnitude;
			GetActor()->MoveInDirection(-direction, speed);
		}
	}

}

Mat44 PlayerController::GetModelToWorldTransform() const
{
	// make a translation matrix, make a rotation matrix, append rotation to translation
	Mat44 translationMatrix = Mat44();
	translationMatrix.AppendTranslation3D(m_position);
	Mat44 rotationMatrix = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	translationMatrix.Append(rotationMatrix);
	return translationMatrix;
}