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
protected:
	void Initialise();
	void Update(float DeltaTime);
	virtual void OnGUI() = 0;
	void Render();
protected:
	Vizualisation m_viz;
	ECS m_entCompSyst;
};