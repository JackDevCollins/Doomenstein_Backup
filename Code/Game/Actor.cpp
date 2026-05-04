#include "Game/Actor.hpp"
#include "Game/Weapon.hpp"
#include "Game/PlayerController.hpp"
#include "Game/AIController.hpp"
#include "Game/Controller.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Actor::Actor(Game* owner, float physicsHeight, float physicsRadius, Vec3 position, Rgba8 color, bool isStatic)
	:m_game(owner)
	,m_physicsHeight(physicsHeight)
	,m_physicsRadius(physicsRadius)
	,m_position(position)
	,m_color(color)
	,m_isStatic(isStatic)
{
	
	
}

Actor::Actor(Map* map, Game* game, const ActorDefinition* actorDef)
{
	m_definition	 = actorDef;
	m_game			 = game;
	m_map			 = map;
	m_corpseLifetime = actorDef->m_corpseLifetime;
	m_decomposeTimer = new Timer(m_corpseLifetime, m_game->m_gameClock);
	m_health		 = actorDef->m_health;
	m_color			 = Rgba8::WHITE;
	m_physicsHeight  = actorDef->m_height;
	m_physicsRadius  = actorDef->m_collision_radius;

	for (int inventoryIndex = 0; inventoryIndex < actorDef->m_inventory.size(); ++inventoryIndex)
	{
		std::string item = actorDef->m_inventory[inventoryIndex];
		Weapon* newItem = new Weapon(this, WeaponDefinition::GetByName(item));
		m_inventory.push_back(newItem);
	}
	if (!m_inventory.empty())
	{
		m_currentWeapon = m_inventory[0];
	}

	if (m_definition->m_name == "SpawnPoint")
	{
		m_isStatic = true;
	}
	if (m_definition->m_name == "Marine")
	{
		m_color = Rgba8(73, 188, 13);
		AddVertsForMe();
		m_animationClock = new Clock(*m_game->m_gameClock);
		//m_aiController = new AIController();
	}
	if (m_definition->m_name == "Demon")
	{
		m_color = Rgba8(255,105,180);
		AddVertsForMe();
		m_animationClock = new Clock(*m_game->m_gameClock);
		m_aiController = new AIController();
	}
	if (m_definition->m_name == "PlasmaProjectile")
	{
		m_color = Rgba8::BLUE;
		m_animationClock = new Clock(*m_game->m_gameClock);
		AddVertsForMe();
	}

	if (m_definition->m_shader != "invalid")
	{
		if (m_definition->m_renderLit)
		{
			m_shader = g_engine->m_render->CreateOrGetShader(m_definition->m_shader.c_str(), VertexType::VERTEX_PCUTBN);
		}
		else
		{
			m_shader = g_engine->m_render->CreateOrGetShader(m_definition->m_shader.c_str(), VertexType::VERTEX);
		}
	}

	m_vertexBuffer = g_engine->m_render->CreateVertexBuffer(1, sizeof(Vertex));
	m_indexBuffer = g_engine->m_render->CreateIndexBuffer(sizeof(unsigned int));
}	
Actor::~Actor()
{
	delete m_decomposeTimer;
	m_decomposeTimer = nullptr;

	for (Weapon* weapon : m_inventory) 
	{
		if (weapon != nullptr) 
		{
			delete weapon->m_animationClock;
			delete weapon;
		}
	}
	m_inventory.clear();

	if (m_aiController != nullptr)
	{
		delete m_aiController;
		m_aiController = nullptr;
	}
	if (m_animationClock != nullptr)
	{
		delete m_animationClock;
		m_animationClock = nullptr;
	}
	delete m_indexBuffer;
	delete m_vertexBuffer;
}

void Actor::Update([[maybe_unused]]float deltaSeconds)		
{
	if(m_definition->m_name == "SpawnPoint") return;

	else if (!m_isDead)
	{
		if (m_aiController)
		{
			m_aiController->Update(deltaSeconds);
		}
	}

	if (m_definition->m_dieOnSpawn)
	{
		m_health = 0;
	}
// update animation //
	UpdateAnimation(deltaSeconds);

	UpdatePhysics(deltaSeconds);


// update audio // 

	if (m_health <= 0)
	{	
		if (!m_isDead)
		{
			m_decomposeTimer->Start();
			if (m_animationClock == nullptr)
			{
				m_animationClock = new Clock(*m_game->m_gameClock);
				m_currentPlayingAnimationGroup = m_definition->m_animationGroups[0];
			}
			PlayAnimationByName("Death", 10);
			m_isDead = true;	
		}
	
		if (m_decomposeTimer->HasPeriodElapsed())
		{
			m_isGarbage = true;
		}
	}
// 	if (m_definition->m_dieOnCollide && m_isDead)
// 	{
// 		m_isGarbage = true;
// 	}
}

