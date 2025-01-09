#pragma once

#include <unordered_map>
#include <vector>

class ECContainer
{
public:
	ECContainer(int Stride);
	void AddComponent(unsigned int Entity);
public:
	int m_stride;
	std::vector<char> m_data;
	std::unordered_map<unsigned int, unsigned int> m_entityToCompMap;
};

class ECS
{
public:
	ECS();
	template<typename T>
	void RegisterCompoenent();
	template<typename T>
	void AddComponent(unsigned int Entity);
	template<typename T>
	void UpdateComponent(float DeltaTime);
public:
	std::unordered_map<unsigned int, ECContainer> m_compContainers;
};

template<typename T>
inline void ECS::RegisterCompoenent()
{
	if (m_compContainers.find(std::size(T)) != m_compContainers.end())
	{
		return;
	}

	m_compContainers.emplace(std::size(T)).
}

template<typename T>
inline void ECS::AddComponent(unsigned int Entity)
{
	if (m_compContainers.find(std::size(T)) != m_compContainers.end())
	{
		return;
	}

	m_compContainers.find(std::size(T))->second
}

template<typename T>
inline void ECS::UpdateComponent(float DeltaTime)
{
	for (auto container& : m_compContainers)
	{
		for (int i = 0; i < container.second.m_data.size() / container.first;)
		{
			std::static_cast<T>(container.second.m_data[i]).Update(DeltaTime);
			i += container.first;
		}
	}
}
