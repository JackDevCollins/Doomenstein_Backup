#pragma once
#include "Game/GameCommon.hpp"
#include <vector>
#include <string>
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/FloatRange.hpp"

class ActorDefinition;
class SpriteAnimDefinition;
class SpriteAnimationGroupDefinition;

class WeaponDefinition
{
public:

	static void  InitializeDefinitions(const char* path);
	static void  ClearDefinitions();
	static const WeaponDefinition* GetByName(const std::string& name);

	static std::vector<WeaponDefinition*> s_definitions;

	bool	LoadFromXmlElement(const XmlElement& element);


	std::string		m_name;
	float			m_refireTime;
	// projectile
	int							m_projectileCount;
	std::string					m_projectileActor;
	float						m_projectileCone;
	float						m_projectileSpeed;
	// hit-scan
	int							m_rayCount;
	float						m_rayCone;
	float						m_rayRange;
	FloatRange					m_rayDamage;
	float						m_rayImpulse;
	// melee
	int							m_meleeCount;
	float						m_meleeArc;
	float						m_meleeRange;
	FloatRange					m_meleeDamage;
	float						m_meleeImpulse;

	// visuals
	std::string					m_HUDshader;
	std::string					m_weaponShader;
	SpriteSheet*				m_weaponSpriteSheet = nullptr;
	std::string					m_baseTexture;
	std::string					m_reticleTexture;
	Vec2						m_reticleSize;
	Vec2						m_spriteSize;
	Vec2						m_spritePivot;

	//std::vector<SpriteAnimationGroupDefinition*>		m_animations;
	std::vector<SpriteAnimDefinition*>		m_animations;

	Strings						m_sounds;
};