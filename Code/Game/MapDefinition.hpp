#pragma once
#include <vector>
#include <string>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"

class Image;
class SpriteSheet;
class Shader;
class Texture;
struct IntVec2;

struct SpawnInfo
{
public:

	std::string  m_actorType = "_";
	Vec3		 m_position = Vec3(0, 0, 0);
	EulerAngles	 m_orientation = EulerAngles(0, 0, 0);
	Vec3		 m_velocity = Vec3(0, 0, 0);

};


class MapDefinition
{
public:

	static void InitializeDefinitions(const char* path);
	static void ClearDefinitions();
	static const MapDefinition* GetByName(const std::string& name);
	static std::vector<MapDefinition*> s_definitions;

	bool	LoadFromXmlElement(const XmlElement& element);

	std::string							m_name;
	Image*								m_image;
	Shader*								m_shader;
	Texture*							m_spriteSheetTexture;
	IntVec2								m_spriteSheetCellCount;
	std::vector<const SpawnInfo*>		m_spawnInfos;
};