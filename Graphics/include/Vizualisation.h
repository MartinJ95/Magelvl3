#pragma once

#include "Renderer.h"

#include <iostream>
#include <thread>

class Vizualisation
{
public:
	Vizualisation(int Width = 500, int Height = 500);
	void Render();
	~Vizualisation();
private:
	int m_width = 500.f;
	int m_height = 500.f;
	Renderer m_renderer;
};