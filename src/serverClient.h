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

#ifndef SERVERCLIENT_H
#define SERVERCLIENT_H

#include <enet/enet.h>

#include "client.h"

class ServerClient : public Client {
public:
	ServerClient(ENetPeer *peer, unsigned char id);

	static ServerClient* getClientWithId(std::map<unsigned char, ServerClient*> clients, unsigned char clientId);

	bool isJoined(void) const;
	ENetPeer* getPeer(void) const;
	bool isAdmin(void) const;

	void join(void);
	void grantAdmin(void);

private:
	ENetPeer *peer;
	bool joined;
	bool admin;
};

#endif
