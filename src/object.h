#ifndef OBJECT_H
#define OBJECT_H

#include <map>
#include <vector>
#include <list>

#include "net.h"
#include "irenderer.h"
#include "vector2.h"

class Object{
public:
	Object(std::string objectId, unsigned int id, Vector2 location);

	std::string getObjectId(void) const;
	std::string getName(void) const;
	unsigned int getId(void) const;
	Vector2 getLocation(void) const;
	bool testLocation(Vector2 location) const;
	bool testCollision(Object *object) const;

	bool isUnder(void) const;
	std::list<Object*> getObjectsAbove();
	bool checkIfUnder(std::vector<Object*> objectOrder);

	void setLocation(Vector2 location);

	void draw(IRenderer *renderer) const;
	void drawSelection(IRenderer *renderer) const;

private:
	std::string objectId;
	std::string name;
	unsigned int id;
	std::string image;
	Vector2 size;
	Vector2 location;
	std::list<Object*> objectsAbove;
};

#endif
