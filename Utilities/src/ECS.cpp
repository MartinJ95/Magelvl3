#include "ECS.h"


ECS* EcsInstance = nullptr;

void ECContainer::AddComponent(const unsigned int Entity, const bool IsRunning, const unsigned int CompSize)
{
	/*
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
	*/
	assert(m_entityToCompMap.find(Entity) == m_entityToCompMap.end());
	assert(m_size < m_capacity);

	m_size++;
	Component* c = (Component*)(&m_data[m_size * m_stride - m_stride]);
	c->AssignEntity(Entity);

	if (IsRunning)
	{
		//Component* c = (Component*)(&obj);
		c->BeginPlay();
	}

	m_entityToCompMap.emplace(Entity, m_size - 1);
}

void ECContainer::CleanComponents()
{
	if (!m_dirty)
		return;

	for (int i = 0; i < m_size; i++)
	{
		Component* obj = (Component*)(&m_data[i*m_stride]);
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
		for (int i = 0; i < container.second.m_size; i++)
		{
			//static_cast<T>(container.second.m_data[i]).Update(DeltaTime);
			Component* obj = (Component*)(&container.second.m_data[i * container.first]);
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
		for (int i = 0; i < container.second.m_size; i++)
		{
			//static_cast<T>(container.second.m_data[i]).Update(DeltaTime);
			Component* obj = (Component*)(&container.second.m_data[i * container.first]);
			obj->LateUpdate();
			//i += container.first;
		}
	}
}

void ECS::UpdateComponentsInput(const int Key, const int Scancode, const int Action, const int Mods)
{
	for (auto& container : m_compContainers)
	{
		for (int i = 0; i < container.second.m_size; i++)
		{
			//static_cast<T>(container.second.m_data[i]).Update(DeltaTime);
			Component* obj = (Component*)(&container.second.m_data[i * container.second.m_stride]);
			obj->OnInput(Key, Scancode, Action, Mods);
		}
	}
}

void ECS::BeginPlay()
{
	m_isRunning = true;
	for (auto& container : m_compContainers)
	{
		for (int i = 0; i < container.second.m_size; i++)
		{
			//static_cast<T>(container.second.m_data[i]).Update(DeltaTime);
			Component* obj = (Component*)(&container.second.m_data[i * container.first]);
			obj->BeginPlay();
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