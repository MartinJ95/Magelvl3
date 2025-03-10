#include "ECS.h"


ECS* EcsInstance = nullptr;

void ECContainer::AddComponent(const unsigned int Entity, const bool IsRunning, const unsigned int CompSize)
{
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

	std::vector<char> objRaw(CompSize);
	std::fill(objRaw.begin(), objRaw.end(), '0');

	Component* obj = (Component*)&*objRaw.data();
	//T obj{};
	obj->AssignEntity(Entity);

	memmove(&m_data[m_data.size() - m_stride], objRaw.data(), CompSize);

	if (IsRunning)
	{
		//Component* c = (Component*)(&obj);
		Component* c = (Component*)(&m_data[m_data.size() - m_stride]);
		c->BeginPlay();
	}

	m_entityToCompMap.emplace(Entity, (m_data.size() / m_stride) - 1);
}

void ECContainer::CleanComponents()
{
	if (!m_dirty)
		return;

	for (int i = 0; i / m_stride < m_data.size() / m_stride; i += m_stride)
	{
		//todo
		//clean component individual
		Component* obj = (Component*)(&m_data[i]);
		obj->CleanComponent();
	}

	m_dirty = false;
}

ECS::ECS() : m_isRunning(false)
{
}

std::unordered_map<unsigned int, Component*> ECS::GetAllComponentsOfEntity(const unsigned int Entity) const
{
	std::unordered_map<unsigned int, Component*> ComponentsOnEntity;

	for (auto& container : m_compContainers)
	{
		
		if (container.second.m_entityToCompMap.find(Entity) != container.second.m_entityToCompMap.end())
		{
			ComponentsOnEntity.emplace(
				std::make_pair(
					container.second.m_stride,
					(Component*) & container.second.m_data.at(
						container.second.m_entityToCompMap.find(Entity)->second * container.second.m_stride
					)
				)
			);
		}
	}
	return ComponentsOnEntity;	
}

void ECS::UpdateComponents(float DeltaTime)
{
	for (auto& container : m_compContainers)
	{
		for (int i = 0; i/container.first < container.second.m_data.size() / container.first; i+=container.first)
		{
			//static_cast<T>(container.second.m_data[i]).Update(DeltaTime);
			Component* obj = (Component*)(&container.second.m_data[i]);
			obj->Update(DeltaTime);
			//i += container.first;
		}
	}
}

void ECS::LateUpdate()
{
	for (auto& container : m_compContainers)
	{
		container.second.CleanComponents();
		for (int i = 0; i / container.first < container.second.m_data.size() / container.first; i += container.first)
		{
			//static_cast<T>(container.second.m_data[i]).Update(DeltaTime);
			Component* obj = (Component*)(&container.second.m_data[i]);
			obj->LateUpdate();
			//i += container.first;
		}
	}
}

void ECS::UpdateComponentsInput(const int Key, const int Scancode, const int Action, const int Mods)
{
	for (auto& container : m_compContainers)
	{
		for (int i = 0; i < container.second.m_data.size() / container.first;)
		{
			//static_cast<T>(container.second.m_data[i]).Update(DeltaTime);
			Component* obj = (Component*)(&container.second.m_data[i]);
			obj->OnInput(Key, Scancode, Action, Mods);
			i += container.first;
		}
	}
}

void ECS::BeginPlay()
{
	m_isRunning = true;
	for (auto& container : m_compContainers)
	{
		for (int i = 0; i < container.second.m_data.size() / container.first;)
		{
			//static_cast<T>(container.second.m_data[i]).Update(DeltaTime);
			Component* obj = (Component*)(&container.second.m_data[i]);
			obj->BeginPlay();
			i += container.first;
		}
	}
}

ECContainer::ECContainer(int Stride) : m_stride(Stride), m_data(), m_entityToCompMap(), m_dirtyListener(*this), m_size(0), m_capacity(ECCCapacity)
{
}

void ECCObserver::OnNotify(const int& Data)
{
	m_parent->m_dirty = true;
}

void ECS::AddComponent(unsigned int Entity, unsigned int CompSize)
{
	assert(m_compContainers.find(CompSize) != m_compContainers.end(), "Forgot to register Component");

	m_entities.emplace(std::make_pair(Entity, "NewEntity"));

	m_compContainers.find(CompSize)->second.AddComponent(Entity, m_isRunning, CompSize);
}