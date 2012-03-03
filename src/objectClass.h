#ifndef OBJECTCLASS_H
#define OBJECTCLASS_H

#include <string>
#include <sstream>
#include <fstream>

#include "vector2.h"

class ObjectClass {
public:
	ObjectClass(std::string package, std::string objectClass);

	std::string getObjectClass(void) const;
	std::string getPackage(void) const;
	std::string getName(void) const;
	Vector2 getGridSize(void) const;
	std::string getFlipsideImage(void) const;

private:
	std::string package;
	std::string objectClass;
	std::string name;
	Vector2 gridSize;
	std::string flipsideImage;

	bool parseLine(std::string line, std::string field, std::string &value);
	bool parseLineFloat(std::string line, std::string field, float &value);
};

#endif
