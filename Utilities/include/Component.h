#pragma once

class Component
{
public:
	virtual void BeginPlay() = 0;
	virtual void Update(float DeltaTime) = 0;
	virtual void LateUpdate() {};
	virtual void OnDestroy() = 0;
	virtual void OnInput(const int Key, const int Scancode, const int Action, const int Mods) {}
	void AssignEntity(const int EntityID);
protected:
	int m_entityID{0};
};