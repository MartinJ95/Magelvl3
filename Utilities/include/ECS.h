#pragma once

#include <unordered_map>
#include <iostream>
#include <vector>
#include "Component.h"
#include "Observer.h"
#include "assert.h"

class ECContainer;

constexpr int ECCCapacity = 100;

class ECCObserver : public Observer<int>
{
public:
	ECCObserver(ECContainer& Container) :
		m_parent(&Container)
	{

	}
	void OnNotify(const int& Data) override final;
public:
	ECContainer* m_parent;
};

class ECContainer
{
public:
	ECContainer(int Stride);
	template<typename T>
	void ConstructComponents();
	template <typename T>
	void AddComponent(unsigned int Entity, bool IsRunning);
	void AddComponent(const unsigned int Entity, const bool IsRunning, const unsigned int CompSize = 0);
	void CleanComponents();
public:
	int m_stride;
	std::vector<char> m_data;
	std::unordered_map<unsigned int, unsigned int> m_entityToCompMap;
	Subject<int> m_dirtyEvent;
	ECCObserver m_dirtyListener;
	int m_size;
	int m_capacity;
	bool m_dirty;
};

class ECS
{
public:
	ECS();
	template<typename T>
	void RegisterComponent(const std::string& CompName);
	template<typename T>
	void AddComponent(unsigned int Entity);
	void AddComponent(unsigned int Entity, unsigned int CompSize);
	template <typename T>
	T& FindComponent(unsigned int Entity);
	std::unordered_map<unsigned int, Component*> GetAllComponentsOfEntity(const unsigned int Entity) const;
	void UpdateComponents(float DeltaTime);
	void LateUpdate();
	void UpdateComponentsInput(const int Key, const int Scancode, const int Action, const int Mods);
	void BeginPlay();
	template <typename T, typename T1>
	void AddComponentDependancy();
public:
	std::unordered_map<unsigned int, ECContainer> m_compContainers;
	std::unordered_map<unsigned int, std::string> m_entities;
	std::unordered_map<unsigned int, std::string> m_componentNames;
	bool m_isRunning;
};

template<typename T>
inline void ECS::RegisterComponent(const std::string& CompName)
{
	/*
	if (m_compContainers.find(sizeof(T)) != m_compContainers.end())
	{
		return;
	}
	*/

	assert(m_compContainers.find(sizeof(T)) == m_compContainers.end());

	m_compContainers.emplace(sizeof(T), sizeof(T));

	m_componentNames.emplace(sizeof(T), CompName);
}

template<typename T>
inline void ECContainer::ConstructComponents()
{
	for (int i = 0; i < ECCCapacity; i++)
	{
		for (int j = 0; j < m_stride; j++)
		{
			m_data.emplace_back();
		}

		T obj{};

		memmove(&m_data[m_data.size() - m_stride], &obj, sizeof(T));
	}
}

template<typename T>
inline void ECContainer::AddComponent(unsigned int Entity, bool IsRunning)
{
	/*
	if (m_entityToCompMap.find(Entity) != m_entityToCompMap.end())
	{
		return;
	}
	*/

	/*
	size_t currentCapactity = m_data.capacity();

	for (int i = 0; i < m_stride; i++)
	{
		m_data.emplace_back();
	}

	if (currentCapactity < m_data.capacity())
	{
		m_dirty = true;
	}

	//T* base = (T)m_data.at(m_data.size() - m_stride);
	//*base = T();

	T obj{};
	obj.AssignEntity(Entity);

	memmove(&m_data[m_data.size() - m_stride], &obj, sizeof(T));
	*/
	assert(m_entityToCompMap.find(Entity) == m_entityToCompMap.end());
	assert(m_size < m_capacity);

	m_size++;

	if (IsRunning)
	{
		//Component* c = (Component*)(&obj);
		Component* c = (Component*)(&m_data[m_size * m_stride - m_stride]);
		c->BeginPlay();
	}

	m_entityToCompMap.emplace(Entity, m_size - 1);
}
template<typename T>
inline void ECS::AddComponent(unsigned int Entity)
{
	/*
	if (m_compContainers.find(sizeof(T)) == m_compContainers.end())
	{
		return;
	}
	*/
	assert(m_compContainers.find(sizeof(T)) != m_compContainers.end(), "Forgot to register Component");

	m_entities.emplace(std::make_pair(Entity, "NewEntity"));

	m_compContainers.find(sizeof(T))->second.AddComponent<T>(Entity, m_isRunning);
}

template<typename T>
inline T& ECS::FindComponent(unsigned int Entity)
{
	
	assert(m_compContainers.find(sizeof(T)) != m_compContainers.end());

	ECContainer& c = m_compContainers.find(sizeof(T))->second;

	assert(c.m_entityToCompMap.find(Entity) != c.m_entityToCompMap.end());

	T* obj = (T*)(&c.m_data[c.m_entityToCompMap.find(Entity)->second * c.m_stride]);

	return *obj;
}

template<typename T, typename T1>
inline void ECS::AddComponentDependancy()
{
	assert(sizeof(T) != sizeof(T1));
	assert(m_compContainers.find(sizeof(T)) != m_compContainers.end());
	assert(m_compContainers.find(sizeof(T1)) != m_compContainers.end());

	m_compContainers.find(sizeof(T))->second.m_dirtyEvent.AddSubscriber(
		&m_compContainers.find(sizeof(T1))->second.m_dirtyListener);
}

extern ECS* EcsInstance;

static ECS& GetEcsInstance()
{
	//assert(EcsInstance != nullptr);

	return *EcsInstance;
}

static void SetEcsInstance(ECS* NewInstance)
{
	EcsInstance = NewInstance;
}