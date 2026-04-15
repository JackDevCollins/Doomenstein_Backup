#pragma once
#include "Game/Controller.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Camera;
struct Mat44;

class PlayerController : public Controller 
{
public:
	
	Camera*			m_camera	= nullptr;
	Vec3			m_position;
	EulerAngles		m_orientation;
	bool			m_cameraMode = false;	// true = fps // false = free-fly //

	void		UpdateInput();
	void		UpdateCamera();

	bool		FreeFlyInput(float deltaSeconds);

	Mat44		GetModelToWorldTransform() const;


	float		m_rotationSpeed = 90.f;
	float		m_moveSpeed = 10.f;
	
};