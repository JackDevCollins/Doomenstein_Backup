#include "Game/ActorHandle.hpp"


const ActorHandle ActorHandle::INVALID = ActorHandle();

ActorHandle::ActorHandle()
	: m_data(0xFFFFFFFF)
{
}

ActorHandle::ActorHandle(unsigned int uid, unsigned int index)
{
	m_data = (static_cast<unsigned int>(uid & 0xFFFF) << 16) | (static_cast<unsigned int>(index) & MAX_ACTOR_INDEX);
}

bool ActorHandle::IsValid() const
{
	 return(m_data != INVALID.m_data);
}

unsigned int ActorHandle::GetIndex() const
{
	return m_data & MAX_ACTOR_INDEX;
}

unsigned int ActorHandle::GetUID() const
{
	return (m_data >> 16) & 0xFFFF;
}

bool ActorHandle::operator==(const ActorHandle& other) const
{
	return m_data == other.m_data;
}

bool ActorHandle::operator!=(const ActorHandle& other) const
{
	return m_data != other.m_data;
}
