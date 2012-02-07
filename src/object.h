#ifndef OBJECT_H
#define OBJECT_H

#include <map>
#include <vector>
#include <list>
#include <set>

#include "net.h"
#include "irenderer.h"
#include "vector2.h"

namespace net{
	struct Client;
}

class Object{
public:
	Object(std::string objectId, unsigned int id, Vector2 location);

	std::string getObjectId(void) const;
	std::string getName(void) const;
	unsigned short getId(void) const;
	Vector2 getLocation(void) const;
	Vector2 getSize(void) const;
	bool testLocation(Vector2 location) const;
	bool testCollision(Object *object, bool second = false);

	bool isUnder(void) const;
	Vector2 getStackDelta(void) const;
	std::list<Object*> getObjectsAbove(std::set<Object*> &visited);
	bool checkIfUnder(std::vector<Object*> objectOrder);
	bool isSelectedBy(net::Client *client);
	net::Client* getSelected(void);
	bool isOwnedBy(net::Client *client);
	net::Client* getOwner(void);
	bool isFlipped(void) const;

	void setLocation(Vector2 location);
	void select(net::Client *client);
	void own(net::Client *client);
	void flip(void);
	void setFlipped(bool flipped);

	void draw(IRenderer *renderer, net::Client *localClient) const;

private:
	std::string objectId;
	std::string name;
	unsigned short id;
	Vector2 size;
	Vector2 location;
	bool flipped;
	std::string image;
	std::string backside;
	Vector2 stackDelta;

	net::Client *selected;
	net::Client *owner;
	std::list<Object*> objectsAbove;
};

#endif
