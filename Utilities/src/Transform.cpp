#include "Transform.h"

void Transform::BeginPlay()
{
}

void Transform::Update(float DeltaTime)
{
}

void Transform::OnDestroy()
{
}

void Transform::LateUpdate()
{
	m_lastPosition = m_position;
	m_lastRotation = m_rotation;
}
