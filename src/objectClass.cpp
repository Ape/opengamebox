#include "objectClass.h"

ObjectClass::ObjectClass(std::string package, std::string objectClass) {
	// TODO: Load object information from a file

	this->package = package;
	this->objectClass = objectClass;
	this->name = "Object";
	this->gridSize = Vector2(0.0f, 0.0f);

	std::ifstream file;
	file.open("data/" + this->package + "/objects/" + this->objectClass + ".txt", std::ios::in);

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
			this->flipsideImage = this->package + "/objects/" + value;
		} else if (this->parseLineFloat(line, "gridwidth", valueFloat)) {
			this->gridSize.x = valueFloat;
		} else if (this->parseLineFloat(line, "gridheight", valueFloat)) {
			this->gridSize.y = valueFloat;
		} else {
			std::cout << "Warning: Could not parse '" << line << "' in " << this->objectClass << ".txt" << std::endl;
		}
	}

	file.close();
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
