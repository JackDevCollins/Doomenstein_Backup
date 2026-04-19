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

			DebugAddWorldCylinder(bulletRaycast.m_rayStartPosition, (bulletRaycast.m_rayStartPosition + (bulletRaycast.m_rayDirection * bulletRaycast.m_rayLength)), 0.01f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
			if (bulletRaycast.m_didImpact)
			{
				DebugAddWorldSphere(bulletRaycast.m_impactPosition, 0.06f, 10.f);	// white sphere at impact pos
				DebugAddWorldArrow(bulletRaycast.m_impactPosition, bulletRaycast.m_impactPosition + (bulletRaycast.m_impactNormal.GetNormalized() * 0.3f), 0.03f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
			}
		}

		if (m_name == "PlasmaRifle")
		{
			// replace getfwdIJK with origin to random point in cone normalized 
			
			SpawnInfo projectileSpawnInfo;
			projectileSpawnInfo.m_actorType = m_definition->m_projectileActor;
			projectileSpawnInfo.m_position = Vec3(m_owner->m_position.x, m_owner->m_position.y, m_owner->m_definition->m_cameraEyeHeight * .75f );
			//projectileSpawnInfo.m_orientation = m_owner->m_game->m_player->m_camera->GetOrientation();
			projectileSpawnInfo.m_orientation = m_owner->m_orientation;
			projectileSpawnInfo.m_velocity = m_owner->m_orientation.GetForwardDir_IFwd_JLeft_KUp() * (m_owner->m_velocity.GetLength() + m_definition->m_projectileSpeed);

			Actor* projectile = m_owner->m_map->SpawnActor(projectileSpawnInfo);
			projectile->m_owner = m_owner;
			m_owner->m_map->AddActorToMap(projectile);
		}

		//RaycastResult3D result = m_game->m_currentMap->RaycastAll(m_position, m_orientation.GetForwardDir_IFwd_JLeft_KUp(), 10.f);
		// 			DebugAddWorldCylinder(result.m_rayStartPosition,(result.m_rayStartPosition + (result.m_rayDirection * result.m_rayLength)), 0.01f, 10.f, Rgba8::WHITE,Rgba8::WHITE, DebugRenderMode::X_RAY);
		// 			if (result.m_didImpact)
		// 			{
		// 				DebugAddWorldSphere(result.m_impactPosition, 0.06f, 10.f);	// white sphere at impact pos
		// 				DebugAddWorldArrow(result.m_impactPosition, result.m_impactPosition + (result.m_impactNormal.GetNormalized() * 0.3f), 0.03f, 10.f, Rgba8::BLUE, Rgba8::BLUE);
		// 			}
	}

	if(m_weaponTimer->IsStopped()) m_weaponTimer->Start();


}

// Vec3 Weapon::GetRandomDirectionInCone()
// {
// 
// }
