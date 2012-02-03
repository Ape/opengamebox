#ifndef OBJECT_H
#define OBJECT_H

#include <map>
#include <vector>

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

	void setLocation(Vector2 location);

	virtual void draw(IRenderer *renderer) const;

private:
	std::string objectId;
	std::string name;
	std::string image;
	unsigned int id;
	Vector2 location;
};

#endif
