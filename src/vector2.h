// Copyright 2012 Lauri Niskanen
// Copyright 2012 Antti Aalto
//
// This file is part of OpenGamebox.
//
// OpenGamebox is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenGamebox is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenGamebox.  If not, see <http://www.gnu.org/licenses/>.

#ifndef VECTOR2_H
#define VECTOR2_H

#include <cmath>
#include <iostream>

class Vector2{
	friend std::ostream& operator<<(std::ostream &output, const Vector2 &vector);
	friend Vector2 operator*(float scalar, const Vector2 &vector);
	friend Vector2 operator/(float scalar, const Vector2 &vector);

public:
	float x;
	float y;

	Vector2(const float x = 0, const float y = 0);

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
	Vector2& operator=(const Vector2 &vector);

	bool operator==(const Vector2 &vector1) const;
	bool operator!=(const Vector2 &vector1) const;

	float angle(void) const;
	Vector2 rotate(float angle);

	float len2(void) const;
	float len(void) const;

	Vector2 nor(void) const;

	float dst2(Vector2 vector) const;
	float dst(Vector2 vector) const;

	Vector2 abs(void) const;
};

inline Vector2 Vector2::cpy() const {
	return Vector2(this->x, this->y);
}

inline Vector2 Vector2::operator+(const Vector2 &vector) const {
	return Vector2(this->x + vector.x, this->y + vector.y);
}

inline Vector2 Vector2::operator-(const Vector2 &vector) const {
	return Vector2(this->x - vector.x, this->y - vector.y);
}

inline Vector2 Vector2::operator*(const float &scalar) const {
	return Vector2(scalar * this->x, scalar * this->y);
}

inline Vector2 operator*(float scalar, const Vector2 &vector) {
	return Vector2(scalar * vector.x, scalar * vector.y);
}

inline Vector2 Vector2::operator/(const float &scalar) const {
	return Vector2(this->x / scalar, this->y / scalar);
}

inline Vector2 operator/(float scalar, const Vector2 &vector) {
	return Vector2(vector.x / scalar, vector.y / scalar);
}

inline Vector2 Vector2::operator*(const Vector2 &vector) const {
	return Vector2(this->x * vector.x, this->y * vector.y);
}

inline Vector2 Vector2::operator/(const Vector2 &vector) const {
	return Vector2(this->x / vector.x, this->y / vector.y);
}

inline Vector2 Vector2::operator%(const Vector2 &vector) const {
	return Vector2(fmod(this->x, vector.x), fmod(this->y, vector.y));
}

#endif
