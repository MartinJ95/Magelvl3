#pragma once

#include "Vizualisation.h"
#include "ECS.h"

extern void InputCallback(const int Key, const int Scancode, const int Action, const int Mods);

class Application
{
public:
	Application();
	Vizualisation& GetVizualisation() { return m_viz; }
	ECS& GetECS() { return m_entCompSyst; }
	~Application();
	void Run();
private:
	void Initialise();
	void Update(float DeltaTime);
	void Render();
private:
	Vizualisation m_viz;
	ECS m_entCompSyst;
};