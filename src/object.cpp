#include "object.h"

Object::Object(std::string objectId, unsigned int id, Vector2 location){
	this->objectId = objectId;
	this->id       = id;
	this->location = location;

	// TODO: Load this object data from the filesystem
	if (this->objectId == "card_7c"){
		this->name = "Seven of Clubs";
	}else if (this->objectId == "card_Kh"){
		this->name = "King of Hearts";
	}

	this->image = this->objectId + ".png";
}

std::string Object::getObjectId() const{
	return this->objectId;
}

std::string Object::getName() const{
	return this->name;
}

unsigned int Object::getId() const{
	return this->id;
}

Vector2 Object::getLocation() const{
	return this->location;
}

void Object::setLocation(Vector2 location){
	this->location = location;
}

void Object::draw(IRenderer *renderer) const{
	renderer->drawBitmap(this->image, Coordinates(0, 0), renderer->getTextureSize(this->image), Coordinates(0, 0), Coordinates(1, 1));
}
