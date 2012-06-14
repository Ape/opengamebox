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

#include "net.h"

// Check if a nick is already used
bool net::isNickTaken(std::map<unsigned char, ServerClient*> clients, std::string nick) {
	for (std::map<unsigned char, ServerClient*>::iterator client = clients.begin(); client != clients.end(); ++client) {
		if (client->second->isJoined() && client->second->getNick().compare(nick) == 0) {
			return true;
		}
	}

	return false;
}

// Broadcast a command
void net::sendCommand(ENetHost *connection, const char *data, size_t length, bool isReliable) {
	int flags = 0;

	if (isReliable) {
		flags |= ENET_PACKET_FLAG_RELIABLE;
	}

	ENetPacket *packet = enet_packet_create(data, length, flags);
	enet_host_broadcast(connection, 0, packet);
}

// Send a command to a single peer
void net::sendCommand(ENetPeer *peer, const char *data, size_t length) {
	ENetPacket *packet = enet_packet_create(data, length, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

// Convert an unsigned int to a byte array (4-byte)
void net::intToBytes(unsigned char* bytes, unsigned int value) {
	bytes[0] = value & 0xFF;
	bytes[1] = (value >> 8) & 0xFF;
	bytes[2] = (value >> 16) & 0xFF;
	bytes[3] = (value >> 24) & 0xFF;
}

// Convert a byte array created with intToBytes() back to an unsigned int
unsigned int net::bytesToInt(unsigned char* bytes) {
	return (bytes[3] << 24) + (bytes[2] << 16) + (bytes[1] << 8) + bytes[0];
}

// Same as intToBytes, but with an unsigned short (2-byte)
void net::shortToBytes(unsigned char* bytes, unsigned short value) {
	bytes[0] = value & 0xFF;
	bytes[1] = (value >> 8) & 0xFF;
}

// Convert a byte array created with shortToBytes() back to an unsigned short
unsigned int net::bytesToShort(unsigned char* bytes) {
	return (bytes[1] << 8) + bytes[0];
}

// Convert a float in range [-MAX_FLOAT, MAX_FLOAT] to a byte array (4-byte)
void net::floatToBytes(unsigned char* bytes, float value) {
	float normalized = (value + MAX_FLOAT) / (2 * MAX_FLOAT);
	if (normalized > 0.99999f) {
		normalized = 0.99999f;
	}

	unsigned int out = normalized * 4294967295u;

	net::intToBytes(bytes, out);
}

// Convert a byte array created with floatToBytes() back to a float
float net::bytesToFloat(unsigned char* bytes) {
	return net::bytesToInt(bytes) * (2.0f * MAX_FLOAT) / 4294967296.0f - MAX_FLOAT;
}

Vector2 net::bytesToVector2(unsigned char* bytes) {
	return Vector2(net::bytesToFloat(bytes), net::bytesToFloat(bytes + 4));
}

// Appends the data with an unsigned int coded with intToBytes
void net::dataAppendInt(std::string &data, unsigned int value) {
	unsigned char bytes[4];

	net::intToBytes(bytes, value);
	data.append(reinterpret_cast<char*>(bytes), 4);
}

// Appends the data with an unsigned short coded with shortToBytes
void net::dataAppendShort(std::string &data, unsigned short value) {
	unsigned char bytes[2];

	net::shortToBytes(bytes, value);
	data.append(reinterpret_cast<char*>(bytes), 2);
}

// Appends the data with a Vector2 coded with floatToBytes
void net::dataAppendVector2(std::string &data, Vector2 value) {
	unsigned char bytes[4];

	net::floatToBytes(bytes, value.x);
	data.append(reinterpret_cast<char*>(bytes), 4);

	net::floatToBytes(bytes, value.y);
	data.append(reinterpret_cast<char*>(bytes), 4);
}

// Convert an integer format IP address to a string representation
std::string net::IPIntegerToString(unsigned int ip) {
	unsigned char bytes[4];
	bytes[0] = ip & 0xFF;
	bytes[1] = (ip >> 8) & 0xFF;
	bytes[2] = (ip >> 16) & 0xFF;
	bytes[3] = (ip >> 24) & 0xFF;

	std::ostringstream string;
	string << static_cast<int>(bytes[0]) << "." << static_cast<int>(bytes[1]) << "." << static_cast<int>(bytes[2]) << "."<< static_cast<int>(bytes[3]);

	return string.str();
}

// Convert an ENetAddress to a string representation
std::string net::AddressToString(ENetAddress address) {
	std::ostringstream string;
	string << net::IPIntegerToString(address.host) << ":" << address.port;

	return string.str();
}

void net::removeObject(std::vector<Object*> &objectOrder, Object* object) {
	for (std::vector<Object*>::size_type i = 0; i < objectOrder.size(); ++i) {
		if (objectOrder.at(i)->getId() == object->getId()) {
			objectOrder.erase(objectOrder.begin() + i);

			break;
		}
	}
}
