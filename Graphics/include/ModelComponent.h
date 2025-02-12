#pragma once

#include "Renderer.h"
#include "Component.h"

class ModelComponent : public Component
{
public:
	ModelComponent();
	virtual void BeginPlay();
	virtual void Update(float DeltaTime);
	virtual void OnDestroy();
	void SetPosition(const Vector3& Position);
private:
	//Vector3 m_position;
	//Vector3 m_rotation;
	unsigned int m_modelID;
};