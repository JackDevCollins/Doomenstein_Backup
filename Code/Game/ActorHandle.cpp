#include "Game/ActorHandle.hpp"


const ActorHandle ActorHandle::INVALID = ActorHandle();

ActorHandle::ActorHandle()
	: m_data(0xFFFFFFFF)
{
}

ActorHandle::ActorHandle(unsigned int uid, unsigned int index)
{
	m_data = (static_cast<unsigned int>(uid) << 16) | (static_cast<unsigned int>(index) & MAX_ACTOR_INDEX);
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
	return m_data & MAX_ACTOR_INDEX;
}

bool ActorHandle::operator==(const ActorHandle& other) const
{
	return m_data == other.m_data;
}

bool ActorHandle::operator!=(const ActorHandle& other) const
{
	return m_data != other.m_data;
}
