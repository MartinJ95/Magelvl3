#pragma once

#include "Vizualisation.h"
#include "PhysicsBase.h"
#include "ECS.h"

extern void InputCallback(const int Key, const int Scancode, const int Action, const int Mods);

class Application
{
public:
	Application();
	Vizualisation& GetVizualisation() { return m_viz; }
	ECS& GetECS() { return m_entCompSyst; }
	Physics& GetPhysics() { return m_physics; }
	~Application();
	void Run();
protected:
	void Initialise();
	void Update(float DeltaTime);
	virtual void OnGUI() = 0;
	void Render(const float DeltaTime);
protected:
	Vizualisation m_viz;
	Physics m_physics;
	ECS m_entCompSyst;
};