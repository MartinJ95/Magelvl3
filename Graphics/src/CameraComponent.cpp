#include "CameraComponent.h"
#include "Vizualisation.h"
#include "glm/gtc/quaternion.hpp"
#include <glm/gtx/quaternion.hpp>

CameraComponent::CameraComponent() : 
	m_position(0, 0, 0), m_rotation(0, 0, 0), m_FinalPosition(0,0,0), m_FinalRotation(0,0,0)
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

	m_FinalRotation = m_FinalRotation + m_rotation * CamRotationSpeedScalar * DeltaTime;

	m_rotation = Vector3(0, 0, 0);

	glm::quat rotation = glm::quat(glm::vec3(glm::radians(m_FinalRotation.x), glm::radians(m_FinalRotation.y), glm::radians(m_FinalRotation.z)));

	glm::vec3 direction = glm::vec3(m_position.x, m_position.y, m_position.z) * DeltaTime;

	direction = rotation * direction;

	m_FinalPosition += Vector3(direction.x, direction.y, direction.z);

	std::cout << "cam pos: " << m_FinalPosition.x << "x, " << m_FinalPosition.y << "y, " << m_FinalPosition.z << "z, " << std::endl;

	m_position = Vector3(0, 0, 0);

	//glm::vec3 dir = 
	
	Renderer& render = GetRenderer();

	render.PositionCamera(m_FinalPosition, m_FinalRotation);
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

