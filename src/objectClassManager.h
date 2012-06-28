// Copyright 2012 Lauri Niskanen
// Copyright 2012 Antti Aalto
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

#ifndef OBJECTCLASSMANAGER_H
#define OBJECTCLASSMANAGER_H

#include <string>
#include <map>
#include <set>

#include "objectClass.h"

class ObjectClassManager {
public:
	ObjectClassManager(void);
	~ObjectClassManager(void);

	ObjectClass* getObjectClass(std::string package, std::string objectClass, std::set<std::string> *missingPackages);

	std::vector<ObjectClass*> getClassesInPackage(std::string package);

private:
	std::map<std::string, std::map<std::string, ObjectClass*>*> packages;
};

#endif
