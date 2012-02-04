#include "object.h"

Object::Object(std::string objectId, unsigned int id, Vector2 location){
	this->objectId = objectId;
	this->id       = id;
	this->location = location;
	this->flipped  = false;
	this->selected = false;

	// TODO: Load this object data from the filesystem
	if (this->objectId == "card_7c"){
		this->name = "Seven of Clubs";
	}else if (this->objectId == "card_Kh"){
		this->name = "King of Hearts";
	}else if (this->objectId == "card_As"){
		this->name = "Ace of Spades";
	}

	this->size = Vector2(135, 189);
	this->image = this->objectId + ".png";
	this->backside = "card_backside.png";
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

bool Object::isUnder() const{
	return ! this->objectsAbove.empty();
}

std::list<Object*> Object::getObjectsAbove(){
	std::list<Object*> allAbove;
	allAbove.push_back(this);

	for (auto& object : this->objectsAbove){
		std::list<Object*> objects = object->getObjectsAbove();

		allAbove.splice(allAbove.end(), objects);
	}

	return allAbove;
}

bool Object::checkIfUnder(std::vector<Object*> objectOrder){
	this->objectsAbove.clear();
	bool thisFound = false;
	for (auto& object : objectOrder){
		if (! thisFound){
			if (object->getId() == this->getId()){
				thisFound = true;
			}
		}else if (this->testCollision(object)){
			this->objectsAbove.push_back(object);
		}
	}

	if (this->objectsAbove.empty()){
		return false;
	}else{
		return true;
	}
}

void Object::setLocation(Vector2 location){
	this->location = location;
}

void Object::select(bool selected){
	this->selected = selected;
}

void Object::flip(){
	this->flipped = ! this->flipped;
}

void Object::draw(IRenderer *renderer) const{
	std::string image;
	if (! this->flipped){
		image = this->image;
	}else{
		image = this->backside;
	}

	if (this->selected){
		renderer->drawBitmapTinted(image, Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), this->location - this->size/2.0f, this->size, 0.25f, 1.0f, 0.25f, 1.0f);
	}else if (!this->isUnder()){
		renderer->drawBitmap(image, Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), this->location - this->size/2.0f, this->size);
	}else{
		renderer->drawBitmapTinted(image, Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), this->location - this->size/2.0f, this->size, 0.75f, 0.75f, 0.75f, 1.0f);
	}
}
