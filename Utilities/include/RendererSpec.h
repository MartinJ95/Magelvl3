#pragma once
#include "Vector.h"

class RendererSpec
{
public:
	virtual void AddToRenderQueue(const unsigned int RenderPass, const Vector3& Pos, const unsigned int ModelID) = 0;
	virtual void PositionCamera(const Vector3& Position, const Vector3& Rotation) = 0;
	virtual void Render(const float DeltaTime) = 0;
	virtual bool WindowShouldClose() const = 0;
	virtual void PollEvents() = 0;
};