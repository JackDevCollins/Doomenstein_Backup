#pragma once
#include "Game/Controller.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Camera;
struct Mat44;

class PlayerController : public Controller 
{
public:
	PlayerController();
	virtual ~PlayerController();


	Camera*			m_camera	= nullptr;
	Vec3			m_position;
	EulerAngles		m_orientation;
	bool			m_cameraMode = false;	// true = fps // false = free-fly //

/*	void		Possess(ActorHandle HauntingActor) override;*/

	void		UpdateInput();
	void		UpdateCamera();

	void		ToggleCameraMode();

	void		FreeFlyInput();
	void		ActorInput();

	Mat44		GetModelToWorldTransform() const;
};