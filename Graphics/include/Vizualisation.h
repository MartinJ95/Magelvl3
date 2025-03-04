#pragma once

#include "Renderer.h"

#include <iostream>
#include <thread>
#include "Vector.h"

class Vizualisation
{
public:
	Vizualisation(int Width = 1280, int Height = 720);
	void AddToRenderQueue(const Vector3& position);
	void Render(const float DeltaTime);
	Renderer& GetRenderer() { return m_renderer; }
	bool WindowShouldClose() const;
	void OnGUIStart();
	~Vizualisation();
private:
	int m_width = 500.f;
	int m_height = 500.f;
	Renderer m_renderer;
};

extern Vizualisation* VizInstance;

extern Renderer* RendererInstance;
static void SetVizInstance(Vizualisation* Viz)
{
	VizInstance = Viz;
}

static Vizualisation& GetVizInstance()
{
	return *VizInstance;
}

static void SetRendererInstance(Renderer* Renderer)
{
	RendererInstance = Renderer;
}

static Renderer& GetRenderer()
{
	assert(RendererInstance != nullptr);

	return *RendererInstance;
}