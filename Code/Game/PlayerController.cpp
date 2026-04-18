#include "Game/PlayerController.hpp"
#include "Game/Controller.hpp"
#include "Game/App.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Camera.hpp"

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
	if (m_camera == nullptr)
	{
		m_camera = m_map->m_playerCamera;
	}

	float tempAspect = g_engine->m_window->m_config.m_clientAspect;
	m_camera->SetPerspectiveView(tempAspect, 60.f, 0.1f, 100.0f);

	if (m_cameraMode)
	{
		if (!GetActor()->m_isDead)
		{
			Vec3 actorPosition = GetActor()->m_position;
			Vec3 CameraPosition = Vec3(actorPosition.x, actorPosition.y, GetActor()->m_definition->m_cameraEyeHeight);
			m_camera->SetPosition(CameraPosition);
			m_camera->SetOrientation(GetActor()->m_orientation);
			//m_camera->SetPositionAndOrientation(Newposition, EulerAngles(GetActor()->m_orientation.m_yawDegrees, GetActor()->m_orientation.m_pitchDegrees, 0));
			//m_camera->SetOrientation(EulerAngles(GetActor()->m_orientation.m_yawDegrees, m_orientation.m_pitchDegrees, 0));
		}
	}
}

bool PlayerController::FreeFlyInput()
{
	float deltaSeconds = (float)g_systemClock->GetDeltaSeconds();

	// get controller
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



	while (g_engine->m_input->IsKeyDown('A'))		// translate positive J							
	{
		Vec3 forward = m_orientation.GetForwardDir_IFwd_JLeft_KUp();
		forward.z = 0.f;
		forward.GetNormalized();
		m_position += forward.GetRotatedAboutZDegrees(90).GetNormalized() * m_moveSpeed * deltaSeconds;
		//m_position += m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetRotatedAboutZDegrees(90).GetNormalized() * m_moveSpeed * deltaSeconds;
		break;
	}

	while (g_engine->m_input->IsKeyDown('D'))		// translate negative J
	{
		Vec3 forward = m_orientation.GetForwardDir_IFwd_JLeft_KUp();
		forward.z = 0.f;
		forward.GetNormalized();
		m_position -= forward.GetRotatedAboutZDegrees(90).GetNormalized() * m_moveSpeed * deltaSeconds;
		//m_position -= m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetRotatedAboutZDegrees(90).GetNormalized() * m_moveSpeed * deltaSeconds;
		break;
	}

	while (g_engine->m_input->IsKeyDown('W'))		// translate positive I
	{
		m_position += m_orientation.GetForwardDir_IFwd_JLeft_KUp() * m_moveSpeed * deltaSeconds;
		break;
	}

	while (g_engine->m_input->IsKeyDown('S'))		// translate negative I
	{
		m_position -= m_orientation.GetForwardDir_IFwd_JLeft_KUp() * m_moveSpeed * deltaSeconds;
		break;
	}

	while (g_engine->m_input->IsKeyDown('Z') || (g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::L_Bumper)) )		// translate positive K
	{
		m_position.z += 1 * m_moveSpeed * deltaSeconds;
		break;
	}

	while (g_engine->m_input->IsKeyDown('C') || (g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::R_Bumper)) )		// translate negative K
	{
		m_position.z -= 1 * m_moveSpeed * deltaSeconds;
		break;
	}

	if (g_engine->m_input->IsKeyDown(KEYCODE_SHIFT) || (g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::A)) )	// Increase speed by a factor of 15 while held
	{
		m_moveSpeed = 15.f;
	}
	else
	{
		m_moveSpeed = 10.f;
	}
		
	

	return 1;
}

bool PlayerController::ActorInput()
{

	// get controller
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

	Vec3 forward = GetActor()->m_orientation.GetForwardDir_IFwd_JLeft_KUp();
	forward.z = 0.f;
	forward = forward.GetNormalized();
	Vec3 left = forward.GetRotatedAboutZDegrees(90.f);

	while (g_engine->m_input->IsKeyDown('A'))		// translate positive J							
	{
		GetActor()->AddForce(left);
		break;
	}

	while (g_engine->m_input->IsKeyDown('D'))		// translate negative J
	{
		GetActor()->AddForce(-left);
		break;
	}

	while (g_engine->m_input->IsKeyDown('W'))		// translate positive I
	{

		GetActor()->AddForce(GetActor()->m_orientation.GetForwardDir_KBwd_JLeft_Iup().GetNormalized());
		break;
	}

	while (g_engine->m_input->IsKeyDown('S'))		// translate negative I
	{
		GetActor()->AddForce(-GetActor()->m_orientation.GetForwardDir_KBwd_JLeft_Iup().GetNormalized());
		break;
	}

	if (g_engine->m_input->IsKeyDown(KEYCODE_SHIFT) || (g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::A)))	// Increase speed by a factor of 15 while held
	{
		m_moveSpeed = 15.f;
	}
	else
	{
		m_moveSpeed = 10.f;
	}

	return 1;
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