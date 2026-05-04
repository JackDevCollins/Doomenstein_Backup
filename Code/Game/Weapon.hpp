#pragma once
#include <string>
#include "Engine/Math/Vec3.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/WeaponDefinition.hpp"

class WeaponDefinition;
class Timer;
class Actor;
class SpriteSheet;
class Shader;
class SpriteAnimDefinition;
class Clock;


class Weapon
{
public:

	Weapon(Actor* owner, const WeaponDefinition* weaponDefinition);
	
	bool Fire();
	Vec3 GetRandomDirectionInCone(const WeaponDefinition m_weaponDefinition);
	void Update();

	Actor*								m_owner;
	std::string							m_name;
	const WeaponDefinition*				m_definition = nullptr;
	SpriteSheet*						m_weaponSpriteSheet;
	Timer*								m_weaponTimer;
	Shader*								m_shader = nullptr;
	SpriteAnimDefinition*				m_currentPlayingAnimation = nullptr;
	Clock*								m_animationClock = nullptr;
	SoundID								m_fireSound;
};