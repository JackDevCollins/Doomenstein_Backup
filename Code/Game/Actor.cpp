#include "Game/Actor.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/VertexUtils.hpp"

Actor::Actor(Game* owner, float physicsHeight, float physicsRadius, Vec3 position, Rgba8 color, bool isStatic)
	:m_game(owner)
	,m_physicsHeight(physicsHeight)
	,m_physicsRadius(physicsRadius)
	,m_position(position)
	,m_color(color)
	,m_isStatic(isStatic)
{
	m_orientation.m_pitchDegrees = 90.f;
	/*AddVertsForCylinder3D(m_physicsCylinder, m_position, m_position + Vec3(0, 0, physicsHeight), m_physicsRadius, m_color);*/
	
}

Actor::~Actor()
{

}

void Actor::Update([[maybe_unused]]float deltaSeconds)		
{
	m_physicsCylinder.clear();
	AddVertsForMe();
}

void Actor::Render() const
{
	g_engine->m_render->BindTexture(nullptr);
	g_engine->m_render->BindShader(nullptr);
	g_engine->m_render->SetBlendMode(BlendMode::OPAQUE);
	g_engine->m_render->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_engine->m_render->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_engine->m_render->SetModelConstants(Mat44(), m_color);
	g_engine->m_render->DrawVertexArray(m_physicsCylinder);

	g_engine->m_render->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);;
	g_engine->m_render->SetModelConstants(Mat44(), Rgba8(m_color.r + 10.f, m_color.g + 10.f, m_color.b + 10.f)); //Rgba8((m_color.r / (unsigned char)1.5), (m_color.g / (unsigned char)1.5), (m_color.b / (unsigned char)1.5)));
	g_engine->m_render->DrawVertexArray(m_physicsCylinder);
}

void Actor::AddVertsForMe()
{
	m_orientation.m_pitchDegrees = 90.f;
	//AddVertsForCylinder3D(m_physicsCylinder, m_position, Vec3(m_position.x , m_position.y, m_position.z + m_physicsHeight), m_physicsRadius, m_color);
	AddVertsForZCylinder3D(m_physicsCylinder, m_position, m_physicsHeight, m_physicsRadius, m_color);
}

Mat44 Actor::GetModelToWorldTransform() const
{	
	// make a translation matrix, make a rotation matrix, append rotation to translation
	Mat44 translationMatrix = Mat44();
	translationMatrix.AppendTranslation3D(m_position);
	Mat44 rotationMatrix = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	translationMatrix.Append(rotationMatrix);
	return translationMatrix;
}

void Actor::TestPojectileInput(Vec3 movement)
{
	m_position += movement * 100.;
}