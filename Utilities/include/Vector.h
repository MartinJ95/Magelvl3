#pragma once

struct Vector3
{
public:
	Vector3() : x(0), y(0), z(0) {}
	Vector3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
public:
	float Length() const 
	{
		return x * x + y * y + z * z;
	}
	Vector3 Normalised() const 
	{
		float length = Length();

		return Vector3(
			x / length,
			y / length,
			z/length
		);
	}
	void operator=(const Vector3& rhs) { x = rhs.x; y = rhs.y; z = rhs.z; };
	Vector3 operator*(const float& s) const { return Vector3(x * s, y * s, z * s); }
	Vector3 operator+(const Vector3& rhs) const { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }
	Vector3 operator-(const Vector3& rhs) const { return Vector3{ x - rhs.x, y - rhs.y, z - rhs.z }; }
	void operator*=(const float s) { x *= s; y *= s; z *= s; };
	void operator+=(const Vector3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; };
	bool operator==(const Vector3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; };
	bool operator!=(const Vector3& rhs) const { return !(*this == rhs); }
public:
	float x, y, z;
};

static const Vector3 UpVector{ 0.f,1.f,0.f };
static const Vector3 DownVector{ 0.f, -1.f, 0 };
static const Vector3 ForwardVector{ 0, 0, 1 };
static const Vector3 BackVector{ 0.f, 0.f, -1 };
static const Vector3 RightVector{ 1, 0, 0 };
static const Vector3 LeftVector{ -1, 0, 0 };
