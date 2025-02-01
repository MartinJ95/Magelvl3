#include "ModelComponent.h"
#include "Vizualisation.h"

ModelComponent::ModelComponent() : m_position(), m_rotation(), m_modelID(0)
{
}

void ModelComponent::BeginPlay()
{
}

void ModelComponent::Update(float DeltaTime)
{
	Renderer& renderer = GetRenderer();

	renderer.AddToRenderQueue(0, m_position);
}

void ModelComponent::OnDestroy()
{
}

void ModelComponent::SetPosition(const Vector3& Position)
{
	m_position = Position;
}
