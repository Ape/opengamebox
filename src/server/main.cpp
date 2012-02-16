#include "main.h"

int main(int argc, char **argv) {
	// Get port number from command line arguments
	unsigned int port;
	if (argc >= 2) {
		std::istringstream portString(argv[1]);
		portString >> port;
	} else {
		port = net::DEFAULT_PORT;
	}

	Server server = Server(port);
	serverPtr = &server;

	return server.run();
}

void catchSignal(int signal) {
	std::cout << std::endl << "Exiting.." << std::endl;

	serverPtr->exit();
}

Server::Server(unsigned int port) {
	this->address.host = ENET_HOST_ANY;
	this->address.port = port;

	this->exiting = false;
	this->lastStreamTime = 0.0;
}

int Server::run() {
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

	this->connection = enet_host_create(&this->address, net::MAX_CLIENTS, net::CHANNELS, 0, 0);

	if (this->connection == nullptr) {
		std::cerr << "Could not bind to " << net::AddressToString(this->address) << "!" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Server listening on " << net::AddressToString(this->address) << "." << std::endl;

	// Enter the main loop
	this->mainLoop();

	// Disconnect all remaining clients
	for (auto& client : this->clients) {
		enet_peer_disconnect_now(client.second->peer, 0);
		delete static_cast<unsigned char*>(client.second->peer->data);
		delete client.second;
	}

	// Dispose all objects
	for (auto& object : this->objects) {
		delete object.second;
	}

	// Exit the server
	this->dispose();
	return EXIT_SUCCESS;
}

void Server::mainLoop() {
	while (! this->exiting) {
		this->networkEvents();

		this->sendStream();
	}
}

void Server::exit() {
	this->exiting = true;
}

void Server::dispose() {
	enet_host_destroy(this->connection);
	enet_deinitialize();
}

void Server::networkEvents() {
	ENetEvent event;

	// Wait up to 100 milliseconds for an event.
	while (enet_host_service(this->connection, &event, 100) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT: {
				std::cout << "A new client connected from " << net::AddressToString(event.peer->address) << "." << std::endl;

				unsigned char *id = new unsigned char;
				*id = net::firstUnusedKey(this->clients);
				net::Client *client = new net::Client(event.peer);
				client->joined = false;
				client->peer = event.peer;
				client->nick.clear();
				client->id = *id;
				this->clients[*id] = client;
				event.peer->data = id;

				break;
			}

			case ENET_EVENT_TYPE_RECEIVE: {
				unsigned char *id = (unsigned char*) event.peer->data;

				if (this->clients[*id]->joined || event.packet->data[0] == net::PACKET_HANDSHAKE) {
					this->receivePacket(event);
				}

				enet_packet_destroy(event.packet);

				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT: {
				unsigned char *id = (unsigned char*) event.peer->data;

				if (this->clients[*id]->joined) {
					std::cout << this->clients[*id]->nick << " has left the server!" << std::endl;

					// Broadcast the received event
					char data[2];
					data[0] = net::PACKET_LEAVE;
					data[1] = *id;

					net::sendCommand(this->connection, data, 2);

					// Release selected and owned objects
					for (auto& object : this->objects) {
						if (object.second->isSelectedBy(this->clients[*id])) {
							object.second->select(nullptr);
						}

						if (object.second->isOwnedBy(this->clients[*id])) {
							object.second->own(nullptr);
						}
					}

					// Reset the peer's client information.
					delete this->clients.find(*id)->second;
					this->clients.erase(*id);

					delete static_cast<unsigned char*>(event.peer->data);
					event.peer->data = nullptr;
				}

				break;
			}

			case ENET_EVENT_TYPE_NONE: {
				break;
			}
		}
	}
}

void Server::receivePacket(ENetEvent event) {
	unsigned char *id = (unsigned char*) event.peer->data;

	switch (event.packet->data[0]) {
		case net::PACKET_HANDSHAKE: {
			if (event.packet->dataLength >= 2 && event.packet->dataLength <= 17) {
				std::string nick = std::string((char*) event.packet->data + 1, event.packet->dataLength - 1);

				if (! isNickTaken(this->clients, nick)) {
					this->clients[*id]->joined = true;
					this->clients[*id]->nick = nick;

					std::cout << this->clients[*id]->nick << " has joined the server!" << std::endl;

					// Reply to the joining client with his ID and the list of clients
					{
						std::string data;
						data += net::PACKET_HANDSHAKE;
						data += *id;

						for (std::map<unsigned char, net::Client*>::iterator client = clients.begin(); client != clients.end(); ++client) {
							if (client->second->joined && client->first != *id)
							{
								data += client->first;
								data += (char) client->second->nick.length();
								data += client->second->nick;
							}
						}

						net::sendCommand(event.peer, data.c_str(), data.length());
					}

					// Send the list of objects
					for (auto& object : this->objects) {
						std::string data;
						data += net::PACKET_CREATE;
						data += 255;
						net::dataAppendShort(data, object.second->getId());
						data += net::clientToClientId(object.second->getSelected());
						data += net::clientToClientId(object.second->getOwner());
						data += object.second->isFlipped();
						net::dataAppendVector2(data, object.second->getLocation());
						data.append(object.second->getObjectId());

						net::sendCommand(event.peer, data.c_str(), data.length());
					}

					// Broadcast a join event
					{
						std::string data;
						data += net::PACKET_JOIN;
						data += *id;
						data += nick;

						net::sendCommand(connection, data.c_str(), data.length());
					}

					// Rush stream information
					this->lastStreamTime = 0.0;
				} else {
					// Reply that the nick is taken
					char data[1];
					data[0] = net::PACKET_NICK_TAKEN;

					net::sendCommand(event.peer, data, 1);
				}
			}

			break;
		}

		case net::PACKET_CHAT: {
			if (event.packet->dataLength >= 1 + 1 && event.packet->dataLength <= 1 + 1 + 255) {
				std::string data;
				data += net::PACKET_CHAT;
				data += *id;
				data.append((char*) event.packet->data + 1, event.packet->dataLength - 1);

				std::cout << clients[*id]->nick << ": " << std::string((char*) (event.packet->data + 1), event.packet->dataLength - 1) << std::endl;
				net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 1);
			}

			break;
		}

		case net::PACKET_DICE: {
			if (event.packet->dataLength == 1 + 2) {
				unsigned short maxValue = net::bytesToShort(event.packet->data + 1);

				if (maxValue == 0) {
					maxValue = 1;
				}

				srand(enet_time_get());

				std::ostringstream reply;
				reply << this->clients[*id]->nick << " threw " << "d" << maxValue << " and got " << 1 + (rand() % maxValue) << ".";

				std::string data;
				data += net::PACKET_CHAT;
				data += 255;
				data.append(reply.str());

				std::cout << reply.str() << std::endl;
				net::sendCommand(this->connection, data.c_str(), 2 + reply.str().length());
			}

			break;
		}

		case net::PACKET_CREATE: {
			if (event.packet->dataLength >= 1 + 9 + 1 && event.packet->dataLength <= 1 + 9 + 255) {
				net::Client *selected = net::clientIdToClient(this->clients, event.packet->data[1]);
				net::Client *owner = net::clientIdToClient(this->clients, event.packet->data[2]);
				bool flipped = event.packet->data[3];
				Vector2 location = net::bytesToVector2(event.packet->data + 4);
				std::string objectId = std::string((char*) event.packet->data + 12, event.packet->dataLength - 12);

				unsigned short objId = net::firstUnusedKey(this->objects);
				Object *object = new Object(objectId, objId, location);
				object->select(selected);
				object->own(owner);
				object->setFlipped(flipped);
				this->objects.insert(std::pair<unsigned short, Object*>(objId, object));
				
				if (! object->getName().empty()) {
					std::string data;
					data += net::PACKET_CREATE;
					data += *id;
					net::dataAppendShort(data, objId);
					data.append((char*) event.packet->data + 1, event.packet->dataLength - 1);

					std::cout << clients[*id]->nick << " created a new " << object->getName() << "." << std::endl;
					net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 3);
				} else {
					std::cout << "Error: object " << objectId << " is not recognized by the server!" << std::endl;
					this->objects.erase(objId);
				}
			}

			break;
		}

		case net::PACKET_MOVE: {
			if (event.packet->dataLength >= 1 + 10) {
				if (this->objects.count(event.packet->data[1]) > 0) {
					std::string data;
					data += net::PACKET_MOVE;
					data += *id;
					data.append((char*) event.packet->data + 1, event.packet->dataLength - 1);

					net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 1);

					unsigned int numberObjects = 0;
					Object *lastObject;

					size_t i = 1;
					while (i < event.packet->dataLength) {
						++numberObjects;
	
						unsigned short objId = net::bytesToShort(event.packet->data + i);
						Vector2 location = net::bytesToVector2(event.packet->data + i + 2);

						Object *object = this->objects.find(objId)->second;
						object->setLocation(location);
						lastObject = object;

						i += 10;
					}

					if (numberObjects == 1) {
						std::cout << clients[*id]->nick << " moved " << lastObject->getName() << "." << std::endl;
					} else if (numberObjects >= 2) {
						std::cout << clients[*id]->nick << " moved " << numberObjects << " objects." << std::endl;
					}
				}
			}

			break;
		}

		case net::PACKET_SELECT: {
			if (event.packet->dataLength >= 1) {
				std::string data;

				data += net::PACKET_SELECT;
				data += *id;
				data.append((char*) event.packet->data + 1, event.packet->dataLength - 1);

				for (auto& object : this->objects) {
					if (object.second->isSelectedBy(this->clients.find(*id)->second)) {
						object.second->select(nullptr);
					}
				}

				size_t i = 1;
				while (i < event.packet->dataLength) {
					unsigned short objId = net::bytesToShort(event.packet->data + i);

					Object* object = this->objects.find(objId)->second;
					object->select(clients[*id]);

					i += 2;
				}

				net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 1);
			}

			break;
		}

		case net::PACKET_FLIP: {
			if (event.packet->dataLength >= 1) {
				std::string data;

				data += net::PACKET_FLIP;
				data += *id;
				data.append((char*) event.packet->data + 1, event.packet->dataLength - 1);

				bool flipped = event.packet->data[1];

				unsigned int numberObjects = 0;
				Object *lastObject;

				size_t i = 2;
				while (i < event.packet->dataLength) {
					++numberObjects;

					unsigned short objId = net::bytesToShort(event.packet->data + i);

					Object* object = this->objects.find(objId)->second;
					object->setFlipped(flipped);
					lastObject = object;

					i += 2;
				}

				if (numberObjects == 1) {
					std::cout << clients[*id]->nick << " flipped " << lastObject->getName() << "." << std::endl;
				} else if (numberObjects >= 2) {
					std::cout << clients[*id]->nick << " flipped " << numberObjects << " objects." << std::endl;
				}

				net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 1);
			}

			break;
		}

		case net::PACKET_OWN: {
			if (event.packet->dataLength >= 1) {
				std::string data;

				data += net::PACKET_OWN;
				data += *id;
				data.append((char*) event.packet->data + 1, event.packet->dataLength - 1);

				bool owned = event.packet->data[1];

				unsigned int numberObjects = 0;
				Object *lastObject;

				size_t i = 2;
				while (i < event.packet->dataLength) {
					++numberObjects;

					unsigned short objId = net::bytesToShort(event.packet->data + i);

					Object* object = this->objects.find(objId)->second;
					if (owned) {
						object->own(clients[*id]);
					} else {
						object->own(nullptr);
					}

					lastObject = object;

					i += 2;
				}

				std::string verb;
				if (owned) {
					verb = "owned";
				} else {
					verb = "disowned";
				}

				if (numberObjects == 1) {
					std::cout << clients[*id]->nick << " " << verb << " " << lastObject->getName() << "." << std::endl;
				} else if (numberObjects >= 2) {
					std::cout << clients[*id]->nick << " " << verb << " " << numberObjects << " objects." << std::endl;
				}

				net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 1);
			}

			break;
		}
	}
}

void Server::sendStream() {
	if (enet_time_get() > this->lastStreamTime + net::STREAM_INTERVAL) {
		this->lastStreamTime = enet_time_get();

		// Stream ping information
		{
			std::string data;
			data += net::PACKET_PINGS;

			for (unsigned char id = 0; id < net::MAX_CLIENTS; ++id) {
				if (this->clients.count(id) > 0 && this->clients[id]->joined) {
					data += id;
					net::dataAppendShort(data, this->clients[id]->peer->roundTripTime);
				}
			}

			// Only send stream data if there is at least one client
			if (data.length() > 1) {
				net::sendCommand(this->connection, data.c_str(), data.length(), false);
			}
		}
	}
}
