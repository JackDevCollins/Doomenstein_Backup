#include "Game/ActorDefinition.hpp"


void ActorDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument document;
	XmlResult result = document.LoadFile(path);
	XmlElement* rootElement = document.RootElement();
	XmlElement* actorDefinitionElement = rootElement->FirstChildElement();
	while (actorDefinitionElement != nullptr)
	{
		std::string elementName = actorDefinitionElement->Name();
		actorDefinition* actorDefinition = new ActorDefinition();
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

std::vector<actorDefinition*> actorDefinition::s_definitions;

bool ActorDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);
	m_isSolid = ParseXmlAttribute(element, "isSolid", m_isSolid);
	m_mapImagePixelColor = ParseXmlAttribute(element, "mapImagePixelColor", m_mapImagePixelColor);
	m_floorSpriteCoords = ParseXmlAttribute(element, "floorSpriteCoords", m_floorSpriteCoords);
	m_ceilingSpriteCoords = ParseXmlAttribute(element, "ceilingSpriteCoords", m_ceilingSpriteCoords);
	m_wallSpriteCoords = ParseXmlAttribute(element, "wallSpriteCoords", m_wallSpriteCoords);
	return true;
}