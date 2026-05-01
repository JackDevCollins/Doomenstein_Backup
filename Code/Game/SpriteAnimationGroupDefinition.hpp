#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class SpriteAnimationGroupDefinition
{
public:
	bool	LoadFromXmlElement(const XmlElement& element);

	const SpriteAnimDefinition& GetAnimationForDirection(const Vec3& direction) const;
	float GetDuration() const;
	std::string m_name;
	bool m_scaleBySpeed = false;

	std::vector<Vec3> m_directions;
	std::vector<SpriteAnimDefinition*> m_spriteAnimationDefinitions;

	SpriteSheet* m_spriteSheet = nullptr;

};

// 
// struct direction
// {
// 	Vec3 m_vector = Vec3(0.f, 0.f, 0.f);
// 	int	 m_startFrame = 0;
// 	int	 m_endFrame = 0;
// };
// 
// struct animationGroup
// {
// 	std::string				m_name = "invalid";
// 	bool					m_scaleBySpeed = false;
// 	float					m_secondsPerFrame = 0.0f;
// 	std::string				m_playbackMode = "invalid";
// 	std::vector<direction>  m_directions;
// };
