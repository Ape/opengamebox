// Copyright 2012 Lauri Niskanen
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

#include "vector2.h"

Vector2::Vector2(const float x, const float y) {
	this->x = x;
	this->y = y;
}

std::ostream& operator<<(std::ostream &output, const Vector2 &vector) {
	output << "(" <<  vector.x << ", " << vector.y << ")";
	return output;
}

void Vector2::operator+=(const Vector2 &vector) {
	this->x += vector.x;
	this->y += vector.y;
}

void Vector2::operator-=(const Vector2 &vector) {
	this->x -= vector.x;
	this->y -= vector.y;
}

void Vector2::operator*=(const float &scalar) {
	this->x *= scalar;
	this->y *= scalar;
}

void Vector2::operator/=(const float &scalar) {
	this->x /= scalar;
	this->y /= scalar;
}

Vector2& Vector2::operator=(const Vector2 &vector) {
	this->x = vector.x;
	this->y = vector.y;

	return *this;
}

bool Vector2::operator==(const Vector2 &vector) const {
	return vector.x == this->x && vector.y == this->y;
}

bool Vector2::operator!=(const Vector2 &vector) const {
	return !(this->cpy() == vector);
}

float Vector2::angle() const {
	return atan2(this->y, this->x);
}

Vector2 Vector2::rotate(float angle) {
	float length = this->len();
	float newAngle = this->angle() + angle;
	this->x = cos(newAngle) * length;
	this->y = sin(newAngle) * length;
	return *this;
}

float Vector2::len2() const {
	return this->x * this->x + this->y * this->y;
}

float Vector2::len() const {
	return sqrt(len2());
}

float Vector2::dst2(Vector2 vector) const {
	return (this->cpy() - vector).len2();
}

float Vector2::dst(Vector2 vector) const {
	return sqrt(dst2(vector));
}

Vector2 Vector2::nor() const {
	return this->cpy() / this->len();
}

Vector2 Vector2::abs() const {
	return Vector2(fabs(this->x), fabs(this->y));
}
