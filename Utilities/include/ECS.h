#pragma once

#include <unordered_map>
#include <vector>
#include "Component.h"
#include "Observer.h"

class ECContainer;

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
	template <typename T>
	void AddComponent(unsigned int Entity, bool IsRunning);
	void CleanComponents();
public:
	int m_stride;
	std::vector<char> m_data;
	std::unordered_map<unsigned int, unsigned int> m_entityToCompMap;
	Subject<int> m_dirtyEvent;
	ECCObserver m_dirtyListener;
	bool m_dirty;
};

class ECS
{
public:
	ECS();
	template<typename T>
	void RegisterComponent();
	template<typename T>
	void AddComponent(unsigned int Entity);
	template <typename T>
	T& FindComponent(unsigned int Entity);
	const std::unordered_map<unsigned int, void*>& GetAllComponentsOfEntity(const int Entity) const;
	void UpdateComponents(float DeltaTime);
	void LateUpdate();
	void UpdateComponentsInput(const int Key, const int Scancode, const int Action, const int Mods);
	void BeginPlay();
	template <typename T, typename T1>
	void AddComponentDependancy();
public:
	std::unordered_map<unsigned int, ECContainer> m_compContainers;
	bool m_isRunning;
};

template<typename T>
inline void ECS::RegisterComponent()
{
	/*
	if (m_compContainers.find(sizeof(T)) != m_compContainers.end())
	{
		return;
	}
	*/

	assert(m_compContainers.find(sizeof(T)) == m_compContainers.end());

	m_compContainers.emplace(sizeof(T), sizeof(T));
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

	assert(m_entityToCompMap.find(Entity) == m_entityToCompMap.end());

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

	if (IsRunning)
	{
		//Component* c = (Component*)(&obj);
		Component* c = (Component*)(&m_data[m_data.size() - m_stride]);
		c->BeginPlay();
	}

	m_entityToCompMap.emplace(Entity, (m_data.size() / m_stride) - 1);
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