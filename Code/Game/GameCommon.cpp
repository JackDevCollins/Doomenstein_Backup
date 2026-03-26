#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Engine.hpp"

bool g_debugDraw = false;
//BitmapFont*	g_defaultFont	= nullptr;

//-----------------------------------------------------------------------------------------------
void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	g_engine->m_render->SetModelConstants();
	g_engine->m_render->BindTexture(nullptr);
	g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_engine->m_render->SetBlendMode(BlendMode::OPAQUE);
	g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_engine->m_render->SetSamplerMode(SamplerMode::POINT_CLAMP);

	float halfThickness = 0.5f * thickness;
	float innerRadius = radius - halfThickness;
	float outerRadius = radius + halfThickness;
	constexpr int NUM_SIDES = 32;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;		// each side is a trapezoid
	constexpr int NUM_VERTS = 3 * NUM_TRIS;		// 3 vertexes per triangle
	Vertex verts[NUM_VERTS];
	
	constexpr float DEGREES_PER_SIDE = 360.f / static_cast<float> ( NUM_SIDES );
	for (int sideNum = 0; sideNum < NUM_SIDES; ++sideNum)
	{
		// compute angle-related terms
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>( sideNum );
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>( sideNum + 1);
		float cosStart = CosDegrees( startDegrees );
		float sinStart = SinDegrees( startDegrees );
		float cosEnd = CosDegrees( endDegrees );
		float sinEnd = SinDegrees( endDegrees );

		// Compute inner and outer positions
		Vec3 innerStartPos ( center.x + innerRadius * cosStart, center.y + innerRadius * sinStart, 0.f );
		Vec3 outerStartPos( center.x + outerRadius * cosStart, center.y + outerRadius * sinStart, 0.f );
		Vec3 outerEndPos ( center.x + outerRadius * cosEnd, center.y + outerRadius * sinEnd, 0.f );
		Vec3 innerEndPos ( center.x + innerRadius * cosEnd, center.y + innerRadius * sinEnd, 0.f );

		// Trapezoid is made of two triangles; ABC and DEF
		// A is inner end; B is inner start; C is outer start
		// D is inner end; E is outer start; F is outer end
		int vertIndexA = (6 * sideNum) + 0;
		int vertIndexB = (6 * sideNum) + 1;
		int vertIndexC = (6 * sideNum) + 2;
		int vertIndexD = (6 * sideNum) + 3;
		int vertIndexE = (6 * sideNum) + 4;
		int vertIndexF = (6 * sideNum) + 5;

		verts[ vertIndexA ].m_position = innerEndPos;
		verts[ vertIndexB ].m_position = innerStartPos;
		verts[ vertIndexC ].m_position = outerStartPos;
		verts[ vertIndexA ].m_color = color;
		verts[ vertIndexB ].m_color = color;
		verts[ vertIndexC ].m_color = color;



		verts[vertIndexD].m_position = innerEndPos;
		verts[vertIndexE].m_position = outerStartPos;
		verts[vertIndexF].m_position = outerEndPos;
		verts[vertIndexD].m_color = color;
		verts[vertIndexE].m_color = color;
		verts[vertIndexF].m_color = color;
	}

	g_engine->m_render->DrawVertexArray(NUM_VERTS,verts );
}


