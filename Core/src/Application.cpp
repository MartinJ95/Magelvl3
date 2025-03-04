#include "Application.h"
#include "CameraComponent.h"
#include "ModelComponent.h"
#include "Transform.h"
#include "ServiceLocator.h"
#include "ctime"
#include "RigidBody.h"

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

	m_entCompSyst.RegisterComponent<CameraComponent>("Camera");
	m_entCompSyst.RegisterComponent<Transform>("Transform");
	m_entCompSyst.AddComponentDependancy<Transform, CameraComponent>();
	m_entCompSyst.AddComponent<Transform>(0);
	m_entCompSyst.AddComponent<CameraComponent>(0);

	m_entCompSyst.RegisterComponent<ModelComponent>("Model");
	m_entCompSyst.RegisterComponent<RigidBody>("RigidBody");
	constexpr int stressTestNum = 20;
	for (int i = 0; i < stressTestNum; i++)
	{
		for (int j = 0; j < stressTestNum; j++)
		{
			for (int k = 0; k < stressTestNum; k++)
			{
				int index = 1 + k + j * stressTestNum + i * (stressTestNum * stressTestNum);
				m_entCompSyst.AddComponent<Transform>(index);
				m_entCompSyst.AddComponent<ModelComponent>(index);
				m_entCompSyst.FindComponent<Transform>(index).SetPosition(Vector3(-stressTestNum * 0.5 + i * stressTestNum, -stressTestNum * 0.5 + j * stressTestNum, -stressTestNum * 0.5 + k * stressTestNum));
				m_entCompSyst.AddComponent<RigidBody>(index);
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
		Render(DTime);
		m_entCompSyst.LateUpdate();
		double ms = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
		std::cout << "Time: " << ms << " ms" << std::endl;
		std::cout << "FPS: " << (ms * 1000) * 60 << std::endl;
		DTime = ms / 1000;
	}
}

void Application::Render(const float DeltaTime)
{
	m_viz.Render(DeltaTime);
	OnGUI();
}

void InputCallback(const int Key, const int Scancode, const int Action, const int Mods)
{
	GetEcsInstance().UpdateComponentsInput(Key, Scancode, Action, Mods);
}
