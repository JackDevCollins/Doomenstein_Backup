#include "Game/TileDefinition.hpp"

void TileDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument document;
	XmlResult result = document.LoadFile(path);
	XmlElement* rootElement = document.RootElement();
	XmlElement* tileDefinitionElement = rootElement->FirstChildElement();
	while (tileDefinitionElement != nullptr)
	{
		std::string elementName = tileDefinitionElement->Name();
		TileDefinition* tileDefinition = new TileDefinition();
		tileDefinition->LoadFromXmlElement(*tileDefinitionElement);
		s_definitions.push_back(tileDefinition);
		tileDefinitionElement = tileDefinitionElement->NextSiblingElement();
	}
}

void TileDefinition::ClearDefinitions()
{
 s_definitions.clear();
}

const TileDefinition* TileDefinition::GetByName(const std::string& name)
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

std::vector<TileDefinition*> TileDefinition::s_definitions;

bool TileDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);
	m_isSolid = ParseXmlAttribute(element, "isSolid", m_isSolid);
	m_mapImagePixelColor = ParseXmlAttribute(element, "mapImagePixelColor", m_mapImagePixelColor);
	m_floorSpriteCoords = ParseXmlAttribute(element, "floorSpriteCoords", m_floorSpriteCoords);
	m_ceilingSpriteCoords = ParseXmlAttribute(element, "ceilingSpriteCoords", m_ceilingSpriteCoords);
	m_wallSpriteCoords = ParseXmlAttribute(element, "wallSpriteCoords", m_wallSpriteCoords);
	return true;
}