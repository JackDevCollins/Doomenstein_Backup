#include "Game/Prop.hpp"


Prop::Prop(Game* owner)
	:Entity(owner)
{
}

Prop::~Prop()
{

}

void Prop::Update([[maybe_unused]]float deltaSeconds)
{

}

void Prop::Render() const
{

	g_engine->m_render->SetModelConstants(GetModelToWorldTransform(), m_color);
	g_engine->m_render->SetBlendMode(BlendMode::OPAQUE);
	g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_engine->m_render->BindTexture(m_texture);
	g_engine->m_render->DrawVertexArray(m_vertexes);
}

void Prop::DebugRender() const
{

}
