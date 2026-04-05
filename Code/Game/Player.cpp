#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Input/InputSystem.hpp"

Player::Player(Game* owner)
	:m_game(owner)
{
}

Player::~Player()
{

}

void Player::Update([[maybe_unused]]float deltaSeconds)
{
	UpdateFromInput(deltaSeconds);
}

void Player::Render() const
{
	
}

void Player::DebugRender() const
{
	
}

bool Player::UpdateFromInput(float deltaSeconds)
{
	if (g_engine->m_input->m_cursorState.m_cursorClientDelta != Vec2(0.f, 0.f))
	{
		m_orientation.m_yawDegrees = m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetOrientationAboutZDegrees() + (g_engine->m_input->m_cursorState.m_cursorClientDelta.x * 0.075f);
		m_orientation.m_pitchDegrees = m_orientation.m_pitchDegrees - (g_engine->m_input->m_cursorState.m_cursorClientDelta.y * 0.075f);
		
		if (m_orientation.m_pitchDegrees > 90.f)  m_orientation.m_pitchDegrees = 89.9f;
		if (m_orientation.m_pitchDegrees < -90.f) m_orientation.m_pitchDegrees = -89.9f;
	}
	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F1))	// change controlling actor to projectile actor
	{
		m_controlProjectile = !m_controlProjectile;
	}

	if (!m_controlProjectile)		// remove after testing 
	{
		while (g_engine->m_input->IsKeyDown('Q'))		// Roll positive
		{
			m_orientation.m_rollDegrees += m_position.GetOrientationAboutZDegrees() * m_rotationSpeed * deltaSeconds;	//ToDo 
			
			break;
		}
																															// Per butler: update the 'props' euler angles and make a new matrix for it each frame then set that as its m_modeltoworldtransform
		while (g_engine->m_input->IsKeyDown('E'))		// Roll negative
		{
			m_orientation.m_rollDegrees -= m_position.GetOrientationAboutZDegrees() * m_rotationSpeed * deltaSeconds;	//ToDo
			break;
		}

		while (g_engine->m_input->IsKeyDown('A'))		// translate positive J												// why does changing the pitch of the camera change the k value when moving left and right???
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

		while (g_engine->m_input->IsKeyDown('Z'))		// translate positive K
		{
			m_position.z += 1 * m_moveSpeed * deltaSeconds;
			break;
		}

		while (g_engine->m_input->IsKeyDown('C'))		// translate negative K
		{
			m_position.z -= 1 * m_moveSpeed * deltaSeconds;
			break;
		}

		if (g_engine->m_input->IsKeyDown('H'))		// Reset position and orientation
		{
			m_position = Vec3(0,0,0);
			m_orientation.m_pitchDegrees = 0.f;
			m_orientation.m_rollDegrees = 0.f;
			m_orientation.m_yawDegrees = 0.f;
		}

		if (g_engine->m_input->IsKeyDown(KEYCODE_SHIFT))	// Increase speed by a factor of 15 while held
		{
			m_moveSpeed = 15.f;
		}
		else
		{
			m_moveSpeed = 1.f;
		}
		if (g_engine->m_input->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))	// Initiate raycast of 10 units
		{

		}
		if (g_engine->m_input->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))	// Initiate raycast of 0.25 units
		{

		}

	}
	else if (m_controlProjectile)
	{
		while (g_engine->m_input->IsKeyDown('A'))		// translate positive J												// why does changing the pitch of the camera change the k value when moving left and right???
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
			m_testProjectile->m_position += m_orientation.GetForwardDir_IFwd_JLeft_KUp() * m_moveSpeed * deltaSeconds;
			break;
		}

		while (g_engine->m_input->IsKeyDown('S'))		// translate negative I
		{
			m_testProjectile->m_position -= m_orientation.GetForwardDir_IFwd_JLeft_KUp() * m_moveSpeed * deltaSeconds;
			break;
		}
	}

	return 1;
}

Mat44 Player::GetModelToWorldTransform() const
{
	// make a translation matrix, make a rotation matrix, append rotation to translation
	Mat44 translationMatrix = Mat44();
	translationMatrix.AppendTranslation3D(m_position);
	Mat44 rotationMatrix = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	translationMatrix.Append(rotationMatrix);
	return translationMatrix;
}

