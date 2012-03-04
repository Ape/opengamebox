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

#ifndef OBJECTCLASS_H
#define OBJECTCLASS_H

#include <string>
#include <sstream>
#include <fstream>

#include "vector2.h"

class ObjectClass {
public:
	ObjectClass(std::string package, std::string objectClass);

	std::string getObjectClass(void) const;
	std::string getPackage(void) const;
	std::string getName(void) const;
	Vector2 getGridSize(void) const;
	std::string getFlipsideImage(void) const;

private:
	std::string package;
	std::string objectClass;
	std::string name;
	Vector2 gridSize;
	std::string flipsideImage;

	bool parseLine(std::string line, std::string field, std::string &value);
	bool parseLineFloat(std::string line, std::string field, float &value);
};

#endif
