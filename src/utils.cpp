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

#include "utils.h"

std::vector<std::string> utils::splitString(std::string string, char delimeter) {
	std::vector<std::string> items;
	std::string item;
	std::stringstream stream(string);

	while (std::getline(stream, item, delimeter)) {
		items.push_back(item);
	}

	return items;
}

std::string utils::toString(int i) {
	std::ostringstream stream;
	stream << i;
	return stream.str();
}

unsigned int utils::hexStringToInt(std::string string) {
	unsigned int i;
	std::istringstream stream(string);
	stream >> std::hex >> i;
	return i;
}

std::string utils::getTextFile(std::string package, std::string path){
	PHYSFS_addToSearchPath(("data/" + package + ".zip").c_str(), 1);
	std::string str;
	if (PHYSFS_exists((package + "/" + path).c_str())) {
		PHYSFS_file* file = PHYSFS_openRead((package + "/" + path).c_str());
		char *buf;
		buf = new char[PHYSFS_fileLength(file) + 1];
		buf[PHYSFS_fileLength(file)] = 0x00;
		int readbytes = PHYSFS_read(file, buf, 1, PHYSFS_fileLength(file));
		if (readbytes != PHYSFS_fileLength(file)) {
			std::cout<< "failed to read data/" << package << ".zip/" << path<<std::endl;
			return "";
		}
		str = std::string(buf);
		PHYSFS_close(file);
	}
	return str;
}