void DebugDrawLine(Vec2 startPos, Vec2 endPos, float thickness, Rgba8 const& color)
{
		// comp half thickness long forward adn left displacment
		float halfThickness = 0.5f * thickness;
		Vec2	forwardDisp = endPos - startPos;
		Vec2	forwardNormal = forwardDisp.GetNormalized();
		Vec2	forwardStep = forwardNormal * (halfThickness);
		Vec2	leftStep = forwardStep.GetRotatedBy90Degrees();
		
		// calc corner pos
		Vec2 endLeft = endPos + forwardStep + leftStep * halfThickness;
		Vec2 endRight = endPos + forwardStep - leftStep * halfThickness;
		Vec2 startLeft = startPos - forwardStep + leftStep * halfThickness;
		Vec2 startRight = startPos - forwardStep - leftStep * halfThickness;

		// create vertexes, then draw
		Vertex verts[6];
		
		verts[0] = Vertex(Vec3(startRight.x, startRight.y, 0.0f), color, Vec2(0.f, 0.f));
		verts[1] = Vertex(Vec3(endRight.x, endRight.y, 0.0f), color, Vec2(0.f, 0.f));
		verts[2] = Vertex(Vec3(endLeft.x, endLeft.y, 0.0f), color, Vec2(0.f, 0.f));
		verts[3] = Vertex(Vec3(endLeft.x, endLeft.y, 0.0f), color, Vec2(0.f, 0.f));
		verts[4] = Vertex(Vec3(startLeft.x, startLeft.y, 0.0f), color, Vec2(0.f, 0.f));
		verts[5] = Vertex(Vec3(startRight.x, startRight.y, 0.0f), color, Vec2(0.f, 0.f));

		g_engine->m_render->DrawVertexArray(6,verts );

		
} 

void DrawGlow(Vec2 pos, Rgba8 color, float alpha, float radius)
{
	color.ScaleAlpha(alpha);

	float innerRadius = 0.f;
	float outerRadius = radius;
	Rgba8 innerColor = color;
	Rgba8 outerColor(innerColor.r, innerColor.g,innerColor.b, 0);
	DrawFadedRing(pos, innerRadius, outerRadius, innerColor, outerColor);
}


void DrawFadedRing(Vec2 const& center, float innerRadius, float outerRadius, Rgba8 const& innerColor, Rgba8 outerColor)
{
	
	constexpr int NUM_SIDES = 32;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;		// each side is a trapezoid
	constexpr int NUM_VERTS = 3 * NUM_TRIS;		// 3 vertexes per triangle
	Vertex verts[NUM_VERTS];

	constexpr float DEGREES_PER_SIDE = 360.f / static_cast<float> (NUM_SIDES);
	for (int sideNum = 0; sideNum < NUM_SIDES; ++sideNum)
	{
		// compute angle-related terms
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum + 1);
		float cosStart = CosDegrees(startDegrees);
		float sinStart = SinDegrees(startDegrees);
		float cosEnd = CosDegrees(endDegrees);
		float sinEnd = SinDegrees(endDegrees);

		// Compute inner and outer positions
		Vec3 innerStartPos(center.x + innerRadius * cosStart, center.y + innerRadius * sinStart, 0.f);
		Vec3 outerStartPos(center.x + outerRadius * cosStart, center.y + outerRadius * sinStart, 0.f);
		Vec3 outerEndPos(center.x + outerRadius * cosEnd, center.y + outerRadius * sinEnd, 0.f);
		Vec3 innerEndPos(center.x + innerRadius * cosEnd, center.y + innerRadius * sinEnd, 0.f);

		// Trapezoid is made of two triangles; ABC and DEF
		// A is inner end; B is inner start; C is outer start
		// D is inner end; E is outer start; F is outer end
		int vertIndexA = (6 * sideNum) + 0;
		int vertIndexB = (6 * sideNum) + 1;
		int vertIndexC = (6 * sideNum) + 2;
		int vertIndexD = (6 * sideNum) + 3;
		int vertIndexE = (6 * sideNum) + 4;
		int vertIndexF = (6 * sideNum) + 5;

		verts[vertIndexA].m_position = innerEndPos;
		verts[vertIndexB].m_position = innerStartPos;
		verts[vertIndexC].m_position = outerStartPos;
		verts[vertIndexA].m_color = innerColor;
		verts[vertIndexB].m_color = innerColor;
		verts[vertIndexC].m_color = outerColor;



		verts[vertIndexD].m_position = innerEndPos;
		verts[vertIndexE].m_position = outerStartPos;
		verts[vertIndexF].m_position = outerEndPos;
		verts[vertIndexD].m_color = innerColor;
		verts[vertIndexE].m_color = outerColor;
		verts[vertIndexF].m_color = outerColor;
	}

	g_engine->m_render->DrawVertexArray(NUM_VERTS, verts);
}
