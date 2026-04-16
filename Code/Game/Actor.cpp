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

	if (m_definition->m_name == "SpawnPoint")
	{
		m_isStatic = true;
	}
	if (m_definition->m_name == "Marine")
	{
		m_color = Rgba8(73, 188, 13);
	}
	if (m_definition->m_name == "Demon")
	{
		m_color = Rgba8(255,105,180);
	}
	
	m_aiController = new AIController();

}	
Actor::~Actor()
{

}

void Actor::Update([[maybe_unused]]float deltaSeconds)		
{
	m_physicsCylinder.clear();
	AddVertsForMe();

	if (m_controller != nullptr)
	{
		static_cast<PlayerController*>(m_controller)->UpdateInput();
		static_cast<PlayerController*>(m_controller)->UpdateCamera();
	}
	else
	{
		m_aiController->Update();
	}

}

void Actor::Render() const
{
	if (m_controller == nullptr)
	{
		g_engine->m_render->BindTexture(nullptr);
		g_engine->m_render->BindShader(nullptr);
		g_engine->m_render->SetBlendMode(BlendMode::OPAQUE);
		g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
		g_engine->m_render->SetModelConstants(Mat44(), m_color);
		g_engine->m_render->DrawVertexArray(m_physicsCylinder);

		g_engine->m_render->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);;
		g_engine->m_render->SetModelConstants(Mat44(), Rgba8(m_color.r + 10.f, m_color.g + 10.f, m_color.b + 10.f)); //Rgba8((m_color.r / (unsigned char)1.5), (m_color.g / (unsigned char)1.5), (m_color.b / (unsigned char)1.5)));
		g_engine->m_render->DrawVertexArray(m_physicsCylinder);

	}
}

void Actor::AddVertsForMe()
{
	m_orientation.m_pitchDegrees = 90.f;
	//AddVertsForCylinder3D(m_physicsCylinder, m_position, Vec3(m_position.x , m_position.y, m_position.z + m_physicsHeight), m_physicsRadius, m_color);
	AddVertsForZCylinder3D(m_physicsCylinder, m_position, m_physicsHeight, m_physicsRadius, m_color);
}

void Actor::Attack()
{	
	m_currentWeapon->Fire();
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
		m_velocity += m_definition->m_drag * (-m_velocity);

		//integrate acceleration
		// integrate velocity
		// integrate position


	}
}

void Actor::Damage(Actor* damager)
{
	m_health -= damager->m_owner->m_currentWeapon->m_definition->m_rayDamage;
	m_aiController->DamagedBy(damager->m_owner);
}

void Actor::AddForce()
{

}

void Actor::AddImpulse()
{

}

void Actor::OnCollide()
{

}

void Actor::OnPossessed()
{
	for (int index = 0; index < m_map->m_actors.size(); ++index)
	{
		Actor* testActor = m_map->m_actors[index];

		if (testActor->m_handle != this->m_handle)
		{
			if (testActor->m_controller != nullptr)
			{
				testActor->OnUnPossessed();
			}
		}
	}
	m_game->m_player->m_cameraMode = true;
}

void Actor::OnUnPossessed()
{
	m_controller = nullptr;

	m_aiController->Possess(m_handle);
}

void Actor::MoveInDirection()
{

}

void Actor::TurnInDirection()
{

}
