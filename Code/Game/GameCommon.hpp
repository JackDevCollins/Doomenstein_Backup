#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

extern bool g_debugDraw;

constexpr float		WORLD_SIZE_X = 200.f;
constexpr float		WORLD_SIZE_Y = 100.f;
constexpr float		SCREEN_SIZE_X = 1600.f;
constexpr float		SCREEN_SIZE_Y = 800.f;
constexpr float		WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float		WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;

void DebugDrawLine(Vec2 startPos, Vec2 endPos, float thickness, Rgba8 const& color);
void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void DrawGlow( Vec2 pos, Rgba8 color, float alpha, float radius);
void DrawFadedRing(Vec2 const& center, float innerRadius, float outerRadius, Rgba8 const& innerColor, Rgba8 outerColor);