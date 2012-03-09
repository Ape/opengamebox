#ifndef CLIENT_H
#define CLIENT_H

#include <enet/enet.h>

#include "color.h"

class Client {
public:
	Client(ENetPeer *peer, unsigned char id); // Constructor for server
	Client(std::string nick, Color color); // Constructor for client
	~Client();

	unsigned char getId();
	Color getColor();
	std::string getNick();
	std::string getColoredNick();

	void setNick(std::string nick);

	bool joined;
	unsigned char id;
	unsigned short int ping; // Only needed on clients
	ENetPeer *peer; // Only needed on servers

private:
	std::string nick;

	Color color;
};

#endif
