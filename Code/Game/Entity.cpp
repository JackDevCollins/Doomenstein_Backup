#include "Game/Entity.hpp"
#include "Engine/Math/Mat44.hpp"

Entity::Entity(Game* owner)
	:m_game(owner)
{
}

Entity::~Entity()
{

}

void Entity::Update([[maybe_unused]]float deltaSeconds)		
{
	
}

void Entity::Render() const
{

}

void Entity::DebugRender() const
{

}

Mat44 Entity::GetModelToWorldTransform() const
{	
	// make a translation matrix, make a rotation matrix, append rotation to translation
	Mat44 translationMatrix = Mat44();
	translationMatrix.AppendTranslation3D(m_position);
	Mat44 rotationMatrix = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	translationMatrix.Append(rotationMatrix);
	return translationMatrix;
}

