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
	this->size = Vector2(135, 189);
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

bool Object::testLocation(Vector2 location) const{
	if (location.x > this->location.x - this->size.x/2.0f && location.x < this->location.x + this->size.x/2.0f
	    && location.y > this->location.y - this->size.y/2.0f && location.y < this->location.y + this->size.y/2.0f){
		return true;
	}else{
		return false;
	}
}

bool Object::testCollision(Object *object) const{
	if (object->testLocation(this->location - this->size/2.0f) || object->testLocation(this->location + this->size/2.0f)
	        || object->testLocation(Vector2(this->location.x - this->size.x/2.0f, this->location.y + this->size.y/2.0f))
	        || object->testLocation(Vector2(this->location.x + this->size.x/2.0f, this->location.y - this->size.y/2.0f))){
		return true;
	}else{
		return false;
	}
}

bool Object::isUnder(std::vector<Object*> objectOrder) const{
	bool thisFound = false;
	for (auto& object : objectOrder){
		if (! thisFound){
			if (object->getId() == this->getId()){
				thisFound = true;
			}
		}else if (this->testCollision(object)){
			return true;
		}
	}

	return false;
}

void Object::setLocation(Vector2 location){
	this->location = location;
}

void Object::draw(IRenderer *renderer) const{
	renderer->drawBitmap(this->image, Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), this->location - this->size/2.0f, this->size);
}
