#pragma once
#include <vector>
#include <string>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"


class ProjectileActorDefinition
{
public:

	static void InitializeDefinitions(const char* path);
	static void ClearDefinitions();
	static const ProjectileActorDefinition* GetByName(const std::string& name);

	static std::vector<ProjectileActorDefinition*> s_definitions;

	bool	LoadFromXmlElement(const XmlElement& element);



	std::string		m_name;
	std::string		m_faction;
	float			m_health;
	bool			m_canBePossessed;
	float			m_corpseLifetime;
	bool			m_isVisible;
	float			m_collision_radius;
	float			m_height;
	bool			m_collidesWithWorld;
	bool			m_collidesWithActors;
	bool			m_physicsSimulated;
	float			m_walkSpeed;
	float			m_runSpeed;
	float			m_turnSpeed;
	float			m_drag;
	float			m_cameraEyeHeight;
	float			m_cameraFOV;
	Vec2			m_visualsSize;
	Vec2			m_pivot;
	BillboardType	m_billboardType;
	bool			m_renderLit;
	bool			m_renderRounded;
	std::string		m_shader;
	std::string		m_spriteSheet;
};