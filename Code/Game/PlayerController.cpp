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
	if (!m_cameraMode)	// if in fps mode
	{
		FreeFlyInput();
	}
	else
	{
		ActorInput();
	}
}

void PlayerController::UpdateCamera()
{
	AABB2 screenBounds = AABB2(m_map->m_game->m_screenCamera->GetOrthoBottomLeft(), m_map->m_game->m_screenCamera->GetOrthoTopRight());

	float tempAspect = g_engine->m_window->m_config.m_clientAspect;
	if (GetActor()->m_definition->m_faction == "Demon")
	{
		m_camera->SetPerspectiveView(tempAspect, 100.f, 0.1f, 100.0f);
	}
	else
	{
		m_camera->SetPerspectiveView(tempAspect, 60.f, 0.1f, 100.0f);
	}

	if (m_cameraMode)
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
	else
	{
		m_camera->SetPosition(m_position);
		m_camera->SetOrientation(m_orientation);
	}
}

void PlayerController::ToggleCameraMode()
{
	m_cameraMode = !m_cameraMode;
	if (!m_cameraMode)
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

	if (g_engine->m_input->IsKeyDown('Z') || (g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::L_Bumper)) )		// translate positive K
	{
		m_position.z += speed * deltaSeconds;
	}

	if (g_engine->m_input->IsKeyDown('C') || (g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::R_Bumper)) )		// translate negative K
	{
		m_position.z -= speed * deltaSeconds;
	}
}

void PlayerController::ActorInput()
{
	const XboxController& controller = g_engine->m_input->GetController(0);
	Vec2 moveInputLeft = controller.GetLeftStick().GetPosition();
	Vec2 moveInputRight = controller.GetRightStick().GetPosition();

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


	///// WEAPON CONTROLS//////

	if (g_engine->m_input->IsKeyDown(KEYCODE_LEFT_MOUSE) || g_engine->m_input->GetController(0).GetRightTrigger() > 0.1)
	{
		if (GetActor()->m_currentWeapon != nullptr)
		{
			GetActor()->m_currentWeapon->Fire();	
		}
	}

	if (g_engine->m_input->WasKeyJustPressed('1') || g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::X))
	{
		if (GetActor()->m_inventory.size() > 1)
		{
			GetActor()->EquipWeapon(0);
		}
	}
	if (g_engine->m_input->WasKeyJustPressed('2') || g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::Y))
	{
		if (GetActor()->m_inventory.size() > 1)
		{
			GetActor()->EquipWeapon(1);
		}
	}
	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_LEFTARROW) || g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::D_Pad_Down))
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
	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_RIGHTARROW) || g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::D_Pad_Up))
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
	if (g_engine->m_input->IsKeyDown(KEYCODE_SHIFT) || (g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::A)))	// Increase speed by a factor of 15 if held
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

Mat44 PlayerController::GetModelToWorldTransform() const
{
	// make a translation matrix, make a rotation matrix, append rotation to translation
	Mat44 translationMatrix = Mat44();
	translationMatrix.AppendTranslation3D(m_position);
	Mat44 rotationMatrix = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	translationMatrix.Append(rotationMatrix);
	return translationMatrix;
}