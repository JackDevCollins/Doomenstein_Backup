#include "Game/Weapon.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/PlayerController.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRender.hpp"


Weapon::Weapon(Actor* owner, const WeaponDefinition* weaponDefinition)
	:m_owner(owner),
	m_definition(weaponDefinition)
{
	m_name = weaponDefinition->m_name;
	m_weaponTimer = new Timer(weaponDefinition->m_refireTime, owner->m_game->m_gameClock);
}

void Weapon::Fire()
{
	if (m_weaponTimer->HasPeriodElapsed() || m_weaponTimer->IsStopped())
	{
		m_weaponTimer->Start();

		if (m_name == "Pistol")
		{	
			// replace getfwdIJK with origin to random point in cone normalized 
			
			RaycastResult3D bulletRaycast = m_owner->m_map->RaycastAll(
			Vec3(m_owner->m_position.x, m_owner->m_position.y, m_owner->m_definition->m_cameraEyeHeight * .75f ) + 
			(m_owner->m_map->m_game->m_player->m_camera->GetOrientation().GetForwardDir_IFwd_JLeft_KUp().GetNormalized() * (m_owner->m_physicsRadius + .01f)),
			m_owner->m_map->m_game->m_player->m_camera->GetOrientation().GetForwardDir_IFwd_JLeft_KUp(),
			m_definition->m_rayRange, m_owner);

			if (bulletRaycast.m_didImpact)
			{
				if (bulletRaycast.m_impactedObjectID == "Demon")
				{
					static_cast<Actor*>(bulletRaycast.m_pointerToImpactedObject)->Damage(m_owner, RollRandomFloatInRange(m_definition->m_rayDamage));
					static_cast<Actor*>(bulletRaycast.m_pointerToImpactedObject)->AddImpulse( - bulletRaycast.m_impactNormal * m_definition->m_rayImpulse);
				}
			}

			DebugAddWorldCylinder(bulletRaycast.m_rayStartPosition, (bulletRaycast.m_rayStartPosition + (bulletRaycast.m_rayDirection * bulletRaycast.m_rayLength)), 0.01f, 1.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
			if (bulletRaycast.m_didImpact)
			{
				DebugAddWorldSphere(bulletRaycast.m_impactPosition, 0.06f, 1.f);	// white sphere at impact pos
				DebugAddWorldArrow(bulletRaycast.m_impactPosition, bulletRaycast.m_impactPosition + (bulletRaycast.m_impactNormal.GetNormalized() * 0.3f), 0.03f, 1.f, Rgba8::BLUE, Rgba8::BLUE);
			}
		}

		if (m_name == "PlasmaRifle")
		{
			// replace getfwdIJK with origin to random point in cone normalized 
			
			SpawnInfo projectileSpawnInfo;
			projectileSpawnInfo.m_actorType = m_definition->m_projectileActor;
			projectileSpawnInfo.m_position = Vec3(m_owner->m_position.x, m_owner->m_position.y, m_owner->m_definition->m_cameraEyeHeight * .75f );
			//projectileSpawnInfo.m_orientation = m_owner->m_game->m_player->m_camera->GetOrientation();
			//projectileSpawnInfo.m_orientation = m_owner->m_orientation;
			projectileSpawnInfo.m_velocity = GetRandomDirectionInCone(*m_definition) * (m_owner->m_velocity.GetLength() + m_definition->m_projectileSpeed);

			Actor* projectile = m_owner->m_map->SpawnActor(projectileSpawnInfo);
			projectile->m_owner = m_owner;
			m_owner->m_map->AddActorToMap(projectile);
		}

		if (m_name == "DemonMelee")
		{
			Actor* closestEnemy = m_owner->m_map->GetClosestVisibleEnemy(m_owner->m_handle);

			if (closestEnemy == nullptr) return;

			Vec3 ownerFwd = m_owner->m_orientation.GetForwardDir_IFwd_JLeft_KUp();
			Vec2 fwd2dDirection =  Vec2(ownerFwd.x,ownerFwd.y);
			Vec2 radiusadjust = (fwd2dDirection.GetNormalized()) * m_owner->m_definition->m_collision_radius;
			Vec2 ownerPos2D = Vec2(m_owner->m_position.x, m_owner->m_position.y);
			Vec2 sectorOrigin = ownerPos2D + radiusadjust;

			if (IsPointInsideOrientedSector2D(Vec2(closestEnemy->m_position.x, closestEnemy->m_position.y), sectorOrigin, m_owner->m_orientation.m_yawDegrees, m_definition->m_meleeArc, m_definition->m_meleeRange))
			{
				closestEnemy->Damage(m_owner, RollRandomFloatInRange(m_definition->m_meleeDamage));
				Vec3 attackVector = Vec3(closestEnemy->m_position - m_owner->m_position).GetNormalized();
				closestEnemy->AddImpulse(attackVector * m_definition->m_meleeImpulse);
			}
		}
	}

	if(m_weaponTimer->IsStopped()) m_weaponTimer->Start();


}

Vec3 Weapon::GetRandomDirectionInCone(const WeaponDefinition definition) 
{
	float projectileCone = definition.m_projectileCone;
	float raycastCone =	definition.m_rayCone;

	EulerAngles aimDirection = static_cast<PlayerController*>(m_owner->m_controller)->m_camera->GetOrientation();

	if (projectileCone > 0)
	{
		aimDirection.m_pitchDegrees += RollRandomFloatInRange(-projectileCone, projectileCone);
		aimDirection.m_yawDegrees += RollRandomFloatInRange(-projectileCone, projectileCone);

	}

	if (raycastCone > 0)
	{
		aimDirection.m_pitchDegrees += RollRandomFloatInRange(-raycastCone, raycastCone);
		aimDirection.m_yawDegrees += RollRandomFloatInRange  (-raycastCone, raycastCone);

	}

	return aimDirection.GetForwardDir_IFwd_JLeft_KUp().GetNormalized();
}


