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

#ifndef MAIN_H
#define MAIN_H

#include <cstdlib>
#include <csignal>
#include <string>
#include <iostream>

#include <enet/enet.h>

#include "../utils.h"
#include "../packet.h"
#include "../net.h"
#include "../settings.h"

const unsigned int MAX_CONNECTIONS = 255;

class MasterServer;

int main(int argc, char **argv);

class MasterServer {
public:
	MasterServer(unsigned int port);
	~MasterServer(void);

	static MasterServer *serverPtr;
	static void catchSignal(int signal);

	int run(void);
	void exit(void);

private:
	ENetAddress address;
	ENetHost *connection;

	Settings *settings;

	struct ServerRecord {
		std::string address;    // Hostname or IP-address
		unsigned short port;    // The connection port
		std::string name;       // The name of the server
		unsigned short players; // The amount of players on the server
		ENetPeer *peer;         // The connected server
	};

	std::vector<ServerRecord*> servers;

	bool exiting;

	void mainLoop(void);
	void dispose(void);

	void networkEvents(void);
	void receivePacket(ENetEvent event);
};

#endif
