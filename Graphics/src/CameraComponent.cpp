#include "CameraComponent.h"
#include "Vizualisation.h"

CameraComponent::CameraComponent() : 
	m_position(0, 0, 0), m_rotation(0, 0, 0)
{
	
}

void CameraComponent::BeginPlay()
{
	
	for (int i = 0; i < m_keyMovementInputs.size(); i++)
	{
		m_keyMovementInputs[i].m_subject.AddSubscriber(&m_keyMovementInputs[i].m_observer);
		if (i < 4)
		{
			m_keyMovementInputs[i].m_observer.SetControlledVector(m_position);
		}
		else
		{
			m_keyMovementInputs[i].m_observer.SetControlledVector(m_rotation);
		}
	}
	
}

void CameraComponent::Update(float DeltaTime)
{
	for (auto& it : m_keyMovementInputs)
	{
		it.Update();
	}
	/*
	
	m_rotation.x += 0.005;

	if (m_rotation.x > 360)
	{
		m_rotation.x = (int)m_rotation.x % 360;
	}
	*/
	
	Renderer& render = GetRenderer();

	render.PositionCamera(m_position, m_rotation);
}

void CameraComponent::OnDestroy()
{
}

void CameraComponent::OnInput(const int Key, const int Scancode, const int Action, const int Mods)
{
	for (auto& it : m_keyMovementInputs)
	{
		it.CheckInput(Key, Scancode, Action, Mods);
	}
}

