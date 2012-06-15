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

#include <sstream>
#include <iomanip>

Client::Client(unsigned char id)
: id(id) {}

Client::Client(std::string nick, Color color, unsigned char id)
: nick(nick),
  color(color),
  id(id),
  ping(static_cast<unsigned short>(66535)) {}

Client* Client::getClientWithId(std::map<unsigned char, Client*> clients, unsigned char clientId) {
	if (clients.count(clientId) != 0) {
		return clients.find(clientId)->second;
	} else {
		return nullptr;
	}
}

Client* Client::getClientWithNick(std::map<unsigned char, Client*> clients, std::string nick) {
	for (auto &client : clients) {
		if (client.second->getNick() == nick) {
			return client.second;
		}
	}

	return nullptr;
}

unsigned char Client::getIdStatic(Client *client) {
	if (client == nullptr) {
		return 255;
	} else {
		return client->getId();
	}
}

unsigned char Client::getId() const {
	return this->id;
}

std::string Client::getNick() const {
	return this->nick;
}

Color Client::getColor() const {
	return this->color;
}

std::string Client::getColorCode() const {
	std::ostringstream colorCode;
	colorCode << "^#" << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(this->id);

	return colorCode.str();
}

std::string Client::getColoredNick() const {
	return this->getColorCode() + this->getNick() + "^fff";
}

unsigned short Client::getPing() const {
	return this->ping;
}

void Client::setNick(std::string nick) {
	this->nick = nick;
}

void Client::setPing(unsigned short ping) {
	this->ping = ping;
}
