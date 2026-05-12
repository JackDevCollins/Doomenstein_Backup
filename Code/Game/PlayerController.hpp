#pragma once
#include "Game/Controller.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Game/GameCommon.hpp"

class Camera;
struct Mat44;
class Timer;

class PlayerController : public Controller 
{
public:
	PlayerController();
	virtual ~PlayerController();


	Camera*			m_camera	= nullptr;
	Vec3			m_position;
	EulerAngles		m_orientation;
	CameraMode		m_cameraMode = FPS;		// true = fps // false = free-fly //
	int				m_playerNum = 0;
	bool			m_isKeyboardPlayer = true;
	Timer*			m_skateTimer = nullptr;
	Timer*			m_jumpTimer = nullptr;

/*	void		Possess(ActorHandle HauntingActor) override;*/

	void		UpdateInput();
	void		UpdateCamera();

	void		ChangeCameraMode( CameraMode cameramode);
	void		PossessNextActor();

	void		FreeFlyInput();
	void		ActorInput();
	void		SkaterInput();

	Mat44		GetModelToWorldTransform() const;
};