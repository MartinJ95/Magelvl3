#include "Application.h"
#include "CameraComponent.h"
#include "ModelComponent.h"
#include "ServiceLocator.h"
#include "ctime"

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
	InstantiateLocator(*this);
	InputFunction = &InputCallback;
	m_entCompSyst.BeginPlay();

	m_entCompSyst.RegisterComponent<CameraComponent>();
	m_entCompSyst.AddComponent<CameraComponent>(0);

	m_entCompSyst.RegisterComponent<ModelComponent>();
	for (int i = 1; i < 5; i++)
	{
		int dist = 10;

		m_entCompSyst.AddComponent<ModelComponent>(i);
		m_entCompSyst.FindComponent<ModelComponent>(i).SetPosition(
			Vector3(
				(i - 1) % 4 < 2 ? 0 : (i - 1) % 2 > 0 ? dist : -dist,
				0,
				(i - 1) % 4 >= 2 ? 0 : (i - 1) % 2 > 0 ? dist : -dist
			));
	};

	m_entCompSyst.AddComponent<ModelComponent>(5);
	m_entCompSyst.FindComponent<ModelComponent>(5).SetPosition(Vector3(0, -10, 0));
	ModelComponent& test = m_entCompSyst.FindComponent<ModelComponent>(1);
	ModelComponent& test2 = m_entCompSyst.FindComponent<ModelComponent>(2);

}

void Application::Update(float DeltaTime)
{
	while (!m_viz.WindowShouldClose())
	{
		std::clock_t start = std::clock();
		m_entCompSyst.UpdateComponents(DeltaTime);
		Render();
		double ms = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
		std::cout << "Time: " << ms << " ms" << std::endl;
		std::cout << "FPS: " << (ms * 1000) * 60 << std::endl;
	}
}

void Application::Render()
{
	m_viz.Render();
}

void InputCallback(const int Key, const int Scancode, const int Action, const int Mods)
{
	GetEcsInstance().UpdateComponentsInput(Key, Scancode, Action, Mods);
}
