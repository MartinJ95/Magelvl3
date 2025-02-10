#pragma once

struct Vector3
{
public:
	Vector3() : x(0), y(0), z(0) {}
	Vector3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
public:
	void operator=(const Vector3& rhs) { x = rhs.x; y = rhs.y; z = rhs.z; };
	Vector3 operator*(const float& s) { return Vector3(x * s, y * s, z * s); }
	Vector3 operator+(const Vector3& rhs) { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }
	void operator*=(const float s) { x *= s; y *= s; z *= s; };
	void operator+=(const Vector3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; };
	bool operator==(const Vector3& rhs) { return x == rhs.x && y == rhs.y && z == rhs.z; };
	bool operator!=(const Vector3& rhs) { return !(*this == rhs); }
public:
	float x, y, z;
};