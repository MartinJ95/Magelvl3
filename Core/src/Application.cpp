#include "Application.h"

Application::Application()
{
}

Application::~Application()
{
}

void Application::Run()
{
	Initialise();
	Update(1.f);
}

void Application::Initialise()
{
}

void Application::Update(float DeltaTime)
{
	Render();
}

void Application::Render()
{
	m_viz.Render();
}
