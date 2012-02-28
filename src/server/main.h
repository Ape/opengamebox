#ifndef MAIN_H
#define MAIN_H

#include <cstdlib>
#include <csignal>
#include <map>
#include <string>
#include <iostream>
#include <random>
#include <functional>

#include <enet/enet.h>

#include "../net.h"
#include "../utils.h"
#include "../vector2.h"
#include "../object.h"

class Server;

Server *serverPtr;

int main(int argc, char **argv);
void catchSignal(int signal);

class Server {
public:
	Server(unsigned int port);

	int run(void);
	void exit(void);

private:
	ENetAddress address;
	ENetHost *connection;

	std::map<unsigned char, net::Client*> clients;
	std::map<unsigned short, Object*> objects;

	bool exiting;

	double lastStreamTime;

	std::mt19937 randomGenerator;

	// TODO: Remove this and use objectClasses dynamically
    ObjectClass *objectClass;

	void mainLoop(void);
	void dispose(void);

	void networkEvents(void);
	void receivePacket(ENetEvent event);
	void sendStream(void);
};

#endif
