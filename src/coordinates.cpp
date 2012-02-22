#include "coordinates.h"

Coordinates::Coordinates(const int x, const int y) {
	this->x = x;
	this->y = y;
}

void Coordinates::operator+=(const Coordinates &coordinates) {
	this->x += coordinates.x;
	this->y += coordinates.y;
}

void Coordinates::operator-=(const Coordinates &coordinates) {
	this->x -= coordinates.x;
	this->y -= coordinates.y;
}

Coordinates& Coordinates::operator=(const Coordinates &coordinates) {
	this->x = coordinates.x;
	this->y = coordinates.y;

	return *this;
}

bool Coordinates::operator== (const Coordinates &coordinates) const {
	return coordinates.x == this->x && coordinates.y == this->y;
}

bool Coordinates::operator!= (const Coordinates &coordinates) const {
	return !(this->cpy() == coordinates);
}
