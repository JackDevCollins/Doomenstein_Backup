#include "Game/Player.hpp"


Player::Player(Game* owner)
	:Entity(owner)
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
		m_orientation.m_yawDegrees = m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetOrientationAboutZDegrees() + (g_engine->m_input->m_cursorState.m_cursorClientDelta.x * 0.125f);
		m_orientation.m_pitchDegrees = m_orientation.m_pitchDegrees - (g_engine->m_input->m_cursorState.m_cursorClientDelta.y * 0.125f);
		
	}

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
		//m_position.y += 1.f * m_moveSpeed * deltaSeconds;
		m_position += m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetRotatedAboutZDegrees(90).GetNormalized() * m_moveSpeed * deltaSeconds;
		break;
	}

	while (g_engine->m_input->IsKeyDown('D'))		// translate negative J
	{
		//m_position.y -= 1.f * m_moveSpeed * deltaSeconds;
		m_position -= m_orientation.GetForwardDir_IFwd_JLeft_KUp().GetRotatedAboutZDegrees(90).GetNormalized() * m_moveSpeed * deltaSeconds;
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

	if (g_engine->m_input->IsKeyDown(KEYCODE_SHIFT))	// Increase speed by a factor of 10 while held
	{
		m_moveSpeed = 10.f;
	}
	else
	{
		m_moveSpeed = 1.f;
	}
	return 1;
}
