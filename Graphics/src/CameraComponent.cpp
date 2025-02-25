#include "CameraComponent.h"
#include "Vizualisation.h"
#include "glm/gtc/quaternion.hpp"
#include <glm/gtx/quaternion.hpp>
#include "ECS.h"
#include "Transform.h"

CameraComponent::CameraComponent() //: 
	//m_position(0, 0, 0), m_rotation(0, 0, 0), m_FinalPosition(0,0,0), m_FinalRotation(0,0,0)
{
	
}

void CameraComponent::BeginPlay()
{
	
	for (int i = 0; i < m_keyMovementInputs.size(); i++)
	{
		m_keyMovementInputs[i].m_subject.AddSubscriber(&m_keyMovementInputs[i].m_observer);
		if (i < 4)
		{
			m_keyMovementInputs[i].m_observer.SetControlledVector(GetEcsInstance().FindComponent<Transform>(m_entityID).GetPositionRef());
		}
		else
		{
			m_keyMovementInputs[i].m_observer.SetControlledVector(GetEcsInstance().FindComponent<Transform>(m_entityID).GetRotationRef());
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

	Transform& t = GetEcsInstance().FindComponent<Transform>(m_entityID);

	/*

	glm::vec3 finalRotation = m_FinalRotation + t.GetRotation() * CamRotationSpeedScalar * DeltaTime;
	//m_FinalRotation = m_FinalRotation + t.GetRotation() * CamRotationSpeedScalar * DeltaTime;

	m_rotation = Vector3(0, 0, 0);

	glm::quat rotation = glm::quat(glm::vec3(glm::radians(m_FinalRotation.x), glm::radians(m_FinalRotation.y), glm::radians(m_FinalRotation.z)));

	glm::vec3 direction = glm::vec3(m_position.x, m_position.y, m_position.z) * DeltaTime;

	direction = rotation * direction;

	m_FinalPosition += Vector3(direction.x, direction.y, direction.z);

	std::cout << "cam pos: " << m_FinalPosition.x << "x, " << m_FinalPosition.y << "y, " << m_FinalPosition.z << "z, " << std::endl;

	m_position = Vector3(0, 0, 0);

	//glm::vec3 dir = 
	
	*/
	Renderer& render = GetRenderer();

	//t.SetPosition(t.GetPosition() + t.GetCurrentMove().Normalised() * DeltaTime);

	t.SetRotation(t.GetRotation() + t.GetCurrentRotate() * DeltaTime * CamRotationSpeedScalar);

	Vector3 finalRotate = t.GetRotation() + t.GetCurrentRotate();
	Vector3 dir = t.GetCurrentMove();

	glm::quat rotation = glm::quat(glm::vec3(glm::radians(finalRotate.x), glm::radians(finalRotate.y), glm::radians(finalRotate.z)));

	glm::vec3 direction = glm::vec3(dir.x, dir.y, dir.z);

	direction = direction * rotation;

	t.SetPosition(t.GetPosition() + Vector3(direction.x, direction.y, direction.z));

	//render.PositionCamera(m_FinalPosition, m_FinalRotation);
	render.PositionCamera(t.GetPosition(), t.GetRotation());
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

void CameraComponent::CleanComponent()
{
	for (int i = 0; i < m_keyMovementInputs.size(); i++)
	{
		//m_keyMovementInputs[i].m_subject.AddSubscriber(&m_keyMovementInputs[i].m_observer);
		if (i < 4)
		{
			m_keyMovementInputs[i].m_observer.SetControlledVector(GetEcsInstance().FindComponent<Transform>(m_entityID).GetPositionRef());
		}
		else
		{
			m_keyMovementInputs[i].m_observer.SetControlledVector(GetEcsInstance().FindComponent<Transform>(m_entityID).GetRotationRef());
		}
	}
}
