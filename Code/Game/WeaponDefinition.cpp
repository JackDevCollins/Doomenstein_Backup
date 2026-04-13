#include "Game/WeaponDefinition.hpp"


void WeaponDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument document;
	XmlResult result = document.LoadFile(path);
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
	m_refireTime		=	 ParseXmlAttribute(element, "name", m_refireTime);
	m_rayCount			=	 ParseXmlAttribute(element, "name", m_rayCount);
	m_rayCone			=	 ParseXmlAttribute(element, "name", m_rayCone);
	m_rayRange			=	 ParseXmlAttribute(element, "name", m_rayRange);
	m_rayDamage			=	 ParseXmlAttribute(element, "name", m_rayDamage);
	m_rayImpulse		=	 ParseXmlAttribute(element, "name", m_rayImpulse);
	m_HUDshader			=	 ParseXmlAttribute(element, "name", m_HUDshader);
	m_baseTexture		=	 ParseXmlAttribute(element, "name", m_baseTexture);
	m_reticleTexture	=	 ParseXmlAttribute(element, "name", m_reticleTexture);
	m_reticleSize		=	 ParseXmlAttribute(element, "name", m_reticleSize);
	m_spriteSize		=	 ParseXmlAttribute(element, "name", m_spriteSize);
	m_spritePivot		=	 ParseXmlAttribute(element, "name", m_spritePivot);

	return true;
}