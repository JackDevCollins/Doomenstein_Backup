#pragma once
#include <vector>
#include <string>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"

class Image;
class SpriteSheet;
class Shader;
class Texture;
struct IntVec2;

class MapDefinition
{
public:

	static void InitializeDefinitions(const char* path);
	static void ClearDefinitions();
	static const MapDefinition* GetByName(const std::string& name);
	static std::vector<MapDefinition*> s_definitions;

	bool	LoadFromXmlElement(const XmlElement& element);

	std::string		m_name;
	Image*			m_image;
	Shader*			m_shader;
	Texture*		m_spriteSheetTexture;
	IntVec2			m_spriteSheetCellCount;
};