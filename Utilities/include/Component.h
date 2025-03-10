#pragma once
#include "vector"
#include <iostream>
#include <array>

enum class ComponentDecompositionTypes
{
	eInt,
	eFloat3,
	eBool,
	ePad
};

class Component
{
public:
	virtual void BeginPlay() = 0;
	virtual void Update(float DeltaTime) = 0;
	virtual void LateUpdate() {};
	virtual void CleanComponent() {};
	virtual void OnDestroy() = 0;
	virtual void OnInput(const int Key, const int Scancode, const int Action, const int Mods) {}
	virtual void SafetyChecks(void* Other)
	{

	}
	virtual void GetDecompositions(std::vector<std::pair<ComponentDecompositionTypes, std::string>>& InVec) const 
	{
		for (int i = 0; i < 4; i++)
		{
			InVec.emplace_back(std::make_pair(ComponentDecompositionTypes::ePad, ""));
		}
		InVec.emplace_back( std::make_pair(ComponentDecompositionTypes::eInt, "Entity")); 
	}
	void AssignEntity(const int EntityID);
	bool UsesDynamicMemory() const 
	{
		return m_usesDynamicMemory; 
	}
protected:
	int m_entityID{0};
	bool m_usesDynamicMemory{ false };
	std::array<bool, 3> m_pads;
};