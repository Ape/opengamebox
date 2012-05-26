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

#include "client.h"

Client::Client(ENetPeer *peer, unsigned char id) {
	this->peer = peer;
	this->joined = false;
	this->id = id;
}

Client::Client(std::string nick, Color color, unsigned char id) {
	this->joined = true;
	this->nick = nick;
	this->color = color;
	this->peer = nullptr;
	this->id = id;
}

unsigned char Client::getId(){
	return this->id;
}

std::string Client::getNick() {
	return this->nick;
}

Color Client::getColor() {
	return this->color;
}

std::string Client::getColorCode() {
	std::ostringstream colorCode;
	colorCode << "^#" << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(this->id);

	return colorCode.str();
}

std::string Client::getColoredNick() {
	return this->getColorCode() + this->getNick() + "^fff";
}

bool Client::isJoined() {
	return this->joined;
}

void Client::join() {
	this->joined = true;
}

void Client::setNick(std::string nick) {
	this->nick = nick;
}

ENetPeer* Client::getPeer() {
	return this->peer;
}
