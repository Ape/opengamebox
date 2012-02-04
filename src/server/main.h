#ifndef MAIN_H
#define MAIN_H

#include <map>
#include <string>
#include <iostream>

#include <enet/enet.h>

#include "../net.h"
#include "../vector2.h"
#include "../object.h"

class Server{
public:
	Server(unsigned int port);

	int run(void);

private:
	ENetAddress address;
	ENetHost *connection;

	std::map<unsigned char, net::Client*> clients;
	std::map<unsigned short, Object*> objects;

	bool exiting;

	unsigned int frame;
	unsigned int lastFrameTime;

	void mainLoop(void);
	void dispose(void);

	void networkEvents(void);
	void receivePacket(ENetEvent event);
	void sendStream(void);
};

#endif
