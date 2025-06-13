#include "ModelComponent.h"
#include "Vizualisation.h"
#include "ECS.h"
#include "Transform.h"

ModelComponent::ModelComponent() : /*m_position(), m_rotation(),*/ m_modelID(2)
{
}

void ModelComponent::BeginPlay()
{
}

void ModelComponent::Update(float DeltaTime)
{
	Renderer& renderer = GetRenderer();
	renderer.AddToRenderQueue(1, GetEcsInstance().FindComponent<Transform>(m_entityID).GetPosition(), m_modelID);
	//renderer.AddToRenderQueue(0, m_position);
}

void ModelComponent::OnDestroy()
{
}

void ModelComponent::SetPosition(const Vector3& Position)
{
	GetEcsInstance().FindComponent<Transform>(m_entityID).SetPosition(Position);
	//m_position = Position;
}
