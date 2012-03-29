#ifndef CLIENT_H
#define CLIENT_H

#include <enet/enet.h>

#include "color.h"

class Client {
public:
	Client(ENetPeer *peer, unsigned char id); // Constructor for server
	Client(std::string nick, Color color, unsigned char id); // Constructor for client
	~Client();

	unsigned char getId();
	Color getColor();
	std::string getNick();
	std::string getColoredNick();
	bool isJoined();
	void setJoined();

	void setNick(std::string nick);

	ENetPeer* getPeer();

	unsigned short int ping; // Only needed on clients

private:
	std::string nick;

	Color color;

	bool joined; //only needed on server
	unsigned char id;

	ENetPeer *peer; // Only needed on servers

};

#endif
