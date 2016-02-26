#include "Vec2.h"

Vec2 operator +(const Vec2& fst, const Vec2& scnd)
{
	return Vec2(fst.x + scnd.x, fst.y + scnd.y);
}

Vec2 operator -(const Vec2& fst, const Vec2& scnd)
{
	return Vec2(fst.x - scnd.x, fst.y - scnd.y);
}

Vec2 operator *(const Vec2& vect, float scalar)
{
	return Vec2(vect.x*scalar, vect.y*scalar);
}

float operator *(const Vec2& vect1, const Vec2& vect2)
{
	return vect1.x*vect2.x + vect1.y*vect2.y;
}