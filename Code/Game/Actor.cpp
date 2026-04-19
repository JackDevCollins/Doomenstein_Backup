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

Actor::Actor(Game* owner, float physicsHeight, float physicsRadius, Vec3 position, Rgba8 color, bool isStatic)
	:m_game(owner)
	,m_physicsHeight(physicsHeight)
	,m_physicsRadius(physicsRadius)
	,m_position(position)
	,m_color(color)
	,m_isStatic(isStatic)
{
	/*AddVertsForCylinder3D(m_physicsCylinder, m_position, m_position + Vec3(0, 0, physicsHeight), m_physicsRadius, m_color);*/
	
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
		//m_aiController = new AIController();
	}
	if (m_definition->m_name == "Demon")
	{
		m_color = Rgba8(255,105,180);
		AddVertsForMe();
		m_aiController = new AIController();
	}
	if (m_definition->m_name == "PlasmaProjectile")
	{
		m_color = Rgba8::BLUE;
		AddVertsForMe();
	}

}	
Actor::~Actor()
{
	
}

void Actor::Update([[maybe_unused]]float deltaSeconds)		
{
	if(m_definition->m_name == "SpawnPoint") return;

	if (m_controller != nullptr && static_cast<PlayerController*>(m_controller) == m_game->m_player && !m_isDead)
	{
		static_cast<PlayerController*>(m_controller)->UpdateInput();
	}

	else if (!m_isDead)
	{
		if (m_aiController)
		{
			m_aiController->Update(deltaSeconds);
		}
	}
	if (m_controller != nullptr && static_cast<PlayerController*>(m_controller) == m_game->m_player)
	{
		static_cast<PlayerController*>(m_controller)->UpdateCamera();
	}

	UpdatePhysics(deltaSeconds);

	if (m_health <= 0)
	{	
		if (!m_isDead)
		{
			m_decomposeTimer->Start();
			m_isDead = true;	
		}
	
		if (m_decomposeTimer->HasPeriodElapsed())
		{
			m_isGarbage = true;
		}
	}
}

void Actor::Render() const
{
	if(!m_definition->m_isVisible) return;

	Rgba8 actorColor = m_color;

	if (m_isDead)
	{
		actorColor.ScaleColor(.4f);
	}

 	if (m_controller == nullptr || !m_game->m_player->m_cameraMode)
 	{
		g_engine->m_render->BindTexture(nullptr);
		g_engine->m_render->BindShader(nullptr);
		g_engine->m_render->SetBlendMode(BlendMode::OPAQUE);
		g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);

		g_engine->m_render->SetModelConstants(GetModelToWorldTransformYawOnly(), actorColor);
		g_engine->m_render->DrawVertexArray(m_physicsCylinder);
		
		g_engine->m_render->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);;
		g_engine->m_render->SetModelConstants(GetModelToWorldTransformYawOnly(), Rgba8(actorColor.r + 10, actorColor.g + 10, actorColor.b + 10)); //Rgba8((m_color.r / (unsigned char)1.5), (m_color.g / (unsigned char)1.5), (m_color.b / (unsigned char)1.5)));
		g_engine->m_render->DrawVertexArray(m_physicsCylinder);
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
	m_currentWeapon->Fire();
}

void Actor::EquipWeapon(int weaponNumber)
{
	m_currentWeapon = m_inventory[weaponNumber];
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

void Actor::TestPojectileInput(Vec3 movement)
{
	m_position += movement * 100.;
}

void Actor::UpdatePhysics(float deltaSeconds)
{
	if (m_definition->m_physicsSimulated)
	{
		if (!m_isFlying)
		{
			m_velocity = Vec3(m_velocity.x,m_velocity.y,0.f);
		}

		AddForce(- m_velocity * m_definition->m_drag);

		m_velocity += (m_acceleration * deltaSeconds);
		m_position += m_velocity * deltaSeconds;
		m_acceleration = Vec3(0,0,0);
	}
}

void Actor::Damage(Actor* damager, float damageAmount)
{
	m_health -= damageAmount;

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

void Actor::OnCollide()
{

}

void Actor::OnPossessed()
{
	if (static_cast<PlayerController*>(m_controller) == m_game->m_player)
	{
		m_game->m_player->m_cameraMode = true;
		static_cast<PlayerController*>(m_controller)->m_position = m_position + Vec3(0.f, 0.f, m_definition->m_cameraEyeHeight);
		static_cast<PlayerController*>(m_controller)->m_orientation = m_orientation;
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
