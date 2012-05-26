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
#include <sstream>
#include <iomanip>

#include "color.h"

class Client {
public:
	Client(ENetPeer *peer, unsigned char id); // Constructor for server
	Client(std::string nick, Color color, unsigned char id); // Constructor for client

	unsigned char getId(void);
	std::string getNick(void);
	Color getColor(void);
	std::string getColorCode(void);
	std::string getColoredNick(void);
	bool isJoined(void);
	void join(void);
	void setNick(std::string nick);
	ENetPeer* getPeer(void);

	// TODO: Should be private with a getter function
	unsigned short int ping; // Only needed on clients

private:
	std::string nick;
	Color color;
	bool joined; // Only needed on server
	unsigned char id;
	ENetPeer *peer; // Only needed on server
};

#endif
