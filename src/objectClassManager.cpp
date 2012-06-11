// Copyright 2012 Lauri Niskanen
//
// This file is part of OpenGamebox.
//
// OpenGamebox is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenGamebox is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenGamebox.  If not, see <http://www.gnu.org/licenses/>.

#include "objectClassManager.h"

ObjectClassManager::ObjectClassManager() {

}

ObjectClassManager::~ObjectClassManager() {
	for (auto& package : this->packages) {
		for (auto& objectClass : *package.second) {
			delete objectClass.second;
		}

		delete package.second;
	}
}

ObjectClass* ObjectClassManager::getObjectClass(std::string package, std::string objectClass, std::set<std::string> *missingPackages) {
	auto packageIterator = this->packages.find(package);
	std::map<std::string, ObjectClass*> *selectedPackage;

	// Add a new package to the list if it was not already loaded
	if (packageIterator == this->packages.end()) {
		std::map<std::string, ObjectClass*> *newPackage = new std::map<std::string, ObjectClass*>();
		this->packages.insert(std::pair<std::string, std::map<std::string, ObjectClass*>*>(package, newPackage));
		selectedPackage = newPackage;
	} else {
		selectedPackage = packageIterator->second;
	}

	auto objectClassIterator = selectedPackage->find(objectClass);
	ObjectClass *selectedObjectClass;

	// Add a new objectClass to the package if it was not already loaded
	if (objectClassIterator == selectedPackage->end()) {
		ObjectClass *newObjectClass = new ObjectClass(package, objectClass, missingPackages);
		selectedPackage->insert(std::pair<std::string, ObjectClass*>(objectClass, newObjectClass));
		selectedObjectClass = newObjectClass;
	} else {
		selectedObjectClass = objectClassIterator->second;
	}

	return selectedObjectClass;
}
