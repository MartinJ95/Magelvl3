#pragma once

class Component
{
public:
	virtual void BeginPlay() = 0;
	virtual void Update(float DeltaTime) = 0;
	virtual void OnDestroy() = 0;
	virtual void OnInput(const int Key, const int Scancode, const int Action, const int Mods) {}
};