#ifndef OBJECT_H
#define OBJECT_H

#include <map>
#include <vector>
#include <list>
#include <set>

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
	std::list<Object*> getObjectsAbove(std::set<Object*> &visited);
	bool checkIfUnder(std::vector<Object*> objectOrder);

	void setLocation(Vector2 location);
	void select(bool selected);
	void flip(void);

	void draw(IRenderer *renderer) const;

private:
	std::string objectId;
	std::string name;
	unsigned int id;
	Vector2 size;
	Vector2 location;
	bool flipped;
	std::string image;
	std::string backside;

	bool selected;
	std::list<Object*> objectsAbove;
};

#endif