void Actor::Render() const
{
	for (PlayerController* player : m_game->m_players)
	{
		if (player == nullptr) return;
		if (!m_definition->m_isVisible) return;

		if (m_controller == player && player->m_cameraMode) return;

		Mat44 playerTransform = player->GetModelToWorldTransform();

		if (player->m_cameraMode)
		{
			playerTransform = player->GetActor()->GetModelToWorldTransform();
		}

		Mat44 billboardMatrix;
		if (m_definition->m_billboardType == "WorldUpFacing") billboardMatrix = GetBillboardTransform(BillboardType::WORLD_UP_FACING, playerTransform, m_position);			// for demons and marines
		if (m_definition->m_billboardType == "WorldUpOpposing") billboardMatrix = GetBillboardTransform(BillboardType::WORLD_UP_OPPOSING, playerTransform, m_position);		// for hit effects
		if (m_definition->m_billboardType == "None") billboardMatrix = GetBillboardTransform(BillboardType::NONE, playerTransform, m_position);						// screenspace like HUD and weapons
 		if (m_definition->m_billboardType == "FullOpposing") billboardMatrix = GetBillboardTransform(BillboardType::FULL_OPPOSING, playerTransform, m_position);	// for plasma projectile

		std::vector<Vertex> spriteQuadVerts;
		std::vector<unsigned int> spriteIndexes;

		//Translate by the inverse pivot times the size to center the quad on its local origin.
		AABB2 actorBounds = AABB2(Vec2::ZERO,m_definition->m_visualsSize);
		actorBounds.Translate(-m_definition->m_pivot * m_definition->m_visualsSize);
		Vec3 topLeft = Vec3(0.f, actorBounds.m_mins.x, actorBounds.m_maxs.y);
		Vec3 topRight = Vec3(0.f, actorBounds.m_maxs.x, actorBounds.m_maxs.y);
		Vec3 bottomLeft = Vec3(0.f, actorBounds.m_mins.x, actorBounds.m_mins.y);
		Vec3 bottomRight = Vec3(0.f, actorBounds.m_maxs.x, actorBounds.m_mins.y);
										
		// get sprite based on direction
		//camera to actor get vector quantity. run that through the actor's world transform inverse transform, that is the direction you plug into the get anim for direction funciton.
		Vec3 directionFromCameraToActor = m_position - player->m_camera->GetPosition();
		Vec3 NormalizedCamtoActDirection = directionFromCameraToActor.GetNormalized();

		Mat44 actorSelfWorldTransform;
		if (m_definition->m_name == "PlasmaProjectile")
		{
			actorSelfWorldTransform = GetModelToWorldTransform();
		}
		else
		{
			actorSelfWorldTransform = GetModelToWorldTransformYawOnly();
		}
		actorSelfWorldTransform = actorSelfWorldTransform.GetOrthonormalInverse();
		Vec3 directionToPlugIntoAnimationDirectionFunction = actorSelfWorldTransform.TransformVectorQuantity3D(NormalizedCamtoActDirection);

		const SpriteAnimDefinition& spriteDirectionDef = m_currentPlayingAnimationGroup->GetAnimationForDirection(directionToPlugIntoAnimationDirectionFunction);
		SpriteDefinition const& spriteAtTimeInDirection = spriteDirectionDef.GetSpriteDefAtTime((float)m_animationClock->GetTotalSeconds());
		AABB2 UVs = spriteAtTimeInDirection.GetUVs();
		
		if (m_definition->m_renderRounded)
		{
			AddVertsForRoundedQuad3D(spriteQuadVerts, spriteIndexes, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE, UVs);
			
		}
		else
		{
			//AddVertsForQuad3D(spriteQuadVerts, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, UVs);
			AddVertsForQuad3D(spriteQuadVerts,    topLeft,bottomLeft,bottomRight,topRight, Rgba8::WHITE, UVs);
		}


		Rgba8 actorColor = m_color;

		if (m_isDead)
		{
			actorColor.ScaleColor(.4f);
		}

		if (m_definition->m_name == "BloodSplatter" || m_definition->m_name == "BulletHit")
		{
			g_engine->m_render->BindTexture(m_currentPlayingAnimationGroup->m_spriteSheet->GetTexture());
			g_engine->m_render->BindShader(g_engine->m_render->CreateOrGetShader(m_definition->m_shader.c_str()));
			g_engine->m_render->SetBlendMode(BlendMode::ALPHA);
			g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
			g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

			g_engine->m_render->SetModelConstants(billboardMatrix);

			g_engine->m_render->CopyCPUToGPU(spriteQuadVerts.data(), (const unsigned int)spriteQuadVerts.size() * sizeof(Vertex), m_vertexBuffer);
			g_engine->m_render->CopyCPUToGPU(spriteIndexes.data(), (const unsigned int)spriteIndexes.size() * sizeof(unsigned int), m_indexBuffer);

			g_engine->m_render->DrawVertexBuffer(m_vertexBuffer, m_vertexBuffer->GetCount());

		}


		if (m_definition->m_name == "PlasmaProjectile")
		{
			//g_engine->m_render->BindTexture(g_engine->m_render->CreateOrGetTextureFromFile("Data/Images/Projectile_BloodSplatter.png"))
			g_engine->m_render->BindTexture(m_currentPlayingAnimationGroup->m_spriteSheet->GetTexture());
			g_engine->m_render->BindShader(m_shader);
			g_engine->m_render->SetBlendMode(BlendMode::OPAQUE);
			g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
			g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

			g_engine->m_render->SetModelConstants(billboardMatrix);

			g_engine->m_render->CopyCPUToGPU(spriteQuadVerts.data(), (const unsigned int)spriteQuadVerts.size() * sizeof(Vertex), m_vertexBuffer);
			g_engine->m_render->CopyCPUToGPU(spriteIndexes.data(), (const unsigned int)spriteIndexes.size() * sizeof(unsigned int), m_indexBuffer);

			g_engine->m_render->DrawVertexBuffer(m_vertexBuffer, m_vertexBuffer->GetCount());
			// physics / hitbox debug
// 			g_engine->m_render->SetModelConstants(GetModelToWorldTransform(), actorColor);
// 			g_engine->m_render->DrawVertexArray(m_physicsCylinder);
// 			g_engine->m_render->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);
// 			g_engine->m_render->SetModelConstants(GetModelToWorldTransform(), actorColor.GetScaledColor(0.5f) ); 
// 			g_engine->m_render->DrawVertexArray(m_physicsCylinder);
		}

//			if (m_controller == nullptr || !m_game->m_player->m_cameraMode )
//			{
// 			g_engine->m_render->BindTexture(nullptr);
// 			g_engine->m_render->BindShader(nullptr);
// 			g_engine->m_render->SetBlendMode(BlendMode::OPAQUE);
// 			g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
// 			g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
// 
// 			g_engine->m_render->SetModelConstants(GetModelToWorldTransformYawOnly(), actorColor);
// 			g_engine->m_render->DrawVertexArray(m_physicsCylinder);
// 			
// 			g_engine->m_render->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);
// 			g_engine->m_render->SetModelConstants(GetModelToWorldTransformYawOnly(), actorColor.GetScaledColor(0.5f)); 
// 			g_engine->m_render->DrawVertexArray(m_physicsCylinder);
// 		}

		else
		{
			g_engine->m_render->BindTexture(m_currentPlayingAnimationGroup->m_spriteSheet->GetTexture());

			g_engine->m_render->BindShader(m_shader);
			g_engine->m_render->SetBlendMode(BlendMode::OPAQUE);
			g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
			g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
			
			g_engine->m_render->SetModelConstants(billboardMatrix);

			//g_engine->m_render->DrawVertexArray(m_physicsCylinder);

			g_engine->m_render->CopyCPUToGPU(spriteQuadVerts.data(), (const unsigned int)spriteQuadVerts.size() * sizeof(Vertex), m_vertexBuffer);
			g_engine->m_render->CopyCPUToGPU(spriteIndexes.data(), (const unsigned int)spriteIndexes.size() * sizeof(unsigned int), m_indexBuffer);

			g_engine->m_render->DrawIndexedVertexBuffer(m_vertexBuffer, m_indexBuffer, m_indexBuffer->GetCount());
		}
	}
}

