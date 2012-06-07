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

#ifndef UTILS_H
#define UTILS_H

#include <physfs.h>

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>

namespace utils {
	const float PI = 3.141592653589f;

	std::vector<std::string> splitString(std::string, char delimeter);
	std::string toString(int i);
	unsigned int hexStringToInt(std::string string);

	template <class T>
	unsigned char firstUnusedKey(std::map<unsigned char, T*> map);

	template <class T>
	unsigned short firstUnusedKey(std::map<unsigned short, T*> map);

	std::string getTextFile(std::string package, std::string path);
}

template <class T>
unsigned char utils::firstUnusedKey(std::map<unsigned char, T*> map) {
	for (unsigned char i = 0; i < 255; ++i) {
		if (map.count(i) == 0) {
			return i;
		}
	}

	return 255;
}

template <class T>
unsigned short utils::firstUnusedKey(std::map<unsigned short, T*> map) {
	for (unsigned short i = 0; i < 65535; ++i) {
		if (map.count(i) == 0) {
			return i;
		}
	}

	return 65535;
}

#endif
