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
#include <map>
#include <string>
#include <iostream>
#include <algorithm>
#include <random>
#include <functional>

#include <enet/enet.h>

#include "../net.h"
#include "../packet.h"
#include "../utils.h"
#include "../vector2.h"
#include "../objectClassManager.h"
#include "../object.h"
#include "../settings.h"

class Server;

int main(int argc, char **argv);

class Server {
public:
	Server(unsigned int port);
	~Server(void);

	static Server *serverPtr;
	static void catchSignal(int signal);

	int run(void);
	void exit(void);

private:
	ENetAddress address;
	ENetHost *connection;

	Settings *settings;

	ObjectClassManager objectClassManager;

	std::map<unsigned char, Client*> clients;
	std::map<unsigned short, Object*> objects;
	std::set<std::string> missingPackages;

	bool exiting;

	double lastStreamTime;

	std::mt19937 randomGenerator;

	void mainLoop(void);
	void dispose(void);

	void networkEvents(void);
	void receivePacket(ENetEvent event);
	void sendStream(void);
};

#endif
