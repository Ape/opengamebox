#include "objectClass.h"

ObjectClass::ObjectClass(std::string package, std::string objectClass) {
	// TODO: Load object information from a file

	this->package = package;
	this->objectClass = objectClass;
	this->name = "Object";
	this->size = Vector2(135.0f, 189.0f);
	this->flipsideImage = this->package + "/objects/" + "card_flipside.png";
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

Vector2 ObjectClass::getSize() const {
	return this->size;
}

std::string ObjectClass::getFlipsideImage() const {
	return this->flipsideImage;
}
