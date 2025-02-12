#include "RigidBody.h"
#include "random"
#include "assert.h"
#include "ECS.h"
#include "Transform.h"

void RigidBody::BeginPlay()
{
	m_velocity.y += std::rand() % 5 + 5;
	m_velocity += m_downDir * 8;
	Transform& t = GetEcsInstance().FindComponent<Transform>(m_entityID);
	t.Translate(m_velocity);
}

void RigidBody::Update(float DeltaTime)
{
	
	m_velocity *= 1-0.5*DeltaTime;
	m_velocity += m_downDir*m_gravityScale*DeltaTime;
	Transform& t = GetEcsInstance().FindComponent<Transform>(m_entityID);
	t.Translate(m_velocity);
	if (t.GetPosition().y < -40)
	{
		t.Translate((m_downDir * -1) * 80);
	}
	
	
}

void RigidBody::OnDestroy()
{
}
