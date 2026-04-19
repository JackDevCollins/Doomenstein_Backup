#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include <string>

extern bool g_debugDraw;

constexpr float		WORLD_SIZE_X = 200.f;
constexpr float		WORLD_SIZE_Y = 100.f;
constexpr float		SCREEN_SIZE_X = 1600.f;
constexpr float		SCREEN_SIZE_Y = 800.f;
constexpr float		WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float		WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;

struct animation
{
	std::string		m_name;
	std::string		m_shader;
	std::string		m_spritesheet;
	Vec2			m_cellcount;
	float			m_secondsPerFrame;
	int				m_startFrame;
	int				m_endFrame;
};

struct direction
{
	Vec3 m_vector = Vec3(0.f, 0.f, 0.f);
	int	 m_startFrame = 0;
	int	 m_endFrame = 0;
};

struct animationGroup
{
	std::string				m_name = "invalid";
	bool					m_scaleBySpeed = false;
	float					m_secondsPerFrame = 0.0f;
	std::string				m_playbackMode = "invalid";
	std::vector<direction>  m_directions;
};

void DebugDrawLine(Vec2 startPos, Vec2 endPos, float thickness, Rgba8 const& color);
void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void DrawGlow( Vec2 pos, Rgba8 color, float alpha, float radius);
void DrawFadedRing(Vec2 const& center, float innerRadius, float outerRadius, Rgba8 const& innerColor, Rgba8 outerColor);