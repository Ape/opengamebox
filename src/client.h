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
	~Client(void);

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
