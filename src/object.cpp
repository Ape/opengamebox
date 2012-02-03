#include "object.h"

Object::Object(unsigned int id, Vector2 location){
	this->id       = id;
	this->location = location;
}

unsigned int Object::getId(){
	return this->id;
}

Vector2 Object::getLocation(){
	return this->location;
}

void Object::setLocation(Vector2 location){
	this->location = location;
}

void Object::draw(IRenderer *renderer){
	
}
