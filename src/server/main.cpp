#include "main.h"

int main(int argc, char **argv){
	// Get port number from command line arguments
	unsigned int port;
	if (argc >= 2){
		std::istringstream portString(argv[1]);
		portString >> port;
	}else{
		port = net::DEFAULT_PORT;
	}

	Server server = Server(port);

	return server.run();
}

Server::Server(unsigned int port){
	this->address.host = ENET_HOST_ANY;
	this->address.port = port;

	this->exiting = false;
	this->frame = 0;
	this->lastFrameTime = 0;
}

int Server::run(){
	// Check the port
	if (this->address.port == 0){
		std::cerr << "Illegal port number!" << std::endl;
		return EXIT_FAILURE;
	}

	// Initialize ENnet
	if (enet_initialize() != 0){
		std::cerr << "Could not initialize network components!" << std::endl;
		return EXIT_FAILURE;
	}

	this->connection = enet_host_create(&this->address, net::MAX_CLIENTS, net::CHANNELS, 0, 0);

	if (this->connection == nullptr){
		std::cerr << "Could not bind to " << net::AddressToString(this->address) << "!" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Server listening on " << net::AddressToString(this->address) << "." << std::endl;

	// Enter the main loop
	this->mainLoop();

	// Exit the server
	this->dispose();
	return EXIT_SUCCESS;
}

void Server::mainLoop(){
	while (! this->exiting){
		// TODO: Simulation

		this->networkEvents();
	}
}

void Server::dispose(){
    enet_host_destroy(this->connection);
    enet_deinitialize();
}

void Server::networkEvents(){
	ENetEvent event;

	// Wait up to 100 milliseconds for an event.
	while (enet_host_service(this->connection, &event, 100) > 0){
		switch (event.type){
			case ENET_EVENT_TYPE_CONNECT:{
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

			case ENET_EVENT_TYPE_RECEIVE:{
				unsigned char *id = (unsigned char*) event.peer->data;

				if (this->clients[*id]->joined || event.packet->data[0] == net::PACKET_HANDSHAKE){
					this->receivePacket(event);
				}

				enet_packet_destroy(event.packet);
				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT:{
				unsigned char *id = (unsigned char*) event.peer->data;

				if (this->clients[*id]->joined){
					std::cout << this->clients[*id]->nick << " has left the server!" << std::endl;

					// Broadcast the received event
					char data[2];
					data[0] = net::PACKET_LEAVE;
					data[1] = *id;

					net::sendCommand(this->connection, data, 2);

					// Reset the peer's client information.
					delete this->clients.find(*id)->second;
					this->clients.erase(*id);
					event.peer->data = nullptr;
				}
				break;
			}

			case ENET_EVENT_TYPE_NONE:{
				break;
			}
		}
	}
}

void Server::receivePacket(ENetEvent event){
	unsigned char *id = (unsigned char*) event.peer->data;

	switch (event.packet->data[0]){
		case net::PACKET_HANDSHAKE:{
			if (event.packet->dataLength >= 2 && event.packet->dataLength <= 17){
				std::string nick = std::string((char*) event.packet->data + 1, event.packet->dataLength - 1);

				if (! isNickTaken(this->clients, nick)){
					this->clients[*id]->joined = true;
					this->clients[*id]->nick = nick;

					std::cout << this->clients[*id]->nick << " has joined the server!" << std::endl;

					// Reply to the joining client with his ID and the list of clients
					{
						std::string data;
						data += net::PACKET_HANDSHAKE;
						data += *id;

						for (std::map<unsigned char, net::Client*>::iterator client = clients.begin(); client != clients.end(); ++client){
							if (client->second->joined && client->first != *id)
							{
								data += client->first;
								data += (char) client->second->nick.length();
								data += client->second->nick;
							}
						}

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
				}else{
					// Reply that the nick is taken
					char data[1];
					data[0] = net::PACKET_NICK_TAKEN;

					net::sendCommand(event.peer, data, 1);
				}
			}
			break;
		}

		case net::PACKET_CHAT:{
			if (event.packet->dataLength >= 1 + 1 && event.packet->dataLength <= 1 + 1 + 255){
				std::string data;
				data += net::PACKET_CHAT;
				data += *id;
				data.append((char*) event.packet->data + 1, event.packet->dataLength - 1);

				std::cout << clients[*id]->nick << ": " << std::string((char*) (event.packet->data + 1), event.packet->dataLength - 1) << std::endl;
				net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 1);
			}
			break;
		}

		case net::PACKET_CREATE:{
			if (event.packet->dataLength >= 1 + 6 + 1 && event.packet->dataLength <= 1 + 6 + 255){
				Vector2 location;
				unsigned char bytes[3];

				std::copy(event.packet->data + 1, event.packet->data + 4, bytes);
				location.x = net::bytesToFloat(bytes);

				std::copy(event.packet->data + 4, event.packet->data + 7, bytes);
				location.y = net::bytesToFloat(bytes);

				std::string objectId = std::string((char*) event.packet->data + 7, event.packet->dataLength - 7);

				unsigned int objId = net::firstUnusedKey(this->objects);
				Object *object = new Object(objectId, objId, location);
				this->objects.insert(std::pair<unsigned int, Object*>(objId, object));
				
				if (! object->getName().empty()){
					std::string data;
					data += net::PACKET_CREATE;
					data += *id;
					data += objId;
					data.append((char*) event.packet->data + 1, event.packet->dataLength - 1);

					std::cout << clients[*id]->nick << " created a new " << object->getName() << " to " << location.x << "," << location.y << std::endl;
					net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 2);
				}else{
					std::cout << "Error: object " << objectId << " is not recognized by the server!" << std::endl;
					this->objects.erase(objId);
				}
			}
			break;
		}

		case net::PACKET_MOVE:{
			if (event.packet->dataLength == 2 + 6){
				std::cout << event.packet->data[1] << ": " << this->objects.count(event.packet->data[1]) << std::endl;
				if (this->objects.count(event.packet->data[1]) > 0){
					Vector2 location;
					unsigned char bytes[3];

					std::copy(event.packet->data + 2, event.packet->data + 5, bytes);
					location.x = net::bytesToFloat(bytes);

					std::copy(event.packet->data + 5, event.packet->data + 8, bytes);
					location.y = net::bytesToFloat(bytes);

					this->objects.find(event.packet->data[1])->second->setLocation(location);

					std::string data;
					data += net::PACKET_MOVE;
					data += *id;
					data.append((char*) event.packet->data + 1, event.packet->dataLength - 1);

					std::cout << clients[*id]->nick << " moved object to " << location.x << "," << location.y << std::endl;
					net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 1);
				}
			}
			break;
		}
	}
}

void Server::sendStream(){
	// TODO: Do we want this?
	/*if (enet_time_get() > this->lastStreamTime + net::STREAM_INTERVAL){
		this->lastStreamTime = enet_time_get();

		// Stream ping information{
			std::string data;
			data += net::PACKET_PINGS;

			for (unsigned char id = 0; id < net::MAX_CLIENTS; ++id){
				if (this->clients.count(id) > 0  && this->clients[id]->joined){
					data += id;
					data += this->clients[id]->peer->lastRoundTripTime & 0xFF;
					data += (this->clients[id]->peer->lastRoundTripTime >> 8) & 0xFF;
				}
			}

			if (data.length() > 1) // Only send stream data if there is at least one client{
				net::sendCommand(this->connection, data.c_str(), data.length(), false);
			}
		}
	}*/
}
