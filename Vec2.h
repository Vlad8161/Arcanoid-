/*#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif*/

#ifndef _VEC2_H
#define _VEC2_H

#include <math.h>

class Vec2
{
public:
	float x;
	float y;

	Vec2(float _x = 0, float _y = 0) :
		x(_x), y(_y) {}

	friend Vec2 operator +(const Vec2& fst, const Vec2& scnd);
	friend Vec2 operator -(const Vec2& fst, const Vec2& scnd);
	friend Vec2 operator *(const Vec2& vect, float scalar);
	friend float operator *(const Vec2& vect1, const Vec2& vect2);

	float module()
	{
		return sqrt(x*x + y*y);
	}

	Vec2 normalize()
	{
		float mod = module();
		return Vec2(x / mod, y / mod);
	}

	Vec2 normal()
	{
		return Vec2(-y, x);
	}

	Vec2 negate()
	{
		return Vec2(-x, -y);
	}

	Vec2 reflect(Vec2 mirror)
	{
		float m2 = mirror * mirror;
		float am = (*this) * mirror;
		Vec2 subtracted = mirror * (2 * am / m2);
		return (*this) - subtracted;
	}
};

Vec2 operator +(const Vec2& fst, const Vec2& scnd);
Vec2 operator -(const Vec2& fst, const Vec2& scnd);
Vec2 operator *(const Vec2& vect, float scalar);
float operator *(const Vec2& vect1, const Vec2& vect2);

#endif