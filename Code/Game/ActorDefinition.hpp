#pragma once
#include <string>
#include <vector>
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


	std::string						m_name;
	std::string						m_faction;
	float							m_health;
	bool							m_canBePossessed;
	float							m_corpseLifetime;
	bool							m_isVisible;

	float							m_collision_radius;
	float							m_height;
	bool							m_collidesWithWorld;
	bool							m_collidesWithActors;

	// projectile only
	FloatRange						m_damageOnCollide;
	float							m_impulseOnCollide;
	bool							m_dieOnCollide;

	bool							m_physicsSimulated;
	float							m_walkSpeed;
	float							m_runSpeed;
	float							m_turnSpeed;
	float							m_drag;

	float							m_cameraEyeHeight;
	float							m_cameraFOV;

	Vec2							m_visualsSize;
	Vec2							m_pivot;
	std::string						m_billboardType;
	bool							m_renderLit;
	bool							m_renderRounded;
	std::string						m_shader;
	std::string						m_spriteSheet;
	Vec2							m_cellCount;

	bool							m_aiEnabled;
	float							m_sightRadius;
	float							m_sightAngle;

	Strings							m_sounds;
	Strings							m_inventory;

	std::vector<animationGroup>		m_animationGroups;

};