#include "object.h"

Object::Object(std::string objectId, unsigned int id, Vector2 location) {
	this->objectId = objectId;
	this->id       = id;
	this->location = location;
	this->flipped  = false;
	this->selected = nullptr;
	this->owner    = nullptr;

	// TODO: Load this object data from the filesystem
	this->size = Vector2(135.0f, 189.0f);

	if (this->objectId == "card_7c") {
		this->name = "Seven of Clubs";
	}else if (this->objectId == "card_Kh") {
		this->name = "King of Hearts";
	}else if (this->objectId == "card_As") {
		this->name = "Ace of Spades";
	}else if (this->objectId == "chessboard") {
		this->name = "Chessboard";
		this->size = Vector2(512.0f, 512.0f);
	}else if (this->objectId == "piece_red") {
		this->name = "Red piece";
		this->size = Vector2(50.0f, 50.0f);
	}else if (this->objectId == "piece_blue") {
		this->name = "Blue piece";
		this->size = Vector2(50.0f, 50.0f);
	}else {
		this->size = Vector2(300.0f, 300.0f);
		this->name = this->objectId;
	}

	this->image = this->objectId + ".png";
	this->backside = "card_backside.png";
	this->stackDelta = Vector2(4.0f, 0.0f);

	this->animationTime = 0.0f;
}

std::string Object::getObjectId() const {
	return this->objectId;
}

std::string Object::getName() const {
	return this->name;
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

Vector2 Object::getSize() const {
	return this->size;
}

bool Object::testLocation(Vector2 location) const {
	if (location.x >= this->getTargetLocation().x - this->size.x/2.0f && location.x <= this->getTargetLocation().x + this->size.x/2.0f
	    && location.y >= this->getTargetLocation().y - this->size.y/2.0f && location.y <= this->getTargetLocation().y + this->size.y/2.0f) {
		return true;
	} else {
		return false;
	}
}

bool Object::testCollision(const Object *object, bool second) const {
	if (object->testLocation(this->getTargetLocation() - this->size/2.0f) || object->testLocation(this->getTargetLocation() + this->size/2.0f)
	        || object->testLocation(Vector2(this->getTargetLocation().x - this->size.x/2.0f, this->getTargetLocation().y + this->size.y/2.0f))
	        || object->testLocation(Vector2(this->getTargetLocation().x + this->size.x/2.0f, this->getTargetLocation().y - this->size.y/2.0f))
	        || (! second && object->testCollision(this, true))) {
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

		for (auto& object : this->objectsAbove) {
			if (visited.count(object) == 0) {
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

bool Object::checkIfUnder(std::vector<Object*> objectOrder) {
	this->objectsAbove.clear();
	bool thisFound = false;
	for (auto& object : objectOrder) {
		if (! thisFound) {
			if (object->getId() == this->getId()) {
				thisFound = true;
			}
		}else if (this->testCollision(object)) {
			this->objectsAbove.push_back(object);
		}
	}

	if (this->objectsAbove.empty()) {
		return false;
	} else {
		return true;
	}
}

bool Object::isSelectedBy(net::Client *client) const {
	return this->selected == client;
}

net::Client* Object::getSelected() {
	return this->selected;
}

bool Object::isOwnedBy(net::Client *client) const {
	return this->owner == client;
}

net::Client* Object::getOwner() {
	return this->owner;
}

bool Object::isFlipped() const {
	return this->flipped;
}

void Object::setLocation(Vector2 location) {
	this->location = location;
}

void Object::select(net::Client* client) {
	this->selected = client;
}

void Object::own(net::Client* client) {
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

void Object::draw(IRenderer *renderer, net::Client *localClient) const {
	std::string image;
	if (! this->flipped) {
		image = this->image;
	} else {
		image = this->backside;
	}

	if (this->selected != nullptr) {
		Vector2 pointA = this->location - this->size/2.0f - Vector2(1.0f, 1.0f);
		Vector2 pointB = this->location + this->size/2.0f + Vector2(1.0f, 1.0f);

		renderer->drawRectangle(pointA, pointB, Color(renderer, this->selected->id), 2.0f);
	}

	Color tint(1.0f, 1.0f, 1.0f, 1.0f);
	if (this->owner == localClient) {
		renderer->drawText(localClient->nick, Color(renderer, localClient->id), this->location + Vector2(0.0f, -this->size.y / 2.0f - 18.0f),
		                   IRenderer::Alignment::CENTER);
		tint.alpha = 0.75f;
	}
	
	if (this->isUnder()) {
		tint.red = 0.75f;
		tint.green = 0.75f;
		tint.blue = 0.75f;
	}
	
	if (this->owner != nullptr || this->owner != localClient) {
		renderer->drawBitmapTinted(image, Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), this->location - this->size/2.0f, this->size, tint);
	}
}
