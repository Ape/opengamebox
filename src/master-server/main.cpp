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

#include "main.h"

int main(int argc, char **argv) {
	// Get port number from command line arguments
	unsigned int port;
	if (argc >= 2) {
		std::istringstream portString(argv[1]);
		portString >> port;
	} else {
		port = net::MASTER_SERVER_PORT;
	}

	MasterServer server = MasterServer(port);
	serverPtr = &server;

	return server.run();
}

void catchSignal(int signal) {
	std::cout << std::endl << "Exiting.." << std::endl;

	serverPtr->exit();
}

MasterServer::MasterServer(unsigned int port) {
	this->address.host = ENET_HOST_ANY;
	this->address.port = port;

	this->exiting = false;

	// Add two test servers
	{
		ServerRecord *test = new ServerRecord;
		test->address = "1.1.1.1";
		test->port = net::DEFAULT_PORT;
		test->name = "Test server";
		test->players = 54;
		test->lastUpdate = 0.0;
		this->servers.push_back(test);
	}
	{
		ServerRecord *test = new ServerRecord;
		test->address = "example.com";
		test->port = 12345;
		test->name = "Card game";
		test->players = 0;
		test->lastUpdate = 0.0;
		this->servers.push_back(test);
	}
}

int MasterServer::run() {
	// Check the port
	if (this->address.port == 0) {
		std::cerr << "Illegal port number!" << std::endl;
		return EXIT_FAILURE;
	}

	// Catch SIGINT
	signal(SIGINT, catchSignal);

	// Initialize ENnet
	if (enet_initialize() != 0) {
		std::cerr << "Could not initialize network components!" << std::endl;
		return EXIT_FAILURE;
	}

	this->connection = enet_host_create(&this->address, MAX_CONNECTIONS, 1, 0, 0);

	if (this->connection == nullptr) {
		std::cerr << "Could not bind to " << net::AddressToString(this->address) << "!" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Master server listening on " << net::AddressToString(this->address) << "." << std::endl;

	// Enter the main loop
	this->mainLoop();

	// Exit the server
	this->dispose();
	return EXIT_SUCCESS;
}

void MasterServer::mainLoop() {
	while (! this->exiting) {
		this->networkEvents();
	}
}

void MasterServer::exit() {
	this->exiting = true;
}

void MasterServer::dispose() {
	enet_host_destroy(this->connection);
	enet_deinitialize();
}

void MasterServer::networkEvents() {
	ENetEvent event;

	// Wait up to 100 milliseconds for an event.
	while (enet_host_service(this->connection, &event, 100) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT: {
				std::cout << "A new connection from " << net::AddressToString(event.peer->address) << "." << std::endl;

				break;
			}

			case ENET_EVENT_TYPE_RECEIVE: {
				this->receivePacket(event);

				enet_packet_destroy(event.packet);

				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT: {
				std::cout << "Client disconnected!" << std::endl;

				break;
			}

			case ENET_EVENT_TYPE_NONE: {
				break;
			}
		}
	}
}

void MasterServer::receivePacket(ENetEvent event) {
	Packet packet(event.packet);

	try {
		switch (packet.readHeader()) {
			case Packet::Header::MS_QUERY: {
				std::cout << "Sending the server list!" << std::endl;

				Packet reply(event.peer);
				reply.writeHeader(Packet::Header::MS_QUERY);

				for (auto& server : this->servers) {
					reply.writeString(server->address);
					reply.writeShort(server->port);
					reply.writeString(server->name);
					reply.writeShort(server->players);
				}

				reply.send();

				break;
			}

			case Packet::Header::MS_REGISTER: {
				std::cout << "A server wants to appear on the list!" << std::endl;

				// TODO: Register the server

				break;
			}

			case Packet::Header::MS_UPDATE: {
				std::cout << "A server wants to refresh its information!" << std::endl;

				// TODO: Update the server information

				break;
			}

			case Packet::Header::HANDSHAKE: {
				// This is not a game server

				Packet reply(event.peer);
				reply.writeHeader(Packet::Header::MS_QUERY);
				reply.send();

				break;
			}

                        default: {
                                throw PacketException("Invalid packet header.");
                        }
                }
        } catch (PacketException &e) {
                std::cout << "Debug: Received an invalid packet from" 
                          << net::AddressToString(event.peer->address)
                          << ": \"" << e.what() << "\"" << std::endl;
        }
}
