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

#include "objectClass.h"

ObjectClass::ObjectClass(std::string package, std::string objectClass, std::set<std::string> *missingPackages)
: package(package),
  objectClass(objectClass),
  name("Object"),
  gridSize(Vector2(0.0f, 0.0f)),
  scale(1.0f) {
	if (missingPackages == nullptr) {
		// Throw an exception if package can't be loaded
		utils::getTextFile(package, "package.txt");
		utils::getTextFile(package, "objects/" + this->objectClass + ".txt");
		return;
	}

	bool exist = true;

	std::stringstream settingFile;
	try {
		settingFile.str(utils::getTextFile(package, "package.txt"));
	} catch (IOException) {
		std::cout << "Warning: package " << package << " couldn't be opened." << std::endl;
		missingPackages->insert(package);
		exist = false;
	}

	Settings *settings;
	try {
		settings = new Settings(&settingFile);
	} catch(libconfig::ParseException &e) {
		std::cout << "Warning: package " << package << "is invalid." << std::endl;
		missingPackages->insert(package);
		exist = false;
	}

	if (!exist) {
		return;
	}

	std::vector<std::string> dependencies = settings->getList<std::string>("package/dependencies");

	delete settings;

	if (dependencies.size() > 0){
		this->checkDependency(dependencies, missingPackages);
	}

	this->loadSettings();
}

ObjectClass::~ObjectClass() {

}

void ObjectClass::loadSettings() {
	std::stringstream file;
	file.str(utils::getTextFile(this->package, "objects/" + this->objectClass + ".txt"));

	std::string line;
	while (file.good()) {
		getline(file, line);

		std::string value;
		float valueFloat;
		if (line.length() == 0 || line.at(0) == '#') {
			// Ignore comment lines
		} else if (this->parseLine(line, "name", value)) {
			this->name = value;
		} else if (this->parseLine(line, "flipside", value)) {
			if (value.find(".") != std::string::npos) {
				std::vector<std::string> path = utils::splitString(value, '.');
				PHYSFS_addToSearchPath((path[0] + ".zip").c_str(), 1);
				this->flipsideImage = path[0] + "/objects/" + path[1];
			} else {
				this->flipsideImage = this->package + "/objects/" + value;
			}
		} else if (this->parseLineFloat(line, "gridwidth", valueFloat)) {
			this->gridSize.x = valueFloat;
		} else if (this->parseLineFloat(line, "gridheight", valueFloat)) {
			this->gridSize.y = valueFloat;
		} else if (this->parseLineFloat(line, "scale", valueFloat)) {
			this->scale = valueFloat;
		} else {
			std::cout << "Warning: Could not parse '" << line << "' in " << this->objectClass << ".txt" << std::endl;
		}
	}
}

std::string ObjectClass::getObjectClass() const {
	return this->objectClass;
}

std::string ObjectClass::getPackage() const {
	return this->package;
}

std::string ObjectClass::getName() const {
	return this->name;
}

Vector2 ObjectClass::getGridSize() const {
	return this->gridSize;
}

std::string ObjectClass::getFlipsideImage() const {
	return this->flipsideImage;
}

bool ObjectClass::parseLine(std::string line, std::string field, std::string &value) {
	field += ": ";

	if (line.compare(0, field.length(), field) == 0) {
		value = line.substr(field.length());
		return true;
	} else {
		return false;
	}
}

bool ObjectClass::parseLineFloat(std::string line, std::string field, float &value) {
	std::string string;
	if (this->parseLine(line, field, string)) {
		std::istringstream stream(string);
		stream >> value;
		return true;
	} else {
		return false;
	}
}

void ObjectClass::checkDependency(std::vector<std::string> dependecies, std::set<std::string> *missingPackages){
	for (auto &dependency : dependecies) {
		std::vector<std::string> newDependencies;
		std::stringstream settingFile(utils::getTextFile(dependency, "package.txt"));

		try {
			Settings newSettings(&settingFile);
			newDependencies = newSettings.getList<std::string>("package/dependencies");
		} catch (libconfig::FileIOException &e){
			std::cout << "Warning: Dependecy package " << dependency << " not found." << std::endl;
			missingPackages->insert(dependency);
		} catch (libconfig::ParseException &e) {
			std::cout << "Warning: Dependecy package " << dependency << " invalid." << std::endl;
			missingPackages->insert(dependency);
		} catch (SettingsException &e) {
			std::cout << "Warning: Dependecy package " << dependency << " invalid." << std::endl;
			missingPackages->insert(dependency);
		}

		if (newDependencies.size() > 0) {
			this->checkDependency(newDependencies, missingPackages);
		}
	}
}

float ObjectClass::getScale(void) const {
	return this->scale;
}
