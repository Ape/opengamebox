#ifndef OBJECTCLASS_H
#define OBJECTCLASS_H

#include <string>

#include "vector2.h"

class ObjectClass {
public:
	ObjectClass(std::string package, std::string objectClass);

	std::string getObjectClass(void) const;
	std::string getPackage(void) const;
	std::string getName(void) const;
	Vector2 getSize(void) const;
	std::string getFlipsideImage(void) const;

private:
	std::string package;
	std::string objectClass;
	std::string name;
	Vector2 size;
	std::string flipsideImage;
};

#endif
