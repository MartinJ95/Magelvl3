#include "ECS.h"

ECS::ECS()
{
}

ECContainer::ECContainer(int Stride) : m_stride(Stride), m_data(), m_entityToCompMap()
{
}

void ECContainer::AddComponent(unsigned int Entity)
{
	if (m_entityToCompMap.find(Entity) != m_entityToCompMap.end())
	{
		return;
	}

	for (int i = 0; i < m_stride; i++)
	{
		m_data.emplace_back();
	}


	m_entityToCompMap.emplace(Entity, m_data.size() / m_stride);
}
