#pragma once
#include "assert.h"
#include "Vector.h"

class Physics
{
public:
	float m_gravityScale{ 8.f };
	Vector3 m_downVec{0, -1, 0};
};

extern Physics* PhysicsInstance;

static void InitPhysicsInstance(Physics& Instance)
{
	assert(PhysicsInstance  == nullptr);

	PhysicsInstance = &Instance;
}

static Physics& GetPhysicsInstance()
{
	assert(PhysicsInstance != nullptr);

	return *PhysicsInstance;
}