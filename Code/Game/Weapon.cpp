#include "Game/Weapon.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/PlayerController.hpp"
#include "Game/SpriteAnimationGroupDefinition.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


Weapon::Weapon(Actor* owner, const WeaponDefinition* weaponDefinition)
	:m_owner(owner),
	m_definition(weaponDefinition)
{
	m_name = weaponDefinition->m_name;
	m_weaponTimer = new Timer(weaponDefinition->m_refireTime, owner->m_game->m_gameClock);
	if (!m_definition->m_animations.empty())
	{
		m_animationClock = new Clock(*m_owner->m_game->m_gameClock);
		m_currentPlayingAnimation = m_definition->m_animations[0];
		m_weaponSpriteSheet = m_definition->m_animations[0]->m_spriteSheet;
	}
}

bool Weapon::Fire()
{
	if (m_weaponTimer->HasPeriodElapsed() || m_weaponTimer->IsStopped())
	{
		m_weaponTimer->Start();
		if (m_currentPlayingAnimation!= nullptr && m_currentPlayingAnimation != m_definition->m_animations[1])
		{
			m_currentPlayingAnimation = m_definition->m_animations[1];
			m_animationClock->Reset();
		}
		if (m_name == "Pistol")
		{	
			// replace getfwdIJK with origin to random point in cone normalized 
			m_fireSound = g_engine->m_audio->CreateOrGetSound(m_definition->GetSoundByName("Fire").c_str());
			g_engine->m_audio->StartSound(m_fireSound);
			RaycastResult3D bulletRaycast = m_owner->m_map->RaycastAll(
			Vec3(m_owner->m_position.x, m_owner->m_position.y, m_owner->m_definition->m_cameraEyeHeight * .75f ) + 
			(static_cast<PlayerController*>(m_owner->m_controller)->m_camera->GetOrientation().GetForwardDir_IFwd_JLeft_KUp().GetNormalized() * (m_owner->m_physicsRadius + .01f)),
			static_cast<PlayerController*>(m_owner->m_controller)->m_camera->GetOrientation().GetForwardDir_IFwd_JLeft_KUp(),
			m_definition->m_rayRange, m_owner);

			if (bulletRaycast.m_didImpact)
			{
				if (bulletRaycast.m_impactedObjectID == "Demon")
				{
					static_cast<Actor*>(bulletRaycast.m_pointerToImpactedObject)->Damage(m_owner, RollRandomFloatInRange(m_definition->m_rayDamage));
					static_cast<Actor*>(bulletRaycast.m_pointerToImpactedObject)->AddImpulse( - bulletRaycast.m_impactNormal * m_definition->m_rayImpulse);
					SpawnInfo impactSpawninfo;
					impactSpawninfo.m_actorType = "BloodSplatter";
					impactSpawninfo.m_position = bulletRaycast.m_impactPosition;
// 					Actor* impactedDemon =  static_cast<Actor*>(bulletRaycast.m_pointerToImpactedObject);
// 					if (impactedDemon != nullptr)
// 					{
// 						impactSpawninfo.m_velocity = impactedDemon->m_velocity;
// 					}
					Actor* spawnedImpact = m_owner->m_map->SpawnActor(impactSpawninfo);
					m_owner->m_map->AddActorToMap(spawnedImpact);

				}
			}
// 			DebugAddWorldCylinder(bulletRaycast.m_rayStartPosition, (bulletRaycast.m_rayStartPosition + (bulletRaycast.m_rayDirection * bulletRaycast.m_rayLength)), 0.01f, 1.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
// 			if (bulletRaycast.m_didImpact)
// 			{
// 				DebugAddWorldSphere(bulletRaycast.m_impactPosition, 0.06f, 1.f);	// white sphere at impact pos
// 				DebugAddWorldArrow(bulletRaycast.m_impactPosition, bulletRaycast.m_impactPosition + (bulletRaycast.m_impactNormal.GetNormalized() * 0.3f), 0.03f, 1.f, Rgba8::BLUE, Rgba8::BLUE);
// 			}
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
			return true;
		}

		if (m_name == "DemonMelee")
		{
			Actor* closestEnemy = m_owner->m_map->GetClosestVisibleEnemy(m_owner->m_handle);

			if (closestEnemy == nullptr) return false;

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
			return true;
		}
	}
	if(m_weaponTimer->IsStopped()) m_weaponTimer->Start();

	return false;

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

void Weapon::Update()
{
	if (m_animationClock->GetTotalSeconds() > (m_definition->m_animations[1]->m_endSpriteIndex) * m_definition->m_animations[1]->m_secondsPerFrame && m_animationClock->GetTotalSeconds() != 0.0)
	{
		m_currentPlayingAnimation = m_definition->m_animations[0];
	}
}

