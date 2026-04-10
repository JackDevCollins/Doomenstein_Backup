#include "Game/MapDefinition.hpp"
#include "Engine/Renderer/Image.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Engine.hpp"

void MapDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument document;
	XmlResult result = document.LoadFile(path);
	XmlElement* rootElement = document.RootElement();
	XmlElement* mapDefinitionElement = rootElement->FirstChildElement();
	while (mapDefinitionElement != nullptr)
	{
		std::string elementName = mapDefinitionElement->Name();
		MapDefinition* mapDefinition = new MapDefinition();
		mapDefinition->LoadFromXmlElement(*mapDefinitionElement);
		s_definitions.push_back(mapDefinition);
		mapDefinitionElement = mapDefinitionElement->NextSiblingElement();
	}
}

void MapDefinition::ClearDefinitions()
{
	s_definitions.clear();
}

const MapDefinition* MapDefinition::GetByName(const std::string& name)
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

std::vector<MapDefinition*> MapDefinition::s_definitions;


bool MapDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);
	std::string imagePath = ParseXmlAttribute(element, "image", "Error");
	m_image = new Image(imagePath.c_str());
	std::string shaderPath = ParseXmlAttribute(element, "shader", "Error");
	m_shader = g_engine->m_render->CreateOrGetShader(shaderPath.c_str(), VertexType::VERTEX_PCUTBN);
	std::string spriteSheetPath = ParseXmlAttribute(element, "spriteSheetTexture", "Error");
	m_spriteSheetTexture = g_engine->m_render->CreateOrGetTextureFromFile(spriteSheetPath.c_str());
	m_spriteSheetCellCount = ParseXmlAttribute(element, "spriteSheetCellCount", m_spriteSheetCellCount);

	return true;
}
