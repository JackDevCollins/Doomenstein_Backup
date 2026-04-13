#include "Game/ActorHandle.hpp"


ActorHandle::ActorHandle()
{

}

ActorHandle::ActorHandle(unsigned int uid, unsigned int index)
{
	
}

bool ActorHandle::IsValid() const
{
	if (m_data != INVALID.m_data)
	{
		return true;
	}
}

unsigned int ActorHandle::GetIndex() const
{

}

bool ActorHandle::operator==(const ActorHandle& other) const
{
	if (m_data == other.m_data)
	{
		return true;
	}
	
	return false;
}

bool ActorHandle::operator!=(const ActorHandle& other) const
{
	if (m_data == other.m_data)
	{
		return false;
	}
	
	return true;
}
