#pragma once
#include "Game/Controller.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Camera;

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