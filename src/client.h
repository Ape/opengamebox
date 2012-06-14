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

#ifndef CLIENT_H
#define CLIENT_H

#include <enet/enet.h>
#include <map>

#include "color.h"

class Client {
public:
	Client(unsigned char id);
	Client(std::string nick, Color color, unsigned char id);

	static unsigned char getIdStatic(Client *client);
	static Client* getClientFromMap(std::map<unsigned char, Client*> clients, unsigned char clientId);

	unsigned char getId(void) const;
	std::string getNick(void) const;
	Color getColor(void) const;
	std::string getColorCode(void) const;
	std::string getColoredNick(void) const;
	unsigned short getPing(void) const;

	void setNick(std::string nick);
	void setPing(unsigned short);

private:
	std::string nick;
	Color color;
	unsigned char id;
	unsigned short ping;
};

#endif
