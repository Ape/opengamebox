// Copyright 2012 Lauri Niskanen
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

#ifndef PACKET_H
#define PACKET_H

#include <string>
#include <sstream>
#include <stdexcept>

#include <enet/enet.h>

#include "vector2.h"

class PacketException : public std::runtime_error {
public:
	PacketException(std::string message)
	: std::runtime_error(message) {}
};

class Packet {
public:
	// Define packet headers
	enum class Header : unsigned char {
		// Client control
		HANDSHAKE  = 0x01, // Initialize connection
		NICK_TAKEN = 0x02, // Inform about a reserved nick name
		JOIN       = 0x03, // Inform about a joining client
		LEAVE      = 0x04, // Inform about a leaving client
		LOGIN      = 0x05, // Login as an admin

		// Object commands
		CREATE  = 0x20, // Create new objects
		SELECT  = 0x21, // Select objects and prevent others from moving them
		REMOVE  = 0x22, // Remove objects
		MOVE    = 0x23, // Move objects
		FLIP    = 0x24, // Flip objects
		OWN     = 0x25, // Make objects private
		SHUFFLE = 0x26, // Shuffle objects
		ROTATE  = 0x27, // Rotate objects

		// Other commands
		CHAT = 0x40, // Send a chat message
		ROLL = 0x41, // Roll a die on the server


		//Files and packages
		PACKAGE_MISSING = 0x60, // Package not found
		FILE_TRANSFER   = 0x61, // Transfer missing file

		// Master server communication
		MS_QUERY    = 0xC0, // Request the server list
		MS_REGISTER = 0xC1, // Register to the server list
		MS_UPDATE   = 0xC2, // Update registered information

		// Streamed packets
		PINGS = 0xE0 // Broadcast ping information
	};

	// Broadcast
	Packet(ENetHost *connection, bool isReliable = true)
	: connection(connection),
	  peer(nullptr),
	  isReliable(isReliable) {}

	// Unicast
	Packet(ENetPeer *peer, bool isReliable = true)
	: connection(nullptr),
	  peer(peer),
	  isReliable(isReliable) {}

	// Received packet
	Packet(ENetPacket *packet);

	void setReliable(bool isReliable);

	void writeHeader(Header value);
	void writeByte(unsigned char value);
	void writeShort(unsigned short value);
	void writeInt(unsigned int value);
	void writeString(std::string value);
	void writeFloat(float value);
	void writeVector2(Vector2 value);

	Header readHeader(void);
	unsigned char readByte(void);
	unsigned short readShort(void);
	unsigned int readInt(void);
	std::string readString(void);
	float readFloat(void);
	Vector2 readVector2(void);

	unsigned int remainingBytes(void) const;
	bool eof(void) const;

	void send(void);

private:
	std::string data;
	size_t readCursor;

	ENetHost *connection;
	ENetPeer *peer;
	bool isReliable;
};

#endif
