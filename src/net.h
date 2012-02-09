#ifndef NET_H
#define NET_H

#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <enet/enet.h>

#include "vector2.h"
#include "object.h"

class Object;

namespace net{
	// Define basic connection parameters
	const unsigned int DEFAULT_PORT   = 13355;
	const unsigned int CHANNELS       = 2;
	const unsigned char MAX_CLIENTS   = 32;
	const float MAX_FLOAT             = 10000.0f;
	const double STREAM_INTERVAL       = 5000.0f;

	// Client control packets
	const unsigned char PACKET_HANDSHAKE  = 0x01;
	const unsigned char PACKET_NICK_TAKEN = 0x02;
	const unsigned char PACKET_JOIN       = 0x03;
	const unsigned char PACKET_LEAVE      = 0x04;

	// Command packets
	const unsigned char PACKET_CHAT     = 0x20; // Send a chat message
	const unsigned char PACKET_CREATE   = 0x21; // Create new objects
	const unsigned char PACKET_SELECT   = 0x22; // Select objects and prevent others from moving them
	const unsigned char PACKET_REMOVE   = 0x24; // Remove objects
	const unsigned char PACKET_MOVE     = 0x25; // Move objects
	const unsigned char PACKET_FLIP     = 0x26; // Flip objects
	const unsigned char PACKET_OWN      = 0x27; // Make objects private

	// Stream packets
	const unsigned char PACKET_PINGS = 0xE0;

	// Structure for storing client information
	struct Client{
		unsigned char id;

		Client(ENetPeer *peer); // Constructor for server
		Client(std::string nick); // Constructor for client

		bool joined;
		std::string nick;
		Object *player;
		ENetPeer *peer; // Only needed on servers
		unsigned short int ping; // Only needed on clients
	};

	bool isNickTaken(std::map<unsigned char, Client*> clients, std::string nick);

	void sendCommand(ENetHost *connection, const char *data, size_t length, bool isReliable=true);
	void sendCommand(ENetPeer *peer, const char *data, size_t length);

	template <class T>
	unsigned char firstUnusedKey(std::map<unsigned char, T*> map);

	template <class T>
	unsigned short firstUnusedKey(std::map<unsigned short, T*> map);

	void intToBytes(unsigned char *bytes, unsigned int value);
	unsigned int bytesToInt(unsigned char *bytes);

	void shortToBytes(unsigned char *bytes, unsigned short value);
	unsigned int bytesToShort(unsigned char *bytes);

	void floatToBytes(unsigned char *bytes, float value);
	float bytesToFloat(unsigned char *bytes);
	Vector2 bytesToVector2(unsigned char *bytes);

	void dataAppendInt(std::string &data, unsigned int value);
	void dataAppendShort(std::string &data, unsigned short value);
	void dataAppendVector2(std::string &data, Vector2 value);

	net::Client* clientIdToClient(std::map<unsigned char, Client*> clients, unsigned char clientId);
	unsigned char clientToClientId(Client *client);

	std::string IPIntegerToString(unsigned int ip);
	std::string AddressToString(ENetAddress address);

	void removeObject(std::vector<Object*> &objectOrder, Object* object);
}

template <class T>
unsigned char net::firstUnusedKey(std::map<unsigned char, T*> map) {
	for (unsigned char i = 0; i < 255; ++i) {
		if (map.count(i) == 0) {
			return i;
		}
	}

	return 255;
}

template <class T>
unsigned short net::firstUnusedKey(std::map<unsigned short, T*> map) {
	for (unsigned short i = 0; i < 65535; ++i) {
		if (map.count(i) == 0) {
			return i;
		}
	}

	return 65535;
}

#endif
