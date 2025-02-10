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
	constexpr int stressTestNum = 6;
	for (int i = 0; i < stressTestNum; i++)
	{
		for (int j = 0; j < stressTestNum; j++)
		{
			for (int k = 0; k < stressTestNum; k++)
			{
				int index = k + j * stressTestNum + i * (stressTestNum * stressTestNum);
				m_entCompSyst.AddComponent<ModelComponent>(index);
				m_entCompSyst.FindComponent<ModelComponent>(index).SetPosition(Vector3(-stressTestNum * 0.5 + i * stressTestNum, -stressTestNum * 0.5 + j * stressTestNum, -stressTestNum * 0.5 + k * stressTestNum));
			}
		}
	}
}

void Application::Update(float DeltaTime)
{
	while (!m_viz.WindowShouldClose())
	{
		static float DTime{ DeltaTime };
		std::clock_t start = std::clock();
		m_entCompSyst.UpdateComponents(DTime);
		Render();
		double ms = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
		std::cout << "Time: " << ms << " ms" << std::endl;
		std::cout << "FPS: " << (ms * 1000) * 60 << std::endl;
		DTime = ms / 1000;
	}
}

void Application::Render()
{
	m_viz.Render();
	OnGUI();
}

void InputCallback(const int Key, const int Scancode, const int Action, const int Mods)
{
	GetEcsInstance().UpdateComponentsInput(Key, Scancode, Action, Mods);
}
