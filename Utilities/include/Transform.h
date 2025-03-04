#pragma once
#include "Component.h"
#include "Vector.h"

class Transform : public Component
{
public:
	Transform() : m_position(), m_rotation(), m_lastPosition(), m_lastRotation()
	{}
	void BeginPlay() override final;
	void Update(float DeltaTime) override final;
	void OnDestroy() override final;
	void LateUpdate() override final;
	void SetPosition(const Vector3& Position) { m_position = Position; }
	void SetRotation(const Vector3& Rotation) { m_rotation = Rotation; }
	void Translate(const Vector3& Position) { m_position += Position; }
	
	void GetDecompositions(std::vector<std::pair<ComponentDecompositionTypes, std::string>>& InVec) const override final
	{
		
		InVec.emplace_back(std::make_pair(ComponentDecompositionTypes::eFloat3, "lastrotation"));
		InVec.emplace_back(std::make_pair(ComponentDecompositionTypes::eFloat3, "lastposition"));
		InVec.emplace_back(std::make_pair(ComponentDecompositionTypes::eFloat3, "rotation"));
		InVec.emplace_back(std::make_pair(ComponentDecompositionTypes::eFloat3, "position"));

		Component::GetDecompositions(InVec);
		
		//return types;
		
	}
	
	const Vector3& GetPosition() const { return m_lastPosition; }
	const Vector3& GetRotation() const { return m_lastRotation; }
	const Vector3& GetCurrentMove() const { return m_position - m_lastPosition; }
	const Vector3& GetCurrentRotate() const { return m_rotation - m_lastRotation; }
	Vector3& GetPositionRef() { return m_position; }
	Vector3& GetRotationRef() { return m_rotation; }
protected:
	Vector3 m_position;
	Vector3 m_rotation;
	Vector3 m_lastPosition;
	Vector3 m_lastRotation;
};