void Actor::AddVertsForMe()
{
	if (m_definition->m_isVisible)
	{
		AddVertsForZCylinder3D(m_physicsCylinder, Vec3(0,0,0) , m_physicsHeight, m_physicsRadius, m_color);
		if (m_definition->m_name == "Marine" || m_definition->m_name == "Demon")	// change back after testing
		{
			Vec3 test = Vec3(0, 0, m_definition->m_cameraEyeHeight);
 			AddVertsForCone3D(m_physicsCylinder, test + (Vec3(1,0,0) * m_physicsRadius), test + (Vec3(1,0,0) * .5), .125); 
		}
	}
}

void Actor::Attack()
{	
	if (m_currentWeapon->Fire())
	{
		PlayAnimationByName("Attack", 2);
	}
}

void Actor::EquipWeapon(int weaponNumber)
{
	m_currentWeapon = m_inventory[weaponNumber];
}

void Actor::PlayAnimationByName(const char* animationName,int priority) // bite animation and hurt animation bugged? double play? 
{
	if (m_currentPlayingAnimationGroup->m_name != animationName)
	{
		if (priority < m_currentAnimationPriority && m_animationClock->GetTotalSeconds() < m_currentPlayingAnimationGroup->GetDuration())
		{
			return;
		}
		if (m_currentAnimationPriority > priority && m_currentPlayingAnimationGroup->GetDuration() > m_animationClock->GetTotalSeconds())
		{
			return;
		}
			for (int index = 0; index < m_definition->m_animationGroups.size(); ++index)
			{
				if (m_definition->m_animationGroups[index]->m_name == animationName)
				{
					m_currentPlayingAnimationGroup = m_definition->m_animationGroups[index];
					m_currentAnimationPriority = priority;
					m_animationClock->Reset();
					break;
				}
			}
	}
}

