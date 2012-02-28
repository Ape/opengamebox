#ifndef OBJECTCLASSMANAGER_H
#define OBJECTCLASSMANAGER_H

#include <string>
#include <map>

#include "objectClass.h"

class ObjectClassManager {
public:
	ObjectClassManager(void);
	~ObjectClassManager(void);

	ObjectClass* getObjectClass(std::string package, std::string objectClass);

private:
	std::map<std::string, std::map<std::string, ObjectClass*>*> packages;
};

#endif
