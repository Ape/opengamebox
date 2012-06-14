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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <libconfig.h++>

#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iostream>

class SettingsException : public std::runtime_error {
public:
	SettingsException(std::string message)
	: std::runtime_error(message) {}
};

// TODO: Meaningful default values

class Settings {
public:
	Settings(std::string file);
	Settings(std::stringstream *stream);

	template<class T>
	T getValue(std::string path) const {
		T value;

		if (!this->config.lookupValue(path, value)) {
			throw SettingsException(std::string("Couldn't get a value for ") + path);
		}

		return value;
	}

	template<class T> void setValue(std::string path, T value) {
		libconfig::Setting &setting = this->config.lookup(path, value);
        setting = value;
	}

	template<class T> std::vector<T> getList(std::string path) const {
		libconfig::Setting &setting = this->config.lookup(path);
		std::vector<T> list;

		if (!setting.isList()) {
			throw SettingsException(std::string("Setting ") + path + " is not a list.");
		}

		for (int i = 0; i < setting.getLength(); ++i) {
			T value;
			std::ostringstream key;
			key << path << ".[" << i << "]";

			if (!this->config.lookupValue(key.str(), value)) {
				throw SettingsException(std::string("Couldn't get a list for ") + path);
			}

			list.push_back(value);
		}

		return list;
	}

private:
	libconfig::Config config;
};

#endif
