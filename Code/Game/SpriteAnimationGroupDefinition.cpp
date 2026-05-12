#include "Game/SpriteAnimationGroupDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Texture.hpp"


bool SpriteAnimationGroupDefinition::LoadFromXmlElement(const XmlElement& element)
{
	m_name			= ParseXmlAttribute(element, "name", "invalid");
	m_scaleBySpeed	= ParseXmlAttribute(element, "scaleBySpeed", false);

	float secondsPerFrame = ParseXmlAttribute(element, "secondsPerFrame", 0.1f);
	std::string playbackModeString = ParseXmlAttribute(element, "playbackMode", "Loop");

	SpriteAnimPlaybackType playbackMode = SpriteAnimPlaybackType::LOOP;	//default
	if (playbackModeString == "Once")      playbackMode = SpriteAnimPlaybackType::ONCE;
	if (playbackModeString == "PingPong")  playbackMode = SpriteAnimPlaybackType::PINGPONG;

	const XmlElement* visualsElement = element.Parent()->ToElement();
	if (!visualsElement) return false;

	std::string spriteSheetPath = ParseXmlAttribute(*visualsElement, "spriteSheet", "invalid");
	IntVec2 cellCount = ParseXmlAttribute(*visualsElement, "cellCount", IntVec2(1,1));

	// "shared" resources
	Texture* sheetTexture = g_engine->m_render->CreateOrGetTextureFromFile(spriteSheetPath.c_str());
	SpriteSheet* groupSpriteSheet = new SpriteSheet(*sheetTexture, cellCount);
	m_spriteSheet = groupSpriteSheet;
	// directions
	const XmlElement* directionElement = element.FirstChildElement("Direction");
	while (directionElement != nullptr)
	{
		Vec3 directionVec = ParseXmlAttribute(*directionElement, "vector", Vec3(1.f, 0.0f, 0.0f)).GetNormalized();

		const XmlElement* animationElement = directionElement->FirstChildElement("Animation");
		if (animationElement != nullptr)
		{
			SpriteAnimDefinition* spriteAnimationDefinition = new SpriteAnimDefinition();
			spriteAnimationDefinition->m_spriteSheet = m_spriteSheet;
			spriteAnimationDefinition->LoadFromXmlElement(*animationElement);
			spriteAnimationDefinition->m_secondsPerFrame = secondsPerFrame;
			spriteAnimationDefinition->m_playbackMode = playbackMode;

			m_directions.push_back(directionVec);
			m_spriteAnimationDefinitions.push_back(spriteAnimationDefinition);
		}

		directionElement = directionElement->NextSiblingElement();
	}
	return true;
}

//std::vector<Vec3> SpriteAnimationGroupDefinition::m_directions;
//std::vector<SpriteAnimDefinition*> SpriteAnimationGroupDefinition::s_spriteAnimationDefinitions;

// camera to actor get vector quantity. run that through the actor's world transform inverse transform, that is the directino you plug into the function below.
const SpriteAnimDefinition& SpriteAnimationGroupDefinition::GetAnimationForDirection(const Vec3& direction) const			// i dont know what the hell to do
{
	int closestDef = 0;
	float closestDot = -2.f;

	Vec3 normalizedDirection = direction.GetNormalized();

	for (int index = 0; index < m_directions.size(); ++index)
	{
		float dotResult = DotProduct3D(normalizedDirection, m_directions[index]);
		if (dotResult > closestDot)
		{
			closestDot = dotResult;
			closestDef = index;
		}
	}

	if (m_spriteAnimationDefinitions[closestDef]->m_spriteSheet == nullptr)
	{
		ERROR_AND_DIE("Spritesheet pointer for animationgroup is null");
	}


	return *m_spriteAnimationDefinitions[closestDef];
}

float SpriteAnimationGroupDefinition::GetDuration() const
{
	int animationFrames = (m_spriteAnimationDefinitions[0]->m_endSpriteIndex - m_spriteAnimationDefinitions[0]->m_startSpriteIndex);
	if (animationFrames <= 0)
	{
		animationFrames = 1;
	}
	return animationFrames * m_spriteAnimationDefinitions[0]->m_secondsPerFrame;
}
