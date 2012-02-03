#ifndef OBJECT_H
#define OBJECT_H

#include <map>
#include <vector>

#include "net.h"
#include "irenderer.h"
#include "vector2.h"

class Object{
protected:
	unsigned int id;
	Vector2 location;

public:
	Object(unsigned int id, Vector2 location);

	unsigned int getId(void);
	Vector2 getLocation(void);

	void setLocation(Vector2 location);

	virtual void draw(IRenderer *renderer);
};

#endif
