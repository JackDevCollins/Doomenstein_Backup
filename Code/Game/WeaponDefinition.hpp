#pragma once
#include <vector>
#include <string>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"

class WeaponDefinition
{
public:

	static void InitializeDefinitions(const char* path);
	static void ClearDefinitions();
	static const WeaponDefinition* GetByName(const std::string& name);

	static std::vector<WeaponDefinition*> s_definitions;

	bool	LoadFromXmlElement(const XmlElement& element);


	std::string		m_name;
	float			m_refireTime;
	int				m_rayCount;
	float			m_rayCone;
	float			m_rayRange;
	float			m_rayDamage;
	float			m_rayImpulse;
	std::string		m_HUDshader;
	std::string		m_baseTexture;
	std::string		m_reticleTexture;
	Vec2			m_reticleSize;
	Vec2			m_spriteSize;
	Vec2			m_spritePivot;

};