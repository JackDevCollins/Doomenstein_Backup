#include "Game/ProjectileActorDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


void ProjectileActorDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument document;
	XmlResult result = document.LoadFile(path);
	if (result != 0)
	{
		ERROR_AND_DIE("XML LOADING ERROR")
	}
	XmlElement* rootElement = document.RootElement();
	XmlElement* projectileActorDefinitionElement = rootElement->FirstChildElement();
	while (projectileActorDefinitionElement != nullptr)
	{
		std::string elementName = projectileActorDefinitionElement->Name();
		ProjectileActorDefinition* projectileActorDefinition = new ProjectileActorDefinition();
		projectileActorDefinition->LoadFromXmlElement(*projectileActorDefinitionElement);
		s_definitions.push_back(projectileActorDefinition);
		projectileActorDefinitionElement = projectileActorDefinitionElement->NextSiblingElement();
	}
}

void ProjectileActorDefinition::ClearDefinitions()
{
	s_definitions.clear();
}

const ProjectileActorDefinition* ProjectileActorDefinition::GetByName(const std::string& name)
{
	for (int index = 0; index < s_definitions.size(); ++index)
	{
		if (s_definitions[index]->m_name == name)
		{
			return s_definitions[index];
		}
	}
	return nullptr;
}

std::vector<ProjectileActorDefinition*> ProjectileActorDefinition::s_definitions;

bool ProjectileActorDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name					= ParseXmlAttribute(element, "name", m_name);
	m_canBePossessed		= ParseXmlAttribute(element, "canBePossessed", m_canBePossessed);
	m_corpseLifetime		= ParseXmlAttribute(element, "corpseLifetime", m_corpseLifetime);
	m_isVisible				= ParseXmlAttribute(element, "visible", m_isVisible);

	const XmlElement* childElement = element.FirstChildElement();
	while (childElement != nullptr)
	{
		std::string childName = childElement->Name();

		if (childName == "Collision")
		{
			m_collision_radius		= ParseXmlAttribute(*childElement, "radius", m_collision_radius);
			m_height				= ParseXmlAttribute(*childElement, "height", m_height);
			m_collidesWithWorld		= ParseXmlAttribute(*childElement, "collidesWithWorld", m_collidesWithWorld);
			m_collidesWithActors	= ParseXmlAttribute(*childElement, "collidesWithActors", m_collidesWithActors);
			m_damageOnCollide		= ParseXmlAttribute(*childElement, "damageOnCollide", m_damageOnCollide);
			m_impulseOnCollide		= ParseXmlAttribute(*childElement, "impulseOnCollide", m_impulseOnCollide);
			m_dieOnCollide			= ParseXmlAttribute(*childElement, "dieOnCollide", m_dieOnCollide);
		}
		else if (childName == "Physics")
		{
			m_physicsSimulated = ParseXmlAttribute(*childElement, "simulated", m_physicsSimulated);
			m_turnSpeed = ParseXmlAttribute(*childElement, "turnSpeed", m_turnSpeed);
			m_flying	= ParseXmlAttribute(*childElement, "flying", m_flying);
			m_drag		= ParseXmlAttribute(*childElement, "drag", m_drag);
		}

		else if (childName == "Visuals")
		{
			m_visualsSize = ParseXmlAttribute(*childElement, "size", m_visualsSize);
			m_pivot = ParseXmlAttribute(*childElement, "pivot", m_pivot);
			m_billboardType = ParseXmlAttribute(*childElement, "billboardType", m_billboardType);
			m_renderLit = ParseXmlAttribute(*childElement, "renderLit", m_renderLit);
			m_renderRounded = ParseXmlAttribute(*childElement, "renderRounded", m_renderRounded);
			m_shader = ParseXmlAttribute(*childElement, "shader", m_shader);
			m_spriteSheet = ParseXmlAttribute(*childElement, "spriteSheet", m_spriteSheet);
			m_cellCount = ParseXmlAttribute(*childElement, "cellCount", m_cellCount);
		}

		else if (childName == "AnimationGroup")
		{

		}


		childElement = childElement->NextSiblingElement();
	}

	return true;
}