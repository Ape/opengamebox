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

#include "serverClient.h"

ServerClient::ServerClient(ENetPeer *peer, unsigned char id)
: Client(id),
  peer(peer),
  joined(false),
  admin(false) {}

ServerClient* ServerClient::getClientWithId(std::map<unsigned char, ServerClient*> clients, unsigned char clientId) {
	if (clients.count(clientId) != 0) {
		return clients.find(clientId)->second;
	} else {
		return nullptr;
	}
}

bool ServerClient::isJoined() const {
	return this->joined;
}

ENetPeer* ServerClient::getPeer() const {
	return this->peer;
}

bool ServerClient::isAdmin() const {
	return this->admin;
}

void ServerClient::join() {
	this->joined = true;
}

void ServerClient::grantAdmin() {
	this->admin = true;
}
