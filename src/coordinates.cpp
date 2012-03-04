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
