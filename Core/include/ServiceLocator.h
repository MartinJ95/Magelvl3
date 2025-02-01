#pragma once

#include "Application.h"

class ServiceLocator
{
public:
	ServiceLocator(Application& App) : m_appRef(App), m_vizRef(App.GetVizualisation())
	{
		SetVizInstance(&m_vizRef);
		//VizInstance = &m_vizRef;
		SetRendererInstance(&m_vizRef.GetRenderer());
		//RendererInstance = &m_vizRef.GetRenderer();
		SetEcsInstance(&App.GetECS());
	}
	Application& GetApp() { return m_appRef; }
	Vizualisation& GetVizualisation() { return m_vizRef; }
	Renderer& GetRenderer() { return m_appRef.GetVizualisation().GetRenderer(); }
private:
	Application& m_appRef;
	Vizualisation& m_vizRef;
};

static ServiceLocator* Instance = nullptr;

static void InstantiateLocator(Application& App)
{
	Instance = new ServiceLocator(App);
}

static ServiceLocator* GetInstance()
{
	assert(Instance != nullptr);

	return Instance;
}

static void ClearLocator()
{
	delete Instance;
	Instance = nullptr;
}