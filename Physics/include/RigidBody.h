#pragma once
#include "Component.h"
#include "Vector.h"

class RigidBody : public Component
{
public:
	void BeginPlay() override final;
	void Update(float DeltaTime) override final;
	void OnDestroy() override final;
public:
	Vector3 m_velocity{0,0,0};
	Vector3 m_downDir{0, -1, 0};
	float m_mass{ 10.f };
	float m_gravityScale{ 0.8f };
};