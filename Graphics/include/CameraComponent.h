#pragma once

#include "Renderer.h"
#include "Component.h"
#include "Observer.h"

class MovementObserver : public Observer<Vector3>
{
public:
	MovementObserver(Vector3* ControlledVector) : m_controlledVector(ControlledVector) 
	{
	}
	virtual void OnNotify(const Vector3& Data)
	{
		*m_controlledVector += Data;
	}
	void SetControlledVector(Vector3& ControlledVector)
	{
		m_controlledVector = &ControlledVector;
	}
private:
	Vector3* m_controlledVector;
};

typedef Subject<Vector3> MovementSubject;

/*
class MovementSubject : public Subject<Vector3>
{

};
*/

struct MovementKey
{
	MovementKey(int Key, const Vector3& MoveVector, Vector3* ControlledVector) : 
		m_key(Key), m_moveVector(MoveVector), m_observer(ControlledVector), m_subject(), m_keyDown(false)
	{
		//m_subject.AddSubscriber(&m_observer);
		//glfwSetKeyCallback(GetRenderer().m_surfaceData.window.handle, KeyCallback);
	}
	MovementKey(const MovementKey& Other) :
		m_key(Other.m_key), m_moveVector(Other.m_moveVector), m_observer(std::move(Other.m_observer)), m_subject(std::move(Other.m_subject)), m_keyDown(Other.m_keyDown)
	{

	}
	MovementKey(MovementKey&& Other) :
		m_key(Other.m_key), m_moveVector(Other.m_moveVector), m_observer(Other.m_observer), m_subject(Other.m_subject), m_keyDown(Other.m_keyDown)
	{

	}
	void operator=(const MovementKey& Other)
	{
		m_key = Other.m_key;
		m_moveVector = Other.m_moveVector;
		m_observer = Other.m_observer;
		m_subject = Other.m_subject;
		m_keyDown = Other.m_keyDown;
	}
	void operator=(MovementKey&& Other)
	{
		m_key = Other.m_key;
		m_moveVector = Other.m_moveVector;
		m_observer = std::move(Other.m_observer);
		m_subject = std::move(Other.m_subject);
		m_keyDown = Other.m_keyDown;
		m_observer.SetSubjects({});
		m_subject.SetObservers({});
	}
	
	/*
	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == m_key)
		{
			if (action == GLFW_KEY_DOWN)
			{
				m_keyDown = true;
			}
			else if (action == GLFW_KEY_UP)
			{
				m_keyDown = false;
			}
		}
	}
	*/
	void CheckInput(const int Key, const int Scancode, const int Action, const int Mods)
	{
		if (Key == m_key)
		{
			if (Action == GLFW_PRESS)
			{
				m_keyDown = true;
			}
			else if (Action == GLFW_RELEASE)
			{
				m_keyDown = false;
			}
		}
	}
	void Update()
	{
		if (m_keyDown)
		{
			m_subject.Notify(m_moveVector);
		}
	}
	int m_key;
	Vector3 m_moveVector;
	MovementObserver m_observer;
	MovementSubject m_subject;
	bool m_keyDown;
};

constexpr float CamSpeed(10);
constexpr float CamRotationSpeedScalar(10);

class CameraComponent : public Component
{
public:
	CameraComponent();
	CameraComponent(const CameraComponent& Other)
	{
		for (int i = 0; i < m_keyMovementInputs.size(); i++)
		{
			m_keyMovementInputs[i] = Other.m_keyMovementInputs[i];
		}
	}
	CameraComponent(CameraComponent&& Other)
	{
		for (int i = 0; i < m_keyMovementInputs.size(); i++)
		{
			m_keyMovementInputs[i] = std::move(Other.m_keyMovementInputs[i]);
		}
	}
	void operator=(const CameraComponent& Other)
	{
		for (int i = 0; i < m_keyMovementInputs.size(); i++)
		{
			m_keyMovementInputs[i] = Other.m_keyMovementInputs[i];
		}
	}
	void operator=(CameraComponent&& Other)
	{
		for (int i = 0; i < m_keyMovementInputs.size(); i++)
		{
			m_keyMovementInputs[i] = std::move(Other.m_keyMovementInputs[i]);
		}
	}
	void BeginPlay() override final;
	void Update(float DeltaTime) override final;
	void OnDestroy() override final;
	void OnInput(const int Key, const int Scancode, const int Action, const int Mods) override final;
	void CleanComponent() override final;
	void SafetyChecks(void* Other) override final;
private:
	std::array<MovementKey, 6> m_keyMovementInputs = {
		MovementKey(GLFW_KEY_W, Vector3(0, 0, CamSpeed), nullptr),
		MovementKey(GLFW_KEY_S, Vector3(0, 0, -CamSpeed), nullptr),
		MovementKey(GLFW_KEY_A, Vector3(-CamSpeed, 0, 0), nullptr),
		MovementKey(GLFW_KEY_D, Vector3(CamSpeed, 0, 0), nullptr ),
		MovementKey(GLFW_KEY_LEFT, Vector3(0, -CamSpeed, 0), nullptr),
		MovementKey(GLFW_KEY_RIGHT, Vector3(0, CamSpeed, 0), nullptr)
	};
	//Vector3 m_position;
	//Vector3 m_rotation;
	//Vector3 m_FinalPosition;
	//Vector3 m_FinalRotation;
};