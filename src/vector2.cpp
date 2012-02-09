#include "vector2.h"

Vector2::Vector2(const float x, const float y){
	this->x = x;
	this->y = y;
}

Vector2::~Vector2(){}

std::ostream& operator<<(std::ostream &output, const Vector2 &vector){
	output << "(" <<  vector.x << ", " << vector.y << ")";
	return output;
}

void Vector2::operator+=(const Vector2 &vector){
	this->x += vector.x;
	this->y += vector.y;
}

void Vector2::operator-=(const Vector2 &vector){
	this->x -= vector.x;
	this->y -= vector.y;
}

void Vector2::operator*=(const float &scalar){
	this->x *= scalar;
	this->y *= scalar;
}

void Vector2::operator/=(const float &scalar){
	this->x /= scalar;
	this->y /= scalar;
}

void Vector2::operator=(const Vector2 &vector){
	this->x = vector.x;
	this->y = vector.y;
}

bool Vector2::operator== (const Vector2 &vector) const{
	return vector.x == this->x && vector.y == this->y;
}

bool Vector2::operator!= (const Vector2 &vector) const{
	return !(this->cpy()==vector);
}

float Vector2::angle() const{
	return atan2(this->y, this->x);
}

float Vector2::len2() const{
	return this->x * this->x + this->y * this->y;
}

float Vector2::len() const{
	return sqrt(len2());
}

float Vector2::dst2(Vector2 vector) const{
	return (this->cpy() - vector).len2();
}

float Vector2::dst(Vector2 vector) const{
	return sqrt(dst2(vector));
}

Vector2 Vector2::nor() const{
	return this->cpy() / this->len();
}

Vector2 Vector2::abs() const{
	return Vector2(fabs(this->x), fabs(this->y));
}
