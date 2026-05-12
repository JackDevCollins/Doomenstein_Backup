#pragma once
#include <vector>
#include <string>
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"

struct IntVec2;

class TileDefinition
{
public:

	static void InitializeDefinitions(const char* path);
	static void ClearDefinitions();
	static const TileDefinition* GetByName(const std::string& name);

	static std::vector<TileDefinition*> s_definitions;

	bool	LoadFromXmlElement(const XmlElement& element);

	std::string		m_name;
	bool			m_isSolid;
	bool			m_hasTop;
	bool			m_hasBottom;
	float			m_height;
	Rgba8			m_mapImagePixelColor;
	IntVec2			m_floorSpriteCoords;
	IntVec2			m_ceilingSpriteCoords;
	IntVec2			m_wallSpriteCoords;

};
