#include "objectClassManager.h"

ObjectClassManager::ObjectClassManager() {

}

ObjectClassManager::~ObjectClassManager() {
	// TODO: Release data
}

ObjectClass* ObjectClassManager::getObjectClass(std::string package, std::string objectClass) {
	std::map<std::string, std::map<std::string, ObjectClass*>*>::iterator packageIterator = this->packages.find(package);
	std::map<std::string, ObjectClass*> *selectedPackage;

	// Add a new package to the list if it was not already loaded
	if (packageIterator == this->packages.end()) {
		std::map<std::string, ObjectClass*> *newPackage = new std::map<std::string, ObjectClass*>();
		this->packages.insert(std::pair<std::string, std::map<std::string, ObjectClass*>*>(package, newPackage));
		selectedPackage = newPackage;
	} else {
		selectedPackage = packageIterator->second;
	}

	std::map<std::string, ObjectClass*>::iterator objectClassIterator = selectedPackage->find(objectClass);
	ObjectClass *selectedObjectClass;

	// Add a new objectClass to the package if it was not already loaded
	if (objectClassIterator == selectedPackage->end()) {
		ObjectClass *newObjectClass = new ObjectClass(package, objectClass);
		selectedPackage->insert(std::pair<std::string, ObjectClass*>(objectClass, newObjectClass));
		selectedObjectClass = newObjectClass;
	} else {
		selectedObjectClass = objectClassIterator->second;
	}

	return selectedObjectClass;
}
