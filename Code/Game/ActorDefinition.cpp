#include "Game/ActorDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void ActorDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument document;
	XmlResult result = document.LoadFile(path);
	if (result != 0)
	{
		ERROR_AND_DIE("XML LOADING ERROR")
	}
	XmlElement* rootElement = document.RootElement();
	XmlElement* actorDefinitionElement = rootElement->FirstChildElement();
	while (actorDefinitionElement != nullptr)
	{
		std::string elementName = actorDefinitionElement->Name();
		ActorDefinition* actorDefinition = new ActorDefinition();
		actorDefinition->LoadFromXmlElement(*actorDefinitionElement);
		s_definitions.push_back(actorDefinition);
		actorDefinitionElement = actorDefinitionElement->NextSiblingElement();
	}
	
}

void ActorDefinition::ClearDefinitions()
{
	s_definitions.clear();
}

const ActorDefinition* ActorDefinition::GetByName(const std::string& name)
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

std::vector<ActorDefinition*> ActorDefinition::s_definitions;

bool ActorDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name					= ParseXmlAttribute(element, "name", m_name);
	m_faction				= ParseXmlAttribute(element, "faction", m_faction);
	m_health				= ParseXmlAttribute(element, "health", m_health);
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
			m_physicsSimulated		= ParseXmlAttribute(*childElement, "simulated", m_physicsSimulated);;
			m_walkSpeed				= ParseXmlAttribute(*childElement, "walkSpeed", m_walkSpeed);
			m_runSpeed				= ParseXmlAttribute(*childElement, "runSpeed", m_runSpeed);
			m_turnSpeed				= ParseXmlAttribute(*childElement, "turnSpeed", m_turnSpeed);
			m_drag					= ParseXmlAttribute(*childElement, "drag", m_drag);
		}
		else if (childName == "Camera")
		{
			m_cameraEyeHeight		= ParseXmlAttribute(*childElement, "eyeHeight", m_cameraEyeHeight);
			m_cameraFOV				= ParseXmlAttribute(*childElement, "cameraFOV", m_cameraFOV);
		}

		else if (childName == "Visuals")
		{
			m_visualsSize			= ParseXmlAttribute(*childElement, "size", m_visualsSize);
			m_pivot					= ParseXmlAttribute(*childElement, "pivot", m_pivot);
			m_billboardType			= ParseXmlAttribute(*childElement, "billboardType", m_billboardType);
			m_renderLit				= ParseXmlAttribute(*childElement, "renderLit", m_renderLit);
			m_renderRounded			= ParseXmlAttribute(*childElement, "renderRounded", m_renderRounded);
			m_shader				= ParseXmlAttribute(*childElement, "shader", m_shader);
			m_spriteSheet			= ParseXmlAttribute(*childElement, "spriteSheet", m_spriteSheet);
			m_cellCount				= ParseXmlAttribute(*childElement, "cellCount", m_cellCount);

			const XmlElement* grandChildElement = childElement->FirstChildElement();

			while (grandChildElement != nullptr)
			{
				std::string grandChildName = grandChildElement->Name();

				if (grandChildName == "AnimationGroup")
				{
					animationGroup newAnimationGroup;

					newAnimationGroup.m_name			= ParseXmlAttribute(*grandChildElement, "name", "invalid");
					newAnimationGroup.m_scaleBySpeed	= ParseXmlAttribute(*grandChildElement, "scaleBySpeed", false);
					newAnimationGroup.m_secondsPerFrame = ParseXmlAttribute(*grandChildElement, "secondsPerFrame", 0.00f);
					newAnimationGroup.m_playbackMode	= ParseXmlAttribute(*grandChildElement, "playbackMode", "Loop");

					const XmlElement* greatGrandChildElement = grandChildElement->FirstChildElement();

					while (greatGrandChildElement != nullptr)
					{
						std::string greatGrandChildName = greatGrandChildElement->Name();

						if (greatGrandChildName == "Direction")
						{
							direction newDirection;

							newDirection.m_vector = ParseXmlAttribute(*grandChildElement, "vector", Vec3(0.f,0.f,0.f));
							newDirection.m_startFrame = ParseXmlAttribute(*grandChildElement, "startFrame", 0);
							newDirection.m_endFrame = ParseXmlAttribute(*grandChildElement, "endFrame", 0);

							newAnimationGroup.m_directions.push_back(newDirection);

							greatGrandChildElement = greatGrandChildElement->NextSiblingElement();
						}
					}

					m_animationGroups.push_back(newAnimationGroup);

					grandChildElement = grandChildElement->NextSiblingElement();
				}
			}
		}

		else if (childName == "Sounds")
		{
			const XmlElement* grandChildElement = childElement->FirstChildElement();
			while (grandChildElement != nullptr)
			{
				std::string grandChildName = grandChildElement->Name();

				if (grandChildName == "Sound")
				{
					std::string sound = "invalid";
					std::string soundLocation = "invalid";
					sound = ParseXmlAttribute(*grandChildElement, "sound", sound);
					soundLocation = ParseXmlAttribute(*grandChildElement, "name", soundLocation);
					std::string soundEntry = sound + "+" + soundLocation;
					m_sounds.push_back(soundEntry);
					
					grandChildElement = grandChildElement->NextSiblingElement();
				}
			}
		}

		else if (childName == "Inventory")
		{
			const XmlElement* grandChildElement = childElement->FirstChildElement();
			while (grandChildElement != nullptr)
			{
				std::string grandChildName = grandChildElement->Name();

				if (grandChildName == "Weapon")
				{
					std::string weaponName = "invalid";
					weaponName = ParseXmlAttribute(*grandChildElement, "name", weaponName);
					m_inventory.push_back(weaponName);

					grandChildElement = grandChildElement->NextSiblingElement();
				}

			}
		}

		childElement = childElement->NextSiblingElement();
	}

	return true;
}