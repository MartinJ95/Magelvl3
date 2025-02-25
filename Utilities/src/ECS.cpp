#include "ECS.h"

ECS* EcsInstance = nullptr;

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

ECContainer::ECContainer(int Stride) : m_stride(Stride), m_data(), m_entityToCompMap(), m_dirtyListener(*this)
{
}

void ECCObserver::OnNotify(const int& Data)
{
	m_parent->m_dirty = true;
}
