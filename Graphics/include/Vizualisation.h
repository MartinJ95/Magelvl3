#pragma once

#include "Renderer.h"

#include <iostream>
#include <thread>
#include "Vector.h"

class Vizualisation
{
public:
	Vizualisation(int Width = 500, int Height = 500);
	void AddToRenderQueue(const Vector3& position);
	void Render();
	~Vizualisation();
private:
	int m_width = 500.f;
	int m_height = 500.f;
	Renderer m_renderer;
};