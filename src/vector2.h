#ifndef VECTOR2_H
#define VECTOR2_H

#include <cmath>
#include <iostream>

class Vector2{
	friend std::ostream& operator<<(std::ostream &output, const Vector2 &vector);

public:
	float x;
	float y;

	Vector2(const float x = 0, const float y = 0);
	~Vector2(void);

	Vector2 cpy(void) const;
	Vector2 operator+(const Vector2 &vector) const;
	Vector2 operator-(const Vector2 &vector) const;
	Vector2 operator*(const float &scalar) const;
	Vector2 operator/(const float &scalar) const;
	Vector2 operator*(const Vector2 &vector) const;
	Vector2 operator/(const Vector2 &vector) const;
	Vector2 operator%(const Vector2 &vector) const;
	void operator+=(const Vector2 &vector);
	void operator-=(const Vector2 &vector);
	void operator*=(const float &scalar);
	void operator/=(const float &scalar);
	void operator=(const Vector2 &vector);

	bool operator== (const Vector2 &vector1);
	bool operator!= (const Vector2 &vector1);

	float angle(void) const;

	float len2(void) const;
	float len(void) const;

	Vector2 nor(void) const;

	float dst(Vector2 vector) const;
	float dst2(Vector2 vector) const;
};

inline Vector2 Vector2::cpy() const{
	return Vector2(this->x, this->y);
}

inline Vector2 Vector2::operator+(const Vector2 &vector) const{
	return Vector2(this->x + vector.x, this->y + vector.y);
}

inline Vector2 Vector2::operator-(const Vector2 &vector) const{
	return Vector2(this->x - vector.x, this->y - vector.y);
}

inline Vector2 Vector2::operator*(const float &scalar) const{
	return Vector2(scalar * this->x, scalar * this->y);
}

inline Vector2 Vector2::operator/(const float &scalar) const{
	return Vector2(this->x / scalar, this->y / scalar);
}

inline Vector2 Vector2::operator*(const Vector2 &vector) const{
	return Vector2(this->x * vector.x, this->y * vector.y);
}

inline Vector2 Vector2::operator/(const Vector2 &vector) const{
	return Vector2(this->x / vector.x, this->y / vector.y);
}

inline Vector2 Vector2::operator%(const Vector2 &vector) const{
	return Vector2(fmod(this->x, vector.x), fmod(this->y, vector.y));
}

#endif
