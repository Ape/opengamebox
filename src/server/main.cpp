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
		port = 0;
	}

	// Unix-like systems such as Linux _need_ to pass argv[0] from main() in here.
	if (PHYSFS_init(argv[0]) == 0) {
		std::cout << "Failed to initialize physfs" << std::endl;
		return EXIT_FAILURE;
	}

	PHYSFS_setWriteDir("data/");
	PHYSFS_addToSearchPath(".", 1);

	Server server = Server(port);
	Server::serverPtr = &server;

	return server.run();
}

Server *Server::serverPtr;

void Server::catchSignal(int signal) {
	std::cout << std::endl << "Exiting.." << std::endl;

	Server::serverPtr->exit();
}

Server::Server(unsigned int port) {
	this->address.host = ENET_HOST_ANY;
	this->address.port = port;

	this->settings = new Settings("server.cfg");

	this->exiting = false;
	this->lastStreamTime = 0.0;

	this->randomGenerator.seed(enet_time_get());
}

Server::~Server() {
	delete this->settings;
}

int Server::run() {
	// Check the port
	if (this->address.port == 0) {
		this->address.port = this->settings->getValue<int>("network.port");
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
	for (auto &client : this->clients) {
		enet_peer_disconnect_now(client.second->getPeer(), 0);
		delete static_cast<unsigned char*>(client.second->getPeer()->data);
		delete client.second;
	}

	// Dispose all objects
	for (auto &object : this->objects) {
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
				*id = utils::firstUnusedKey(this->clients);
				ServerClient *client = new ServerClient(event.peer, *id);
				this->clients[*id] = client;
				event.peer->data = id;

				// Set ping interval for the connection, this is only supported in ENet >= 1.3.4
				#if ENET_VERSION >= ENET_VERSION_CREATE(1, 3, 4)
				enet_peer_ping_interval(event.peer, net::PING_INTERVAL);
				#endif

				break;
			}

			case ENET_EVENT_TYPE_RECEIVE: {
				unsigned char *id = static_cast<unsigned char*>(event.peer->data);

				if (this->clients[*id]->isJoined() || event.packet->data[0] == net::PACKET_HANDSHAKE) {
					this->receivePacket(event);
				} else if (event.packet->data[0] == net::PACKET_MS_QUERY) {
					// This is not a master server
					enet_peer_disconnect_now(event.peer, 0);

					// TODO: Serve as a master server with only the local server on the list
				}

				enet_packet_destroy(event.packet);

				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT: {
				unsigned char *id = static_cast<unsigned char*>(event.peer->data);

				if (this->clients[*id]->isJoined()) {
					std::cout << this->clients[*id]->getNick() << " has left the server!" << std::endl;

					// Broadcast the received event
					char data[2];
					data[0] = net::PACKET_LEAVE;
					data[1] = *id;

					net::sendCommand(this->connection, data, 2);

					// Release selected and owned objects
					for (auto &object : this->objects) {
						if (object.second->isSelectedBy(this->clients[*id])) {
							object.second->select(nullptr);
						}

						if (object.second->isOwnedBy(this->clients[*id])) {
							object.second->setOwner(nullptr);
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
	Packet packet(event.packet);
	Packet::Header header = packet.readHeader();

	try {
		unsigned char *id = static_cast<unsigned char*>(event.peer->data);
		ServerClient *sender = this->clients[*id];

		switch (header) {
			case Packet::Header::HANDSHAKE: {
				if (event.packet->dataLength >= 2 && event.packet->dataLength <= 17) {
					std::string nick = std::string(reinterpret_cast<char*>(event.packet->data + 1), event.packet->dataLength - 1);

					if (! net::isNickTaken(this->clients, nick)) {
						this->clients[*id]->join();
						this->clients[*id]->setNick(nick);

						std::cout << this->clients[*id]->getNick() << " has joined the server!" << std::endl;

						// Reply to the joining client with his ID and the list of clients
						{
							std::string data;
							data += net::PACKET_HANDSHAKE;
							data += *id;

							for (std::map<unsigned char, ServerClient*>::iterator client = this->clients.begin(); client != this->clients.end(); ++client) {
								if (client->second->isJoined() && client->first != *id)
								{
									data += client->first;
									data += static_cast<char>(client->second->getNick().length());
									data += client->second->getNick();
								}
							}

							net::sendCommand(event.peer, data.c_str(), data.length());
						}

						// Send the list of objects
						{
							std::string data;
							data += net::PACKET_CREATE;
							data += 255; // The objects are now new

							for (auto &object : this->objects) {
								net::dataAppendShort(data, object.second->getId());
								data += Client::getIdStatic(object.second->getSelected());
								data += Client::getIdStatic(object.second->getOwner());
								data += object.second->isFlipped();
								net::dataAppendVector2(data, object.second->getLocation());
								data.push_back(floor(object.second->getRotation() / (utils::PI / 8.0f) + 0.5f));
								data.push_back(static_cast<char>(object.second->getFullId().size()));
								data.append(object.second->getFullId());
							}

							net::sendCommand(event.peer, data.c_str(), data.length());
						}

						// Send object order
						{
							if (!this->objects.empty()) {
								Packet reply(this->connection);
								reply.writeHeader(Packet::Header::ORDER);
								for (auto &object : this->objectOrder) {
									reply.writeShort(object->getId());
								}
								reply.send();
							}
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

			case Packet::Header::LOGIN: {
				if (this->settings->getValue<bool>("network.allowadmin")
						&& packet.readString() == this->settings->getValue<std::string>("network.adminpassword")) {
					std::cout << sender->getNick() << " logged in as an admin." << std::endl;

					// TODO: Send a chat message informing about the login.

					sender->grantAdmin();
				}

				break;
			}

			case Packet::Header::KICK: {
				if (sender->isAdmin()) {
					ServerClient *target = ServerClient::getClientWithId(this->clients, packet.readByte());
					if (target != nullptr) {
						// TODO: Send a chat message informing about the kick.

						enet_peer_disconnect(target->getPeer(), 0);
					}
				}

				break;
			}

			case Packet::Header::CHAT: {
				if (event.packet->dataLength >= 1 + 1 && event.packet->dataLength <= 1 + 1 + 255) {
					std::string data;
					data += net::PACKET_CHAT;
					data += *id;
					data.append(reinterpret_cast<char*>(event.packet->data + 1), event.packet->dataLength - 1);

					std::cout << this->clients[*id]->getNick() << ": " << std::string(reinterpret_cast<char*>(event.packet->data + 1), event.packet->dataLength - 1) << std::endl;
					net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 1);
				}

				break;
			}

			case Packet::Header::ROLL: {
				if (event.packet->dataLength == 1 + 2) {
					unsigned short maxValue = net::bytesToShort(event.packet->data + 1);

					if (maxValue == 0) {
						maxValue = 1;
					}

					std::uniform_int_distribution<unsigned short> distribution(1, maxValue);

					std::ostringstream reply;
					reply << this->clients[*id]->getColoredNick() << " rolled a " << "d" << maxValue << " and got " << distribution(this->randomGenerator) << ".";

					std::string data;
					data += net::PACKET_CHAT;
					data += 255;
					data.append(reply.str());

					std::cout << reply.str() << std::endl;
					net::sendCommand(this->connection, data.c_str(), 2 + reply.str().length());
				}

				break;
			}

			case Packet::Header::CREATE: {
				unsigned int i = 0;
				unsigned int amount = 0;
				std::string data;
				data.push_back(net::PACKET_CREATE);
				data += *id;

				if (event.packet->dataLength >= 1 + 9 + 1) {
					while (i < event.packet->dataLength - 1) {
						ServerClient *selected = ServerClient::getClientWithId(this->clients, event.packet->data[i + 1]);
						ServerClient *owner = ServerClient::getClientWithId(this->clients, event.packet->data[i + 2]);
						bool flipped = event.packet->data[i + 3];
						Vector2 location = net::bytesToVector2(event.packet->data + i + 4);
						float rotation = event.packet->data[i + 12] * utils::PI / 8.0f;
						unsigned char length = event.packet->data[i + 13];
						std::vector<std::string> objectData = utils::splitString(std::string(reinterpret_cast<char*>(event.packet->data + i + 14), static_cast<int>(length)), '.');
						if (objectData.size() == 3) {
							ObjectClass *objectClass;
							try {
								objectClass = this->objectClassManager.getObjectClass(objectData.at(0), objectData.at(1), nullptr);
							} catch (IOException &e) {
								std::cout << "Error: object " << objectData.at(0) << "." << objectData.at(1) << "." << objectData.at(2)
								          << " is not recognized by the server!" << std::endl;

								// TODO: Inform the client that the object is not recognized.

								break;
							}

							unsigned short objId = utils::firstUnusedKey(this->objects);
							Object *object = new Object(objectClass, objectData.at(2), objId, location);
							object->select(selected);
							object->setOwner(owner);
							object->setFlipped(flipped);
							object->rotate(rotation);
							this->objects.insert(std::pair<unsigned short, Object*>(objId, object));
							this->objectOrder.push_back(object);

							std::string temp;
							net::dataAppendShort(temp, objId);

							for (char c : temp) {
								data.push_back(c);
							}

							std::string temp2(reinterpret_cast<char*>(event.packet->data + i + 1), static_cast<int>(length) + 13);

							for (char c : temp2) {
								data.push_back(c);
							}

							amount++;
						}

						i += length + 13;
					}

					std::cout << this->clients[*id]->getNick() << " created " << amount << " objects." << std::endl;
				}

				net::sendCommand(this->connection, data.c_str(), data.size());
				break;
			}

			case Packet::Header::MOVE: {
				if (event.packet->dataLength >= 1 + 10) {
					if (this->objects.count(event.packet->data[1]) > 0) {
						std::string data;
						data += net::PACKET_MOVE;
						data += *id;
						data.append(reinterpret_cast<char*>(event.packet->data + 1), event.packet->dataLength - 1);

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

							net::removeObject(this->objectOrder, object);
							this->objectOrder.push_back(object);

							i += 10;
						}

						if (numberObjects == 1) {
							std::cout << this->clients[*id]->getNick() << " moved " << lastObject->getName() << "." << std::endl;
						} else if (numberObjects >= 2) {
							std::cout << this->clients[*id]->getNick() << " moved " << numberObjects << " objects." << std::endl;
						}
					}
				}

				break;
			}

			case Packet::Header::SELECT: {
				if (event.packet->dataLength >= 1) {
					std::string data;

					data += net::PACKET_SELECT;
					data += *id;
					data.append(reinterpret_cast<char*>(event.packet->data + 1), event.packet->dataLength - 1);

					for (auto &object : this->objects) {
						if (object.second->isSelectedBy(this->clients.find(*id)->second)) {
							object.second->select(nullptr);
						}
					}

					size_t i = 1;
					while (i < event.packet->dataLength) {
						unsigned short objId = net::bytesToShort(event.packet->data + i);

						Object* object = this->objects.find(objId)->second;
						object->select(this->clients[*id]);

						i += 2;
					}

					net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 1);
				}

				break;
			}

			case Packet::Header::REMOVE: {
				if (event.packet->dataLength >= 1) {
					std::string data;

					data += net::PACKET_REMOVE;
					data += *id;
					data.append(reinterpret_cast<char*>(event.packet->data + 1), event.packet->dataLength - 1);

					unsigned int numberObjects = 0;
					std::string lastObject;

					size_t i = 1;
					while (i < event.packet->dataLength) {
						++numberObjects;

						unsigned short objId = net::bytesToShort(event.packet->data + i);

						Object *object = this->objects.find(objId)->second;
						this->objects.erase(objId);
						net::removeObject(this->objectOrder, object);

						lastObject = object->getName();
						delete object;

						i += 2;
					}

					if (numberObjects == 1) {
						std::cout << this->clients[*id]->getNick() << " removed " << lastObject << "." << std::endl;
					} else if (numberObjects >= 2) {
						std::cout << this->clients[*id]->getNick() << " removed " << utils::toString(numberObjects) << " objects." << std::endl;
					}

					net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 1);
				}

				break;
			}

			case Packet::Header::FLIP: {
				if (event.packet->dataLength >= 1) {
					std::string data;

					data += net::PACKET_FLIP;
					data += *id;
					data.append(reinterpret_cast<char*>(event.packet->data + 1), event.packet->dataLength - 1);

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
						std::cout << this->clients[*id]->getNick() << " flipped " << lastObject->getName() << "." << std::endl;
					} else if (numberObjects >= 2) {
						std::cout << this->clients[*id]->getNick() << " flipped " << numberObjects << " objects." << std::endl;
					}

					net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 1);
				}

				break;
			}

			case Packet::Header::OWN: {
				if (event.packet->dataLength >= 1) {
					std::string data;

					data += net::PACKET_OWN;
					data += *id;
					data.append(reinterpret_cast<char*>(event.packet->data + 1), event.packet->dataLength - 1);

					bool owned = event.packet->data[1];

					unsigned int numberObjects = 0;
					Object *lastObject;

					size_t i = 2;
					while (i < event.packet->dataLength) {
						++numberObjects;

						unsigned short objId = net::bytesToShort(event.packet->data + i);

						Object* object = this->objects.find(objId)->second;
						if (owned) {
							object->setOwner(this->clients[*id]);
						} else {
							object->setOwner(nullptr);
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
						std::cout << this->clients[*id]->getNick() << " " << verb << " " << lastObject->getName() << "." << std::endl;
					} else if (numberObjects >= 2) {
						std::cout << this->clients[*id]->getNick() << " " << verb << " " << numberObjects << " objects." << std::endl;
					}

					net::sendCommand(this->connection, data.c_str(), event.packet->dataLength + 1);
				}

				break;
			}

			case Packet::Header::SHUFFLE: {
				if (event.packet->dataLength == 1) {
					// Randomize object locations
					{
						std::string data;
						data += net::PACKET_MOVE;
						data += *id;

						std::vector<Object*> objects;
						std::vector<std::pair<int, Vector2>> locations;

						// Get objects to suffle and order and locations of them
						for (auto &object : this->objects) {
							if (object.second->isSelectedBy(this->clients[*id])) {
								objects.push_back(object.second);
								int orderIndex = std::distance(this->objectOrder.begin(), std::find(this->objectOrder.begin(),
																this->objectOrder.end(), object.second));
								locations.push_back(std::pair<int, Vector2>(orderIndex, object.second->getLocation()));
							}
						}

						// Suffle
						std::random_shuffle(objects.begin(), objects.end());

						// Move the objects
						std::vector<Vector2>::size_type location = 0;
						for (auto &object : objects) {
							object->setLocation(locations.at(location).second);
							this->objectOrder[locations.at(location).first] = object;
							net::dataAppendShort(data, object->getId());
							net::dataAppendVector2(data, object->getLocation());
							++location;
						}

						net::sendCommand(this->connection, data.c_str(), data.length());

						// Send new obj order
						Packet reply(this->connection);
						reply.writeHeader(Packet::Header::ORDER);
						for (auto &object : this->objectOrder) {
							reply.writeShort(object->getId());
						}
						reply.send();
					}
				}

				break;
			}

			case Packet::Header::ROTATE: {
				unsigned short objId = net::bytesToShort(event.packet->data + 1);
				char rotation = event.packet->data[3];
				this->objects[objId]->rotate(rotation * utils::PI / 8.0f);
				net::sendCommand(this->connection, reinterpret_cast<char*>(event.packet->data), event.packet->dataLength);

				break;
			}

			case Packet::Header::PACKAGE_MISSING: {
				std::string package = packet.readString();
				std::cout<< this->clients.find(*id)->second->getNick() << " is missing package "<<package<<std::endl;

				if (PHYSFS_exists(("data/" + package + ".zip").c_str())) {
					PHYSFS_file* file = PHYSFS_openRead(("data/" + package + ".zip").c_str());
					Packet reply(event.peer);
					reply.writeHeader(Packet::Header::FILE_TRANSFER);
					reply.writeShort(0);
					reply.writeInt(PHYSFS_fileLength(file));
					reply.writeString(package);
					reply.send();

					char *buf;
					buf = new char[PHYSFS_fileLength(file)];
					PHYSFS_read(file, buf, 1, PHYSFS_fileLength(file));
					int i = 0;
					short number = 1;

					const int packetSize = event.peer->mtu - 7 - 50;

					while (i < PHYSFS_fileLength(file)) {
						Packet piece(event.peer);
						piece.writeHeader(Packet::Header::FILE_TRANSFER);
						piece.writeShort(number);
						piece.writeInt(i);
						number++;

						if (i + packetSize <= PHYSFS_fileLength(file)) {
							piece.writeInt(packetSize);
							piece.writeString(std::string(buf + i, packetSize));
						} else {
							piece.writeInt(PHYSFS_fileLength(file) - i);
							piece.writeString(std::string(buf + i, PHYSFS_fileLength(file) - i));
						}

						piece.send();
						i += packetSize;
					}

					PHYSFS_close(file);
				}

				break;
			}
			case Packet::Header::SCALE: {
				if (event.packet->dataLength >= 1) {
					Packet reply(this->connection);
					reply.writeHeader(Packet::Header::SCALE);
					while(!packet.eof()) {
						unsigned short id = packet.readShort();
						float scale = packet.readFloat();
						this->objects.at(id)->setScale(scale);
						reply.writeShort(id);
						reply.writeFloat(scale);
					}
					reply.send();
				}
				break;
			}

			default: {
				throw PacketException("Invalid packet header.");
			}
		}
	} catch (PacketException &e) {
		std::cout << "Debug: Received an invalid packet from "
		          << net::AddressToString(event.peer->address)
		          << ": \"" << e.what() << "\"" << std::endl;
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
				if (this->clients.count(id) > 0 && this->clients[id]->isJoined()) {
					data += id;
					net::dataAppendShort(data, this->clients[id]->getPeer()->roundTripTime);
				}
			}

			// Only send stream data if there is at least one client
			if (data.length() > 1) {
				net::sendCommand(this->connection, data.c_str(), data.length(), false);
			}
		}
	}
}