void Actor::UpdateAnimation([[maybe_unused]]float deltaSeconds)
{
	if (m_animationClock == nullptr) return;

	if (m_currentPlayingAnimationGroup == nullptr)
	{
		m_currentPlayingAnimationGroup = m_definition->m_animationGroups[0];
	}

 	if (m_animationClock->GetTotalSeconds() > m_currentPlayingAnimationGroup->GetDuration() && m_currentPlayingAnimationGroup->m_spriteAnimationDefinitions[0]->m_playbackMode != SpriteAnimPlaybackType::LOOP)
 	{
//  		if (m_queuedAnimation != nullptr)
//  		{
//  			PlayAnimationByName(m_queuedAnimation->m_name.c_str(), 1);
//  			m_queuedAnimation = nullptr;
//  		}
//  		else
//  		{
 			PlayAnimationByName("Walk", 0);
 		//}
 	}

	if (m_currentPlayingAnimationGroup->m_scaleBySpeed)
	{
		m_animationClock->SetTimeScale(m_velocity.GetLength() / m_definition->m_runSpeed);
	}
	else
	{
		m_animationClock->SetTimeScale(1.0);
	}

	if (!m_inventory.empty() && m_currentWeapon->m_currentPlayingAnimation != nullptr)
	{
		m_currentWeapon->Update();
	}

}

Mat44 Actor::GetModelToWorldTransform() const
{	
	// make a translation matrix, make a rotation matrix, append rotation to translation
	Mat44 translationMatrix = Mat44();
	translationMatrix.AppendTranslation3D(m_position);
	Mat44 rotationMatrix = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	translationMatrix.Append(rotationMatrix);
	return translationMatrix;
}

Mat44 Actor::GetModelToWorldTransformYawOnly() const
{
	// make a translation matrix, make a rotation matrix, append rotation to translation
	Mat44 translationMatrix = Mat44();
	translationMatrix.AppendTranslation3D(m_position);
	EulerAngles yawOnlyOrientation = EulerAngles(m_orientation.m_yawDegrees, 0.f, 0.f);
	Mat44 rotationMatrix = yawOnlyOrientation.GetAsMatrix_IFwd_JLeft_KUp();
	translationMatrix.Append(rotationMatrix);
	return translationMatrix;
}

void Actor::UpdatePhysics(float deltaSeconds)
{
	if (m_definition->m_physicsSimulated)
	{
		AddForce(- m_velocity * m_definition->m_drag);

		if (!m_definition->m_isFlying)
		{
			m_velocity = Vec3(m_velocity.x,m_velocity.y,0.f);
		}
		m_velocity += (m_acceleration * deltaSeconds);
		if (m_velocity.GetLengthSquared() > .1f)
		{
			PlayAnimationByName("Walk", 0);
		}
		m_position += m_velocity * deltaSeconds;
		m_acceleration = Vec3(0,0,0);
	}
}

