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

#ifndef COORDINATES_H
#define COORDINATES_H

class Coordinates{
public:
	int x;
	int y;

	Coordinates(const int x = 0, const int y = 0);

	Coordinates cpy(void) const;

	Coordinates operator+(const Coordinates &coordinates) const;
	Coordinates operator-(const Coordinates &coordinates) const;
	Coordinates operator%(const Coordinates &coordinates) const;

	void operator+=(const Coordinates &coordinates);
	void operator-=(const Coordinates &coordinates);
	Coordinates& operator=(const Coordinates &coordinates);

	bool operator== (const Coordinates &coordinates) const;
	bool operator!= (const Coordinates &coordinates) const;
};

inline Coordinates Coordinates::cpy() const {
	return Coordinates(this->x, this->y);
}

inline Coordinates Coordinates::operator+(const Coordinates &coordinates) const {
	return Coordinates(this->x + coordinates.x, this->y + coordinates.y);
}

inline Coordinates Coordinates::operator-(const Coordinates &coordinates) const {
	return Coordinates(this->x - coordinates.x, this->y - coordinates.y);
}

inline Coordinates Coordinates::operator%(const Coordinates &coordinates) const {
	return Coordinates(this->x % coordinates.x, this->y % coordinates.y);
}

#endif
