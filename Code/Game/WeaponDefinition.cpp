#include "Game/WeaponDefinition.hpp"
#include "Game/SpriteAnimationGroupDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Engine.hpp"

void WeaponDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument document;
	XmlResult result = document.LoadFile(path);
	if (result != 0)
	{
		ERROR_AND_DIE("XML LOADING ERROR")
	}
	XmlElement* rootElement = document.RootElement();
	XmlElement* weaponDefinitionElement = rootElement->FirstChildElement();

	while (weaponDefinitionElement != nullptr)
	{
		std::string elementName = weaponDefinitionElement->Name();
		WeaponDefinition* weaponDefinition = new WeaponDefinition();
		weaponDefinition->LoadFromXmlElement(*weaponDefinitionElement);
		s_definitions.push_back(weaponDefinition);
		weaponDefinitionElement = weaponDefinitionElement->NextSiblingElement();
	}
}

void WeaponDefinition::ClearDefinitions()
{
	s_definitions.clear();
}

const WeaponDefinition* WeaponDefinition::GetByName(const std::string& name)
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

std::vector<WeaponDefinition*> WeaponDefinition::s_definitions;

bool WeaponDefinition::LoadFromXmlElement(const XmlElement& element)
{

	m_name 				=	 ParseXmlAttribute(element, "name", m_name);
	m_refireTime		=	 ParseXmlAttribute(element, "refireTime", m_refireTime);
	m_projectileCount	=	 ParseXmlAttribute(element, "projectileCount", 0);
	m_projectileActor	=	 ParseXmlAttribute(element, "projectileActor", "Invalid");
	m_projectileCone	=	 ParseXmlAttribute(element, "projectileCone", 0.0f);
	m_projectileSpeed	=	 ParseXmlAttribute(element, "projectileSpeed", 0.0f);
	m_rayCount			=	 ParseXmlAttribute(element, "rayCount", m_rayCount);
	m_rayCone			=	 ParseXmlAttribute(element, "rayCone", m_rayCone);
	m_rayRange			=	 ParseXmlAttribute(element, "rayRange", m_rayRange);
	m_rayDamage			=	 ParseXmlAttribute(element, "rayDamage", m_rayDamage);
	m_rayImpulse		=	 ParseXmlAttribute(element, "rayImpulse", m_rayImpulse);

	m_meleeCount		=	 ParseXmlAttribute(element, "meleeCount", m_meleeCount);
	m_meleeArc			=	 ParseXmlAttribute(element, "meleeArc", m_meleeArc);
	m_meleeRange		=	 ParseXmlAttribute(element, "meleeRange", m_meleeRange);
	m_meleeDamage		=	 ParseXmlAttribute(element, "meleeDamage", m_meleeDamage);
	m_meleeImpulse		=	 ParseXmlAttribute(element, "meleeImpulse", m_meleeImpulse);

	const XmlElement* childElement = element.FirstChildElement();
	while (childElement != nullptr)
	{
		std::string childName = childElement->Name();

		if (childName == "HUD")
		{
			m_HUDshader			= ParseXmlAttribute(*childElement, "shader", m_HUDshader);
			m_baseTexture		= ParseXmlAttribute(*childElement, "baseTexture", m_baseTexture);
			m_reticleTexture	= ParseXmlAttribute(*childElement, "reticleTexture", m_reticleTexture);
			m_reticleSize		= ParseXmlAttribute(*childElement, "reticleSize", m_reticleSize);
			m_spriteSize		= ParseXmlAttribute(*childElement, "spriteSize", m_spriteSize);
			m_spritePivot		= ParseXmlAttribute(*childElement, "spritePivot", m_spritePivot);

			const XmlElement* grandChildElement = childElement->FirstChildElement();
			while (grandChildElement != nullptr)
			{
				std::string grandChildName = grandChildElement->Name();

				if (grandChildName == "Animation")
				{
					SpriteAnimDefinition* newSpriteAnim = new SpriteAnimDefinition;
					

					//newSpriteAnimationGroup->m_name				= ParseXmlAttribute(*grandChildElement, "name", "invalid");
					m_weaponShader									= ParseXmlAttribute(*grandChildElement, "shader", "invalid");
					if (m_weaponSpriteSheet == nullptr)
					{
						std::string spriteSheetPath					= ParseXmlAttribute(*grandChildElement, "spriteSheet", "invalid");
						Texture* spriteTexture						= g_engine->m_render->CreateOrGetTextureFromFile(spriteSheetPath.c_str());
 						IntVec2 cellcount							= ParseXmlAttribute(*grandChildElement, "cellCount", IntVec2(0, 0));
						m_weaponSpriteSheet							= new SpriteSheet(*spriteTexture, cellcount);
					}
					newSpriteAnim->m_spriteSheet					= m_weaponSpriteSheet;
					newSpriteAnim->m_secondsPerFrame				= ParseXmlAttribute(*grandChildElement, "secondsPerFrame", 0.f);
					newSpriteAnim->m_startSpriteIndex				= ParseXmlAttribute(*grandChildElement, "startFrame", 0);
					newSpriteAnim->m_endSpriteIndex					= ParseXmlAttribute(*grandChildElement, "endFrame", 0);

					m_animations.push_back(newSpriteAnim);

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
					sound						= ParseXmlAttribute(*grandChildElement, "sound", sound);
					soundLocation				= ParseXmlAttribute(*grandChildElement, "name", soundLocation);
					std::string soundEntry = sound + "+" + soundLocation;
					m_sounds.push_back(soundEntry);

					grandChildElement = grandChildElement->NextSiblingElement();
				}
			}
		}
	
		childElement = childElement->NextSiblingElement();
	}
	return true;
}
