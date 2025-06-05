#pragma once
#include "Vector.h"

struct Circle
{
	Vector3 Position;
	float Radius;
};

struct Square
{
	Vector3 Origin;
	float MinX;
	float MinY;
	float MaxX;
	float MaxY;
};

struct Ray
{
	Vector3 Origin;
	Vector3 Direction;
};

struct Triangle
{
	Vector3 Point0;
	Vector3 Point1;
	Vector3 Point2;
};

struct Cylinder
{
	Vector3 Origin;
	float Height;
	float Radius;
};