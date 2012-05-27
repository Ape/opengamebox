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

#include "packet.h"

Packet::Packet(unsigned char *data)
: readCursor(0) {
	this->data = std::string(reinterpret_cast<char*>(data));
}

void Packet::setReliable(bool isReliable) {
	this->isReliable = isReliable;
}

void Packet::writeHeader(Packet::Header value) {
	this->writeByte(static_cast<unsigned char>(value));
}

void Packet::writeByte(unsigned char value) {
	this->data.push_back(value);
}

void Packet::writeShort(unsigned short value) {
	
}

void Packet::writeInt(unsigned int value) {
	
}

void Packet::writeString(std::string value) {
	
}

void Packet::writeFloat(float value) {
	
}

void Packet::writeVector2(Vector2 value) {
	
}

Packet::Header Packet::readHeader() {
	return static_cast<Packet::Header>(this->readByte());
}

unsigned char Packet::readByte() {
	unsigned char value = this->data.at(this->readCursor);
	++this->readCursor;
	return value;
}

void Packet::send(){
	int flags = 0;

	if (this->isReliable) {
		flags |= ENET_PACKET_FLAG_RELIABLE;
	}

	ENetPacket *packet = enet_packet_create(this->data.c_str(), this->data.size(), flags);

	if (this->connection != nullptr) {
		enet_host_broadcast(this->connection, 0, packet);
	} else if (this->peer != nullptr) {
		enet_peer_send(this->peer, 0, packet);
	}
}
