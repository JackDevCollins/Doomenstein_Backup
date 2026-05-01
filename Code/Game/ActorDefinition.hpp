#pragma once
#include <string>
#include <vector>
#include "Game/SpriteAnimationGroupDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"

class ActorDefinition
{
public:

	static void InitializeDefinitions(const char* path);
	static void ClearDefinitions();
	static const ActorDefinition* GetByName(const std::string& name);

	static std::vector<ActorDefinition*> s_definitions;

	bool	LoadFromXmlElement(const XmlElement& element);


	std::string						m_name					= "invalid";
	std::string						m_faction				= "neutral";
	float							m_health				= 1.f;
	bool							m_canBePossessed		= false;
	float							m_corpseLifetime		= 0.f;
	bool							m_isVisible				= false;

	float							m_collision_radius		= 0.f;
	float							m_height				= 0.f;
	bool							m_collidesWithWorld		= false;
	bool							m_collidesWithActors	= false;

	// projectile only
	FloatRange						m_damageOnCollide		= FloatRange(0,0);
	float							m_impulseOnCollide		= 0.f;
	bool							m_dieOnCollide			= false;

	bool							m_physicsSimulated		= false;
	float							m_walkSpeed				= 0.f;
	float							m_runSpeed				= 0.f;
	float							m_turnSpeed				= 0.f;
	float							m_drag					= 0.f;
	bool							m_isFlying				= false;

	float							m_cameraEyeHeight		= 0.f;
	float							m_cameraFOV				= 0.f;

	Vec2							m_visualsSize			= Vec2 (0,0);
	Vec2							m_pivot					= Vec2 (0,0);
	std::string						m_billboardType			= "invalid";
	bool							m_renderLit				= false;
	bool							m_renderRounded			= false;
	std::string						m_shader				= "invalid";
	std::string						m_spriteSheet			= "invalid";
	Vec2							m_cellCount				= Vec2 (0,0);

	bool							m_aiEnabled				= false;
	float							m_sightRadius			= 1.f;
	float							m_sightAngle			= 1.f;

	Strings							m_sounds;
	Strings							m_inventory;

	std::vector<SpriteAnimationGroupDefinition*>		m_animationGroups;

};