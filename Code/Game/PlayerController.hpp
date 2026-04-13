#pragma once
#include "Game/Controller.hpp"


class PlayerController : public Controller 
{
public:
	
	Camera*			m_camera;
	Vec3			m_position;
	EulerAngles		m_orientation;
	bool			m_cameraMode;

	void		UpdateInput();
	void		UpdateCamera();
};