void Actor::Damage(Actor* damager, float damageAmount)
{
	m_health -= damageAmount;
	if (m_health > 0)
	{
		PlayAnimationByName("Hurt", 3);
	}

	if (m_definition->m_dieOnCollide) return;

	if (m_definition->m_aiEnabled)
	{
		m_aiController->DamagedBy(damager);
	} 
	else
	{
		static_cast<PlayerController*>(m_controller)->DamagedBy(damager);
	}
}

void Actor::AddForce(Vec3 force)
{
	m_acceleration += (force);
}

void Actor::AddImpulse(Vec3 impulse)
{
	m_velocity += impulse;
}

void Actor::OnCollide(ActorHandle collidedWith)
{
	Actor* hit = m_map->GetActorByHandle(collidedWith);

	if (hit != nullptr)
	{
		if (m_definition->m_damageOnCollide.m_min > 0.f)
		{
			hit->Damage(m_owner, RollRandomFloatInRange(m_definition->m_damageOnCollide));

			Vec3 impulseDirection = m_orientation.GetForwardDir_IFwd_JLeft_KUp();
			hit->AddImpulse(impulseDirection.GetNormalized() * m_definition->m_impulseOnCollide);

			m_health = 0.f;
		}	

		if (m_definition->m_canBePossessed && hit->m_definition->m_canBePossessed)
		{
			Vec2 actorAXYPosition = Vec2(m_position.x,m_position.y);
			Vec2 actorBXYPosition = Vec2(hit->m_position.x,hit->m_position.y);

			PushDiscOutOfEachOther2D(actorAXYPosition, m_physicsRadius, actorBXYPosition, hit->m_physicsRadius);
			m_position = Vec3(actorAXYPosition.x, actorAXYPosition.y, m_position.z);
			hit->m_position = Vec3(actorBXYPosition.x, actorBXYPosition.y, hit->m_position.z);
		}
	}
}

// Actor::OnCollide(ActorHandle collidedWith) 
// {
// 	Actor* hit = m_map->GetActorByHandle(collidedWith); 
// 	if (hit != nullptr) 
// 	{ 
// 		if (m_definition->m_damageOnCollide.m_min > 0) 
// 		{ 
// 			hit->Damage(m_owner, RollRandomFloatInRange (m_definition->m_damageOnCollide.m_min, m_definition->m_damageOnCollide.m_max)); 
// 		} 
// 		if (m_definition->m_impulseOnCollide > 0) 
// 		{ 
// 			Vec3 impulseDirection = m_orientation.GetForwardDir_IFwd_JLeft_KUp(); 
// 			hit->AddImpulse(impulseDirection.GetNormalized() * m_definition->m_impulseOnCollide); 
// 		} 
// 	} 
// 	if (m_definition->m_dieOnCollide) 
// 	{
// 		m_isDead = true; 
// 	} 
//}


void Actor::OnPossessed()
{
	for (PlayerController* player : m_game->m_players)
	{
		if (static_cast<PlayerController*>(m_controller) == player)
		{
			player->m_cameraMode = true;
			static_cast<PlayerController*>(m_controller)->m_position = m_position + Vec3(0.f, 0.f, m_definition->m_cameraEyeHeight);
			static_cast<PlayerController*>(m_controller)->m_orientation = m_orientation;
		}
	}
}

void Actor::OnUnPossessed()
{
	m_controller = nullptr;

	if (m_aiController)
	{
		m_aiController->Possess(m_handle);
	}
}

void Actor::MoveInDirection(Vec3 direction, float speed)
{
	float drag = m_definition->m_drag;

	AddForce(direction * speed * drag);
}

void Actor::TurnInDirection(Vec3 goal, float deltaSeconds)
{
	Vec2 actorToGoal = Vec2(goal.x - m_position.x, goal.y - m_position.y);
	float goalYaw = actorToGoal.GetOrientationDegrees();

	m_orientation.m_yawDegrees = GetTurnedTowardDegrees(m_orientation.m_yawDegrees, goalYaw, (m_definition->m_turnSpeed * deltaSeconds) );
	
}
