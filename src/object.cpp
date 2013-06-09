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

#include "object.h"

Object::Object(ObjectClass *objectClass, std::string objectId, unsigned int id, Vector2 location) {
	this->objectClass = objectClass;
	this->objectId    = objectId;
	this->id          = id;
	this->location    = location;
	this->flipped     = false;
	this->selected    = nullptr;
	this->owner       = nullptr;
	this->rotation    = 0.0f;

	this->image = this->objectClass->getPackage() + "/objects/" + this->objectClass->getObjectClass() + "/" + this->objectId;
	this->stackDelta = Vector2(4.0f, 0.0f);

	this->animationTime = 0.0f;

	//To make decks work when not yet initialised
	this->size = Vector2(8.0f, 8.0f);
}

void Object::initForClient(IRenderer *renderer) {
	Coordinates textureSize = renderer->getTextureSize(this->image);
	this->size = Vector2(textureSize.x, textureSize.y);
}

ObjectClass* Object::getObjectClass() const {
	return this->objectClass;
}

std::string Object::getObjectId() const {
	return this->objectId;
}

std::string Object::getFullId() const {
	return this->objectClass->getPackage() + "." + this->objectClass->getObjectClass() + "." + this->objectId;
}

unsigned short Object::getId() const {
	return this->id;
}

Vector2 Object::getLocation() const {
	return this->location;
}

Vector2 Object::getTargetLocation() const {
	if (this->animationTime > 0.0f) {
		return this->animationTarget;
	} else {
		return this->location;
	}
}

std::string Object::getName() const {
	return this->objectClass->getName();
}

Vector2 Object::getSize() const {
	return this->size;
}

Vector2 Object::getGridSize() const {
	Vector2 gridSize = this->objectClass->getGridSize();

	if (gridSize.x == 0.0f) {
		gridSize.x = this->size.x;
	}

	if (gridSize.y == 0.0f) {
		gridSize.y = this->size.y;
	}

	return gridSize;
}

float Object::getRotation() const {
	return this->rotation;
}

bool Object::testLocation(Vector2 location) const {
	const Vector2 targetLocation = this->getTargetLocation();
	const Vector2 rotatedSize = (this->getSize() / 2.0f).rotate(this->rotation);
	const float rotatedSizeAbsX = std::abs(rotatedSize.x);
	const float rotatedSizeAbsY = std::abs(rotatedSize.y);

	if (location.x >= targetLocation.x - rotatedSizeAbsX
			&& location.x <= targetLocation.x + rotatedSizeAbsX
			&& location.y >= targetLocation.y - rotatedSizeAbsY
			&& location.y <= targetLocation.y + rotatedSizeAbsY) {
		return true;
	} else {
		return false;
	}
}

bool Object::testCollision(const Object *object, bool second) const {
	const Vector2 thisRotatedSize = (this->getSize() / 2.0f).rotate(this->rotation);
	const Vector2 objectRotatedSize = (object->getSize() / 2.0f).rotate(object->rotation);
	const Vector2 addedRotatedSizes = Vector2(std::abs(thisRotatedSize.x) + std::abs(objectRotatedSize.x), std::abs(thisRotatedSize.y) + std::abs(objectRotatedSize.y));
	const Vector2 subtractionVector = this->location - object->location;
	if (addedRotatedSizes.x >= std::abs(subtractionVector.x) and addedRotatedSizes.y >= std::abs(subtractionVector.y)) {
		return true;
	} else {
		return false;
	}
}

bool Object::isUnder() const {
	return ! this->objectsAbove.empty();
}

Vector2 Object::getStackDelta() const {
	return this->stackDelta;
}

std::list<Object*> Object::getObjectsAbove(std::set<Object*> &visited) {
	std::list<Object*> allAbove;
	visited.insert(this);

	bool fail = false;

	if (this->selected == nullptr) {
		allAbove.push_back(this);

		for (auto &object : this->objectsAbove) {
			if (visited.count(object) == 0 && object->owner == this->owner) {
				std::list<Object*> objects = object->getObjectsAbove(visited);

				if (objects.size() == 1 && objects.front() == nullptr) {
					fail = true;
				} else {
					allAbove.splice(allAbove.end(), objects);
				}
			}
		}
	} else {
		fail = true;
	}

	if (fail) {
		allAbove.clear();
		allAbove.push_back(nullptr);
	}

	return allAbove;
}

