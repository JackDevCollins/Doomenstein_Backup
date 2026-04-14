#include "Game/Weapon.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/Actor.hpp"
#include "Engine/Core/Timer.hpp"


Weapon::Weapon(Actor* owner, const WeaponDefinition* weaponDefinition)
	:m_owner(owner),
	m_definition(weaponDefinition)
{
	m_name = weaponDefinition->m_name;
	m_weaponTimer = new Timer(weaponDefinition->m_refireTime, owner->m_game->m_gameClock);
}

void Weapon::Fire()
{
	
}

// Vec3 Weapon::GetRandomDirectionInCone()
// {
// 
// }
