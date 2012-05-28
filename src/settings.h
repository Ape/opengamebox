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

#include <libconfig.h++>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

class Settings
{
public:
	Settings(std::string file);

	template<class T>
	T getValue(std::string path) const
	{
		T value;
		if(!this->cfg.lookupValue(path, value))
		{
			std::cout<<"Getting value failed."<<std::endl;
		}
		return value;
	}

	template<class T>
	void setValue(std::string path, T value)
	{
		libconfig::Setting &setting = this->cfg.lookup(path, value);
        setting = value;
	}

	template<class T>
	std::vector<T> getList(std::string path) const
	{
		libconfig::Setting &setting = cfg.lookup(path);
		std::vector<T> tempVector;
		if(!setting.isList())
		{
			std::cout<<"Setting is not a list."<<std::endl;
		}
		for(int i = 0; setting.getLength(); i++)
		{
			T tempValue;
			std::stringstream ss;
			ss<<path<<"["<<i<<"]";
			if(!this->cfg.lookupValue(ss.str(),tempValue))
			{
				std::cout<<"Getting list failed."<<std::endl;
				break;
			}
			tempVector.push_back(tempValue);
		}
		return tempVector;
	}

private:
	libconfig::Config cfg;
};
