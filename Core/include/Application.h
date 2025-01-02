#pragma once

#include "Vizualisation.h"

class Application
{
public:
	Application();
	~Application();
	void Run();
private:
	void Initialise();
	void Update(float DeltaTime);
	void Render();
private:
	Vizualisation m_viz;
};