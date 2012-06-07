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

#include "settings.h"

Settings::Settings(std::string file) {
	// Read the file. If there is an error, report it.
	try {
		this->config.readFile(file.c_str());
	} catch (const libconfig::FileIOException &e) {
		std::cout << "Error: Couldn't read the config file!" << std::endl;
		throw e;
	} catch (const libconfig::ParseException &e) {
		std::cout << "Error: Couldn't parse the config file at " << e.getFile() << ":" << e.getLine() << " - " << e.getError() << std::endl;
		throw e;
	}
}

Settings::Settings(std::stringstream *stream) {
	try {
		this->config.readString(stream->str());
	} catch (const libconfig::ParseException &e) {
		std::cout << "Error: Couldn't parse the config file at " << e.getFile() << ":" << e.getLine() << " - " << e.getError() << std::endl;
		throw e;
	}
}