bool Object::checkIfUnder(Object* other) {
	this->objectsAbove.remove(other);
	if(other != this) {
		if(other->owner == this->owner && this->testCollision(other)) {
			this->objectsAbove.push_back(other);
			return true;
		}
	}
	return false;
}

void Object::notUnder(Object* object) {
	this->objectsAbove.remove(object);
}

void Object::clearUnder() {
	this->objectsAbove.clear();
}

bool Object::isSelectedBy(Client* client) const {
	return this->selected == client;
}

Client* Object::getSelected() {
	return this->selected;
}

bool Object::isOwnedBy(Client *client) const {
	return this->owner == client;
}

Client* Object::getOwner() {
	return this->owner;
}

bool Object::isFlipped() const {
	return this->flipped;
}

void Object::setLocation(Vector2 location) {
	this->location = location;
}

void Object::select(Client* client) {
	this->selected = client;
}

void Object::setOwner(Client* client) {
	if (client == nullptr){
		this->flipped = true;
	} else {
		this->flipped = false;
	}

	this->owner = client;
}

void Object::flip() {
	this->flipped = ! this->flipped;
}

void Object::setFlipped(bool flipped) {
	this->flipped = flipped;
}

void Object::setAnimation(Vector2 target, float time) {
	this->animationTarget = target;
	this->animationTime = time;
}

void Object::animate(double deltaTime) {
	if (this->animationTime > 0.0f) {
		const float distance = this->location.dst(this->animationTarget);
		const float speed = distance / this->animationTime;

		if (distance < speed * deltaTime || distance <= 0.002f) {
			this->location = this->animationTarget;
			this->animationTime = 0.0f;
		} else {
			this->location += speed * deltaTime * (this->animationTarget - this->location).nor();
			this->animationTime -= deltaTime;
		}
	}
}

void Object::draw(IRenderer *renderer, Client *localClient) const {
	std::string image;
	if ((! this->flipped && (this->owner == nullptr || this->owner == localClient)) || this->objectClass->getFlipsideImage().empty()) {
		image = this->image;
	} else {
		image = this->objectClass->getFlipsideImage();
	}

	Color tint(1.0f, 1.0f, 1.0f, 1.0f);

	if (this->isUnder()) {
		tint.red = 0.75f;
		tint.green = 0.75f;
		tint.blue = 0.75f;
	}

	if (this->selected != nullptr) {
		const std::vector<Vector2> corners = this->getCorners(true, 1.0f);
		renderer->drawRectangle(corners.at(0), corners.at(1), this->selected->getColor(), 2.0f, IRenderer::Transformation::CAMERA);
	}

	renderer->drawBitmapTinted(image, this->location, this->getSize(), tint, this->rotation);

	if (this->owner != nullptr) {
		const std::vector<Vector2> corners = this->getCorners(true);

		Color indicatorColor = this->owner->getColor();
		float alpha = 0.2f;
		indicatorColor.red *= alpha;
		indicatorColor.green *= alpha;
		indicatorColor.blue *= alpha;
		indicatorColor.alpha = alpha;
		renderer->drawRectangleFilled(corners.at(0), corners.at(1), indicatorColor, IRenderer::Transformation::CAMERA);
	}
}

void Object::rotate(float angle) {
	this->rotation += angle;
}

const std::vector<Vector2> Object::getCorners(bool onlyDiagonal, float margin) const {
	const Vector2 rotatedSize1 = (Vector2(this->size.x + margin, this->size.y + margin) / 2.0f).rotate(this->rotation);
	Vector2 rotatedSize2;

	if (!onlyDiagonal) {
		rotatedSize2 = (Vector2(this->size.x + margin, -(this->size.y + margin)) / 2.0f).rotate(this->rotation);
	}

	std::vector<Vector2> corners;
	corners.push_back(this->getLocation() + rotatedSize1);

	if (!onlyDiagonal) {
		corners.push_back(this->getLocation() + rotatedSize2);
	}

	corners.push_back(this->getLocation() - rotatedSize1);

	if (!onlyDiagonal) {
		corners.push_back(this->getLocation() - rotatedSize2);
	}

	return corners;
}
