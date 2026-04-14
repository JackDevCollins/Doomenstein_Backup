#pragma once
#include <string>
#include "Engine/Math/Vec3.hpp"

class WeaponDefinition;
class Timer;
class Actor;

class Weapon
{
public:

	Weapon(Actor* owner, const WeaponDefinition* weaponDefinition);

	void Fire();
	Vec3 GetRandomDirectionInCone();

	Actor*						m_owner;
	std::string					m_name;
	const WeaponDefinition*		m_definition = nullptr;
	Timer*						m_weaponTimer;
};