// Copyright 2012 Lauri Niskanen
// Copyright 2012 Antti Aalto
// Copyright 2012 Markus Mattinen
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
	// Get host address and port from command line arguments
	std::string address;
	int port;

	Game game;
	if (!game.init()) {
		return EXIT_FAILURE;
	}

	if (argc == 1) {
		if (game.connectMasterServer(net::MASTER_SERVER, net::MASTER_SERVER_PORT)) {
			return game.run();
		} else {
			return EXIT_FAILURE;
		}
	}

	if (argc >= 2) {
		address = std::string(argv[1]);
	} else {
		address = "localhost";
	}

	if (argc >= 3) {
		std::istringstream stream(argv[2]);
		stream >> port;
	} else {
		port = net::DEFAULT_PORT;
	}

	if (game.connect(address, port)) {
		return game.run();
	} else {
		return EXIT_FAILURE;
	}
}

Game::Game(void) {
	this->state = Game::State::INITIALIZING;
	this->connectionState = Game::ConnectionState::NOT_CONNECTED;
	this->nextFrame = true;
	this->deltaTime = 0.0f;
	this->input = nullptr;
	this->localClient = net::MAX_CLIENTS;

	this->dragging = false;
	this->keyStatus = KeyStatus();
}

bool Game::init() {
	// Initialize Allegro
	if (! al_init()) {
		std::cerr << "Failed to initialize Allegro!" << std::endl;
		return false;
	}

	// Reset frame timestamp
	this->previousTime = al_get_time();

	// Initialize input
	if (! al_install_keyboard() || ! al_install_mouse()) {
		std::cerr << "Failed to initialize the input components!" << std::endl;
		return false;
	}

	// Set the window mode
	if (FULLSCREEN) {
		al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
	} else {
		al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
	}

	// Initialize the renderer
	this->renderer = new Renderer(Coordinates(SCREEN_W, SCREEN_H), MULTISAMPLING_SAMPLES);

	// Set window title
	this->renderer->setWindowTitle("OpenGamebox", "gfx/icon");

	// Create a timer for the main loop
	this->timer = al_create_timer(1.0f / FPS_LIMIT);
	if (! this->timer) {
		std::cerr << "Failed to create a timer!" << std::endl;
		return false;
	}

	// Create an event queue
	this->event_queue = al_create_event_queue();
	if (! this->event_queue) {
		std::cerr << "Failed to create an event queue!" << std::endl;
		return false;
	}

	// Register event sources
	al_register_event_source(this->event_queue, al_get_display_event_source(this->renderer->getDisplay()));
	al_register_event_source(this->event_queue, al_get_timer_event_source(this->timer));
	al_register_event_source(this->event_queue, al_get_keyboard_event_source());
	al_register_event_source(this->event_queue, al_get_mouse_event_source());

	al_start_timer(this->timer);

	// Initialize enet
	if (enet_initialize() != 0) {
		std::cerr << "Failed to initialize the network components!" << std::endl;
		return false;
	}

	return true;
}

bool Game::connect(std::string address, int port) {
	this->connection = enet_host_create (NULL,          // Create a client host
	                                     1,             // Only allow 1 outgoing connection
	                                     net::CHANNELS, // Number of channels
	                                     0,             // Unlimited downstream bandwidth
	                                     0);            // Unlimited upstream bandwidth

	if (enet_address_set_host(&(this->hostAddress), address.c_str()) < 0) {
		std::cerr << "Unknown host!" << std::endl;
		return false;
	}

	if (port == 0) {
		std::cerr << "Illegal port number!" << std::endl;
		return false;
	}
	this->hostAddress.port = port;

	if (this->connection == NULL) {
		std::cerr << "Could not create a connection!" << std::endl;
		return false;
	}

	// Initialize the connection
	this->addMessage("Connecting to " + net::AddressToString(this->hostAddress) + "...");
	host = enet_host_connect(this->connection, &this->hostAddress, 1, 0);

	if (this->host == NULL) {
		std::cerr << "Could not connect to the server!" << std::endl;
		return false;
	}

	this->connectionState = Game::ConnectionState::CONNECTING;

	return true;
}

bool Game::connectMasterServer(std::string address, int port) {
	// Create a connection to the master server
	this->connection = enet_host_create (NULL, 1, 1, 0, 0);

	if (enet_address_set_host(&(this->hostAddress), address.c_str()) < 0) {
		std::cerr << "Unknown master server host!" << std::endl;
		return false;
	}

	if (port == 0) {
		std::cerr << "Illegal master server port number!" << std::endl;
		return false;
	}
	this->hostAddress.port = port;

	if (this->connection == NULL) {
		std::cerr << "Could not create a connection to the master server!" << std::endl;
		return false;
	}

	// Initialize the connection
	this->addMessage("Connecting to " + net::AddressToString(this->hostAddress) + " (master server)...");
	host = enet_host_connect(this->connection, &this->hostAddress, 1, 0);

	if (this->host == NULL) {
		std::cerr << "Could not connect to the master server!" << std::endl;
		return false;
	}

	this->connectionState = Game::ConnectionState::CONNECTING_MASTER_SERVER;

	return true;
}

int Game::run() {
	this->state = Game::State::RUNNING;

	// Enter the main loop
	this->mainLoop();

	// Exit the application when returned from the main loop
	this->dispose();
	return EXIT_SUCCESS;
}

void Game::mainLoop() {
	while (this->state != Game::State::TERMINATED) {
		// Process network events
		this->networkEvents();

		// Handle local events
		this->localEvents();

		// Render the screen with limited FPS
		if (this->nextFrame && al_is_event_queue_empty(this->event_queue)) {
			this->nextFrame = false;
			this->update();
			this->render();
		}
	}
}

void Game::quit() {
	if (this->connectionState == Game::ConnectionState::CONNECTED) {
		this->addMessage("Disconnecting...");
		enet_peer_disconnect(this->host, 0);

		this->state = Game::State::EXITING;
		this->connectionState = Game::ConnectionState::DISCONNECTING;
	} else {
		this->state = Game::State::TERMINATED;
	}
}

void Game::dispose() {
	// Dispose client information
	for (auto& client : this->clients) {
		delete client.second;
	}

	// Dispose all objects
	for (auto& object : this->objects) {
		delete object.second;
	}

	// Dispose enet
	if (this->connection != nullptr) {
		enet_host_destroy(this->connection);
		this->connection = nullptr;
	}
	
	enet_deinitialize();

	// Dispose the renderer
	delete this->renderer;

	// Dispose events
	al_destroy_timer(this->timer);
	al_destroy_event_queue(this->event_queue);

	// Dispose input
	al_uninstall_mouse();
	al_uninstall_keyboard();

	// Dispose Allegro
	al_uninstall_system();
}

void Game::localEvents() {
	ALLEGRO_EVENT event;
	al_wait_for_event(event_queue, &event);

	if (event.type == ALLEGRO_EVENT_TIMER) {
		this->nextFrame = true;
	} else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
		this->quit();
	} else if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
		this->renderer->resize();
	} else if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
		if (input != nullptr) {
			this->input->onKey(event.keyboard);
		} else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
			// Create a new chat input widget
			this->input = new InputBox(this, &Game::sendChat, "Chat", Vector2(2.0f, this->renderer->getDisplaySize().y / 2.0f + 20.0f), 300.0f,
			                           this->renderer->getFont(), 255);
		}
	} else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
		if (input == nullptr) {
			if (event.keyboard.keycode == ALLEGRO_KEY_F10) {
				this->quit();
			} else if (event.keyboard.keycode == ALLEGRO_KEY_DELETE) {
				if (this->selectedObjects.size() > 0) {
					std::string data;
					data.push_back(net::PACKET_REMOVE);

					for (auto& object : this->selectedObjects) {
						net::dataAppendShort(data, object->getId());
					}

					net::sendCommand(this->connection, data.c_str(), data.length());
					this->selectedObjects.clear();
					this->dragging = false;
				}
			} else if (event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
				if (this->selectedObjects.size() > 0) {
					std::string data;
					data.push_back(net::PACKET_OWN);

					// If even one of the selected objects isn't owned by the player every selected object will be owned. If all of the
					// objects are owned then all of the objects will be disowned.
					bool owned = true;
 					for (auto& object : this->selectedObjects) {
						if (! object->isOwnedBy(this->clients.find(localClient)->second)) {
							owned = false;

							break;
						}
					}

					data += ! owned;

					for (auto& object : this->selectedObjects) {
						if (owned) {
							object->own(nullptr);
						} else if (object->isOwnedBy(nullptr)) {
							object->own(this->clients.find(localClient)->second);
						}

						net::dataAppendShort(data, object->getId());
					}

					net::sendCommand(this->connection, data.c_str(), data.length());
				}
			} else if (event.keyboard.keycode == ALLEGRO_KEY_S) {
				if (this->selectedObjects.size() > 0) {
					std::string data;
					data += net::PACKET_SHUFFLE;
					net::sendCommand(this->connection, data.c_str(), data.length());

					this->selectedObjects.clear();
					this->dragging = false;
				}
			} else if (event.keyboard.keycode == ALLEGRO_KEY_F) {
				if (this->selectedObjects.size() > 0) {
					std::string data;
					data.push_back(net::PACKET_MOVE);

					Vector2 nextLocation = this->selectedObjects.front()->getLocation();
					for (auto& object : this->selectedObjects) {
						net::dataAppendShort(data, object->getId());
						net::dataAppendVector2(data, nextLocation);

						object->setAnimation(nextLocation, ANIMATION_TIME);
						nextLocation += object->getStackDelta();
					}

					net::sendCommand(this->connection, data.c_str(), data.length());
				}
			} else if (event.keyboard.keycode == ALLEGRO_KEY_PAD_PLUS) {
				this->keyStatus.screenZoomIn = true;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_PAD_MINUS) {
				this->keyStatus.screenZoomOut = true;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
				this->keyStatus.screenMoveLeft = true;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
				this->keyStatus.screenMoveRight = true;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
				this->keyStatus.screenMoveUp = true;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
				this->keyStatus.screenMoveDown = true;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_LCTRL) {
				this->keyStatus.snappingToGrid = true;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_Q) {
				this->keyStatus.screenRotateCClockwise = true;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_W) {
				this->keyStatus.screenRotateClockwise = true;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_E) {
				if(this->selectedObjects.size() > 0) {
					for(auto object : this->selectedObjects) {
						std::string data;
						data.push_back(net::PACKET_ROTATE);
						net::dataAppendShort(data, object->getId());
						data.push_back(0xfc); //-4 twos complement

						net::sendCommand(this->connection, data.c_str(), data.size());
						//object->rotate(utils::PI / 2);
					}
				}
			} else if (event.keyboard.keycode == ALLEGRO_KEY_R) {
				if(this->selectedObjects.size() > 0) {
					for(auto object : this->selectedObjects) {
						std::string data;
						data.push_back(net::PACKET_ROTATE);
						net::dataAppendShort(data, object->getId());
						data.push_back(0x04);

						net::sendCommand(this->connection, data.c_str(), data.size());
						//object->rotate(-utils::PI / 2);
					}
				}
			}
		}
	} else if (event.type == ALLEGRO_EVENT_KEY_UP) {
		if (input == nullptr) {
			if (event.keyboard.keycode == ALLEGRO_KEY_PAD_PLUS) {
				this->keyStatus.screenZoomIn = false;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_PAD_MINUS) {
				this->keyStatus.screenZoomOut = false;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
				this->keyStatus.screenMoveLeft = false;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
				this->keyStatus.screenMoveRight = false;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
				this->keyStatus.screenMoveUp = false;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
				this->keyStatus.screenMoveDown = false;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_Q) {
				this->keyStatus.screenRotateCClockwise = false;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_W) {
				this->keyStatus.screenRotateClockwise = false;
			} else if (event.keyboard.keycode == ALLEGRO_KEY_LCTRL) {
				this->keyStatus.snappingToGrid = false;
			}
		}
	} else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1) {
		Vector2 location(event.mouse.x, event.mouse.y);
		this->renderer->transformLocation(IRenderer::CAMERA_INVERSE, location);

		if (! this->dragging) {
			for (auto& object : this->selectedObjects) {
				object->select(nullptr);
			}
			this->selectedObjects.clear();

			std::string data;
			data.push_back(net::PACKET_SELECT);

			for (auto object = this->objectOrder.rbegin(); object != this->objectOrder.rend(); ++object) {
				if ((*object)->isSelectedBy(nullptr) && ((*object)->isOwnedBy(nullptr) || (*object)->isOwnedBy(this->clients.find(localClient)->second))
				    && (*object)->testLocation(location)) {
					std::set<Object*> visited;
					this->selectedObjects = (*object)->getObjectsAbove(visited);

					if (this->selectedObjects.size() == 1 && this->selectedObjects.front() == nullptr) {
						this->selectedObjects.clear();
					}

					for (auto& objectA : this->selectedObjects) {
						net::dataAppendShort(data, objectA->getId());

						net::removeObject(this->objectOrder, objectA);
						this->objectOrder.push_back(objectA);
					}

					break;
				}
			}

			net::sendCommand(this->connection, data.c_str(), data.length());
		}

		if (!this->selectedObjects.empty()) {
			for (auto& object : this->selectedObjects) {
				if (object->testLocation(location)) {
					this->dragging = true;
					this->draggingStart = this->selectedObjects.front()->getLocation() - location;

					break;
				}
			}
		}
	} else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 2) {
		if (this->dragging) {
			std::string data;
			data.push_back(net::PACKET_FLIP);

			// If even one of the selected objects isn't flipped every selected object will be flipped. If all of the
			// objects are flipped then all of the objects will be unflipped.
			bool flipped = true;
			for (auto& object : this->selectedObjects) {
				if (! object->isFlipped()) {
					flipped = false;

					break;
				}
			}

			data += ! flipped;

			for (auto& object : this->selectedObjects) {
				object->setFlipped(! flipped);

				net::dataAppendShort(data, object->getId());
			}

			net::sendCommand(this->connection, data.c_str(), data.length());
		}
	} else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 3) {
		this->keyStatus.moveScreen = true;
		this->moveScreenStart = Vector2(event.mouse.x, event.mouse.y);
	} else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 1) {
		if (this->dragging) {
			Vector2 location(event.mouse.x, event.mouse.y);
			this->renderer->transformLocation(IRenderer::CAMERA_INVERSE, location);

			std::string data;
			data.push_back(net::PACKET_MOVE);

			for (auto& object : this->selectedObjects) {
				net::dataAppendShort(data, object->getId());
				net::dataAppendVector2(data, object->getLocation());

				object->setAnimation(object->getLocation(), 0.0f);
			}

			net::sendCommand(this->connection, data.c_str(), data.length());

			this->dragging = false;
		}
	} else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 3) {
		this->keyStatus.moveScreen = false;
	} else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
		if (event.mouse.dz != 0) {
			this->renderer->zoomScreen(1 + 0.1f * event.mouse.dz);
			this->renderer->updateTransformations();
		} else if (this->dragging && (event.mouse.dx != 0 || event.mouse.dy != 0)) {
			Vector2 location(event.mouse.x, event.mouse.y);
			this->renderer->transformLocation(IRenderer::CAMERA_INVERSE, location);

			if (this->keyStatus.snappingToGrid) {
				Vector2 grid = this->selectedObjects.front()->getGridSize() + Vector2(0.002f, 0.002f);

				if (location.x < 0.0f) {
					location.x -= grid.x;
				}
				if (location.y < 0.0f) {
					location.y -= grid.y;
				}

				location += -1.0f * (location % grid) + this->selectedObjects.front()->getSize() / 2.0f;
			} else {
				location += this->draggingStart;
			}

			Vector2 delta = location - this->selectedObjects.front()->getLocation();
			for (auto& object : this->selectedObjects) {
				Vector2 destination = object->getLocation() + delta;

				if (destination.x > net::MAX_FLOAT) {
					destination.x = net::MAX_FLOAT;
				}
				if (destination.x < -net::MAX_FLOAT) {
					destination.x = -net::MAX_FLOAT;
				}
				if (destination.y > net::MAX_FLOAT) {
					destination.y = net::MAX_FLOAT;
				}
				if (destination.y < -net::MAX_FLOAT) {
					destination.y = -net::MAX_FLOAT;
				}

				object->setLocation(destination);
			}
		} else if (this->keyStatus.moveScreen && (event.mouse.dx != 0 || event.mouse.dy != 0)) {
			Vector2 location(event.mouse.x, event.mouse.y);

			this->renderer->scrollScreen(location - this->moveScreenStart);
			this->moveScreenStart = location;

			this->renderer->updateTransformations();
		}
	}
}

void Game::networkEvents() {
	ENetEvent event;

	while (this->connectionState != Game::ConnectionState::NOT_CONNECTED && enet_host_service(this->connection, &event, 0) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT: {
				if (this->connectionState == Game::ConnectionState::CONNECTING) {
					this->addMessage("Connected! Please enter your nick.");

					this->connectionState = Game::ConnectionState::CONNECTED;
					this->askNick();
				} else if (this->connectionState == Game::ConnectionState::CONNECTING_MASTER_SERVER) {
					this->addMessage("Connected to the master server.");

					this->connectionState = Game::ConnectionState::CONNECTED_MASTER_SERVER;
					this->queryMasterServer();
				}

				break;

			}

			case ENET_EVENT_TYPE_RECEIVE: {
				if (this->connectionState == Game::ConnectionState::CONNECTED_MASTER_SERVER) {
					this->receivePacket(event);
				} else if (this->connectionState == Game::ConnectionState::CONNECTED) {
					if (this->localClient != net::MAX_CLIENTS || event.packet->data[0] == net::PACKET_HANDSHAKE ||
					    event.packet->data[0] == net::PACKET_NICK_TAKEN) {
						this->receivePacket(event);
					}
				}

				enet_packet_destroy(event.packet);

				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT: {
				this->addMessage("Disconnected from the server.");
				this->connectionState = Game::ConnectionState::NOT_CONNECTED;

				if (this->state == Game::State::EXITING) {
					this->state = Game::State::TERMINATED;
				}

				break;
			}

			case ENET_EVENT_TYPE_NONE: {
				break;
			}
		}
	}
}

void Game::receivePacket(ENetEvent event) {
	switch (event.packet->data[0]) {
		case net::PACKET_HANDSHAKE: {
			if (event.packet->dataLength >= 2 && event.packet->dataLength <= 2 + 34*(net::MAX_CLIENTS - 1)) {
				// Store the received client id
				this->localClient = event.packet->data[1];

				// Update the local client list
				size_t i = 2;
				while (i < event.packet->dataLength) {
					Client *client = new Client(std::string(reinterpret_cast<char*>(event.packet->data + i + 2), event.packet->data[i + 1]),
												Color(this->renderer, event.packet->data[i]), event.packet->data[i]);
					client->ping = 65535;
					this->clients[event.packet->data[i]] = client;

					i += 2 + event.packet->data[i + 1];
				}
			}

			break;
		}

		case net::PACKET_NICK_TAKEN: {
			if (event.packet->dataLength == 1) {
				this->addMessage("That nick is already reserved!");

				this->askNick();
			}

			break;
		}

		case net::PACKET_JOIN: {
			if (event.packet->dataLength >= 3 && event.packet->dataLength <= 35) {
				// Store the client information
				Client *client = new Client(std::string(reinterpret_cast<char*>(event.packet->data + 2), event.packet->dataLength - 2),
											Color(this->renderer, event.packet->data[1]), event.packet->data[1]);
				client->ping = 65535;
				this->clients[event.packet->data[1]] = client;

				this->addMessage(client->getColoredNick() + " has joined the server!");
			}

			break;
		}

		case net::PACKET_LEAVE: {
			if (event.packet->dataLength == 2) {
				this->addMessage(this->clients[event.packet->data[1]]->getColoredNick() + " has left the server!");

				// Release selected and owned objects
				for (auto& object : this->objects) {
					if (object.second->isSelectedBy(this->clients[event.packet->data[1]])) {
						object.second->select(nullptr);
					}

					if (object.second->isOwnedBy(this->clients[event.packet->data[1]])) {
						object.second->own(nullptr);
					}
				}

				// Clear the client information
				delete this->clients.find(event.packet->data[1])->second;
				this->clients.erase(event.packet->data[1]);
			}

			break;
		}

		case net::PACKET_CHAT: {
			if (event.packet->dataLength >= 1 + 1 + 1 && event.packet->dataLength <= 1 + 1 + 1 + 255) {
				if (event.packet->data[1] == 255) {
					this->addMessage(std::string(reinterpret_cast<char*>(event.packet->data + 2), event.packet->dataLength - 2));
				} else {
					this->addMessage(net::clientIdToClient(this->clients, event.packet->data[1])->getColoredNick() + ": " +
										std::string(reinterpret_cast<char*>(event.packet->data + 2), event.packet->dataLength - 2));
				}
			}
			break;
		}

		case net::PACKET_CREATE: {
			if (event.packet->dataLength >= 1 + 6 + 8 + 1) {
				Client *client;
				if (event.packet->data[1] != 255) {
					client = this->clients.find(event.packet->data[1])->second;
				}

				int amount = 0;
				unsigned int i = 1;

				while (i < event.packet->dataLength - 1) {
					unsigned short objId = net::bytesToShort(event.packet->data + i + 1);
					Client *selected = net::clientIdToClient(this->clients, event.packet->data[i + 3]);
					Client *owner = net::clientIdToClient(this->clients, event.packet->data[i + 4]);
					bool flipped = event.packet->data[i + 5];
					Vector2 location = net::bytesToVector2(event.packet->data + i + 6);
					float rotation = event.packet->data[i + 14] * utils::PI / 8;
					unsigned char length = event.packet->data[i + 15];
					std::vector<std::string> objectData = utils::splitString(std::string(reinterpret_cast<char*>(event.packet->data + i + 16),
																			static_cast<int>(length)), '.');
					ObjectClass *objectClass = this->objectClassManager.getObjectClass(objectData.at(0), objectData.at(1));

					Object *object = new Object(objectClass, objectData.at(2), objId, location);
					object->initForClient(this->renderer);
					object->select(selected);
					object->own(owner);
					object->setFlipped(flipped);
					object->rotate(rotation);

					this->objects.insert(std::pair<unsigned int, Object*>(objId, object));
					this->objectOrder.push_back(object);

					amount++;
					this->checkObjectOrder();

					i += 15 + length;
				}
				if(event.packet->data[1] != 255) {
					this->addMessage(client->getColoredNick() + " created " + utils::toString(amount) + " objects.");
				}
			}
			break;
		}

		case net::PACKET_MOVE: {
			if (event.packet->dataLength >= 2 + 10) {
				Client *client = this->clients.find(event.packet->data[1])->second;

				unsigned int numberObjects = 0;
				Object *lastObject;

				size_t i = 2;
				while (i < event.packet->dataLength) {
					++numberObjects;

					unsigned short objId = net::bytesToShort(event.packet->data + i);
					Vector2 location = net::bytesToVector2(event.packet->data + i + 2);

					Object *object = this->objects.find(objId)->second;

					if (ANIMATION_TIME == 0) {
						object->setLocation(location);
					} else {
						object->setAnimation(location, ANIMATION_TIME);
					}

					net::removeObject(this->objectOrder, object);
					this->objectOrder.push_back(object);

					lastObject = object;
					i += 10;
				}

				if (lastObject->isOwnedBy(nullptr)) {
					if (numberObjects == 1) {
						this->addMessage(client->getColoredNick() + " moved " + lastObject->getName() + ".");
					} else if (numberObjects >= 2) {
						this->addMessage(client->getColoredNick() + " moved " + utils::toString(numberObjects) + " objects.");
					}
				}

				this->checkObjectOrder();
			}

			break;
		}

		case net::PACKET_SELECT: {
			if (event.packet->dataLength >= 2) {
				Client *client = this->clients.find(event.packet->data[1])->second;

				for (auto& object : this->objects) {
					if (object.second->isSelectedBy(client)) {
						object.second->select(nullptr);
					}
				}

				size_t i = 2;
				while (i < event.packet->dataLength) {
					unsigned short objId = net::bytesToShort(event.packet->data + i);

					this->objects.find(objId)->second->select(client);

					i += 2;
				}
			}

			break;
		}

		case net::PACKET_REMOVE: {
			if (event.packet->dataLength >= 2) {
				Client *client = this->clients.find(event.packet->data[1])->second;

				unsigned int numberObjects = 0;
				std::string lastObject;

				size_t i = 2;
				while (i < event.packet->dataLength) {
					++numberObjects;

					unsigned short objId = net::bytesToShort(event.packet->data + i);

					Object *object = this->objects.find(objId)->second;
					net::removeObject(this->objectOrder, object);
					this->objects.erase(objId);

					lastObject = object->getName();
					delete object;

					i += 2;
				}

				this->checkObjectOrder();

				if (numberObjects == 1) {
					this->addMessage(client->getColoredNick() + " removed " + lastObject + ".");
				} else if (numberObjects >= 2) {
					this->addMessage(client->getColoredNick() + " removed " + utils::toString(numberObjects) + " objects.");
				}
			}

			break;
		}

		case net::PACKET_FLIP: {
			if (event.packet->dataLength >= 2) {
				Client *client = this->clients.find(event.packet->data[1])->second;

				unsigned int numberObjects = 0;
				Object *lastObject;

				bool flipped = event.packet->data[2];

				size_t i = 3;
				while (i < event.packet->dataLength) {
					++numberObjects;

					unsigned short objId = net::bytesToShort(event.packet->data + i);

					Object *object = this->objects.find(objId)->second;
					object->setFlipped(flipped);

					lastObject = object;
					i += 2;
				}

				if (lastObject->isOwnedBy(nullptr)) {
					if (numberObjects == 1) {
						this->addMessage(client->getColoredNick() + " flipped " + lastObject->getName() + ".");
					} else if (numberObjects >= 2) {
						this->addMessage(client->getColoredNick() + " flipped " + utils::toString(numberObjects) + " objects.");
					}
				}
			}

			break;
		}

		case net::PACKET_OWN: {
			if (event.packet->dataLength >= 2) {
				Client *client = this->clients.find(event.packet->data[1])->second;

				unsigned int numberObjects = 0;
				Object *lastObject;

				bool owned = event.packet->data[2];

				size_t i = 3;
				while (i < event.packet->dataLength) {
					++numberObjects;

					unsigned short objId = net::bytesToShort(event.packet->data + i);

					Object *object = this->objects.find(objId)->second;
					if (owned) {
						object->own(client);
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
					this->addMessage(client->getColoredNick() + " " + verb + " " + lastObject->getName() + ".");
				} else if (numberObjects >= 2) {
					this->addMessage(client->getColoredNick() + " " + verb + " " + utils::toString(numberObjects) + " objects.");
				}

				this->checkObjectOrder();
			}

			break;
		}

		case net::PACKET_ROTATE: {
			unsigned short objId = net::bytesToShort(event.packet->data + 1);
			char rotation = event.packet->data[3];
			this->objects[objId]->rotate(rotation * utils::PI / 8);

			break;
		}

		case net::PACKET_PINGS: {
			if (event.packet->dataLength >= 4) {
				size_t i = 1;
				while (i < event.packet->dataLength) {
					if (this->clients.count(event.packet->data[i]) > 0) {
						this->clients[event.packet->data[i]]->ping = net::bytesToShort(event.packet->data + i + 1);
					}

					i += 3;
				}
			}

			break;
		}

		case net::PACKET_MS_QUERY: {
			if (this->connectionState == Game::ConnectionState::CONNECTED_MASTER_SERVER && event.packet->dataLength >= 1) {
				std::cout << "Received the server list!" << std::endl;

				// Disconnect from the master server
				enet_peer_disconnect_now(this->host, 0);
				enet_host_destroy(this->connection);
				this->connection = nullptr;

				this->connectionState = Game::ConnectionState::NOT_CONNECTED;
			}

			break;
		}
	}
}

void Game::addMessage(std::string text) {
	Message message;
	message.message = text;
	message.time = previousTime;
	std::cout << message.message << std::endl; //Todo: colors
	this->messages.push_back(message);
}

// Send a chat packet
void Game::sendChat(std::string text) {
	delete input;
	this->input = nullptr;

	if (this->connectionState == Game::ConnectionState::CONNECTED && text.length() > 0) {
		if (text.at(0) == '/') {
			this->chatCommand(text.substr(1));
		} else {
			std::string data;
			data.push_back(net::PACKET_CHAT);
			data.append(text);

			net::sendCommand(this->connection, data.c_str(), data.length());
		}

		if (this->sentMessages.size() == 0 || text != this->sentMessages.at(this->sentMessages.size() - 1)) {
			this->sentMessages.push_back(text);
		}
	}
}

std::string Game::getSentMessage(size_t index) {
	return this->sentMessages.at(this->sentMessages.size() - index);
}

size_t Game::getSentMessageCount() {
	return this->sentMessages.size();
}

void Game::chatCommand(std::string commandstr) {
	std::vector<std::string> parameters = utils::splitString(commandstr, ' ');

	if (parameters.at(0) == "load") {
		if (parameters.size() == 2) {
			this->loadScript(parameters.at(1));
		} else {
			this->addMessage("Usage: /" + parameters.at(0) + " script");
		}
	} else if (parameters.at(0) == "save") {
		if (parameters.size() == 2) {
			this->saveScript(parameters.at(1));
		} else {
			this->addMessage("Usage: /" + parameters.at(0) + " name");
		}
	} else if (parameters.at(0) == "create") {
		if (parameters.size() == 2) {
			std::string data;
			data.push_back(net::PACKET_CREATE);
			data += this->createObject(parameters.at(1));
			net::sendCommand(this->connection, data.c_str(), data.size());
		} else if (parameters.size() == 4) {
			Vector2 location;
			{
				std::istringstream stream(parameters.at(2));
				stream >> location.x;
			}
			{
				std::istringstream stream(parameters.at(3));
				stream >> location.y;
			}
			std::string data;
			data.push_back(net::PACKET_CREATE);
			data += this->createObject(parameters.at(1), location);
			net::sendCommand(this->connection, data.c_str(), data.size());
		} else {
			this->addMessage("Usage: /" + parameters.at(0) + " object [x y]");
		}
	} else if (parameters.at(0) == "dcreate") {
		if (parameters.size() == 2) {
			this->dCreateBuffer.push_back(parameters.at(1));
		} else {
			this->addMessage("Usage: /" + parameters.at(0) + " object");
		}
	} else if (parameters.at(0) == "dflush") {
		float x = 0.0f;
		std::string data;
		data.push_back(net::PACKET_CREATE);
		for(auto object : this->dCreateBuffer)
		{
			data += this->createObject(object, Vector2(x, 0.0f));

			x += 4;
		}
		net::sendCommand(this->connection, data.c_str(), data.size());
		this->dCreateBuffer.clear();
	} else if (parameters.at(0) == "roll") {
		if (parameters.size() <= 3) {
			unsigned short maxValue = 6;

			if (parameters.size() >= 2) {
				std::istringstream stream(parameters.at(1));
				stream >> maxValue;
			}

			std::string data;
			data.push_back(net::PACKET_ROLL);
			net::dataAppendShort(data, maxValue);
			net::sendCommand(this->connection, data.c_str(), data.length());
		} else {
			this->addMessage("Usage: /" + parameters.at(0) + " [max value]");
		}
	} else {
		this->addMessage(parameters.at(0) + ": command not found!");
	}
}

void Game::loadScript(std::string script) {
	std::vector<std::string> scriptPath = utils::splitString(script, '.');

	if (scriptPath.size() == 1 || scriptPath.size() == 2) {
		this->addMessage("Running script '" + script + "'.");

		std::ifstream file;
		if (scriptPath.size() == 1) {
			file.open("scripts/" + scriptPath.at(0) + ".txt");
		} else { // scriptPath.size() == 2
			file.open("data/" + scriptPath.at(0) + "/scripts/" + scriptPath.at(1) + ".txt");
		}

		std::string line;
		while (file.good()) {
			getline(file, line);
			if (line.length() == 0 || line.at(0) == '#') {
				// Ignore comment lines
			} else {
				this->chatCommand(line);
			}
		}

		file.close();
	} else {
		this->addMessage("Could not run script '" + script + "'.");
	}
}

void Game::saveScript(std::string name) {
	this->addMessage("Saving objects to script '" + name + "'.");

	std::ofstream file;
	file.open("scripts/" + name + ".txt");

	for (auto& object : this->objectOrder) {
		file << "create " << object->getFullId() << " " << object->getLocation().x << " " << object->getLocation().y << std::endl;
	}

	file.close();
}

std::string Game::createObject(std::string objectId, Vector2 location) {
	std::string data;
	data += 255; // Not selected
	data += 255; // Not owned
	bool flipped = false;
	data += flipped; // Not flipped
	net::dataAppendVector2(data, location);
	data.push_back(0x00); // Not rotated
	data += (objectId.size());
	data.append(objectId);
	return data;
}

void Game::checkObjectOrder() {
	for (auto& object : this->objectOrder) {
		object->checkIfUnder(this->objectOrder);
	}
}

void Game::askNick() {
	this->input = new InputBox(this, &Game::identifyToServer, "Nick", Vector2(2.0f, this->renderer->getDisplaySize().y / 2.0f + 20.0f), 225.0f,
	                           this->renderer->getFont(), 16);
}

void Game::identifyToServer(std::string nick) {
	delete input;
	this->input = nullptr;

	if (nick.length() > 0) {
		std::string data;
		data.push_back(net::PACKET_HANDSHAKE);
		data.append(nick, 0, 16); // Limit nick to 16 characters

		net::sendCommand(this->connection, data.c_str(), data.length());
	} else {
		this->askNick();
	}
}

void Game::queryMasterServer() {
	std::string data;
	data.push_back(net::PACKET_MS_QUERY);
	net::sendCommand(this->connection, data.c_str(), data.length());
}

void Game::update() {
	// Calculate deltaTime
	this->deltaTime = al_get_time() - this->previousTime;
	this->previousTime = al_get_time();

	// Animate objects
	for (auto& object : this->objectOrder) {
		object->animate(this->deltaTime);
	}

	// Translate, rotate and scale the screen
	if (this->keyStatus.screenZoomIn) {
		this->renderer->zoomScreen(1.05f);
	}
	if (this->keyStatus.screenZoomOut) {
		this->renderer->zoomScreen(1.0f / 1.05f);
	}

	if (this->keyStatus.screenMoveLeft) {
		this->renderer->scrollScreen(Vector2(10.0f, 0.0f) * this->deltaTime * 60.0f);
	}
	if (this->keyStatus.screenMoveRight) {
		this->renderer->scrollScreen(Vector2(-10.0f, 0.0f) * this->deltaTime * 60.0f);
	}
	if (this->keyStatus.screenMoveUp) {
		this->renderer->scrollScreen(Vector2(0.0f, 10.0f) * this->deltaTime * 60.0f);
	}
	if (this->keyStatus.screenMoveDown) {
		this->renderer->scrollScreen(Vector2(0.0f, -10.0f) * this->deltaTime * 60.0f);
	}

	if (this->keyStatus.screenRotateClockwise) {
		this->renderer->rotateScreen(utils::PI / 40.0f * this->deltaTime * 60.0f);
	}
	if (this->keyStatus.screenRotateCClockwise) {
		this->renderer->rotateScreen(-utils::PI / 40.0f * this->deltaTime * 60.0f);
	}

	this->renderer->updateTransformations();
}

void Game::render() {
	// Clear the screen
	al_clear_to_color(al_map_rgb_f(0.1f, 0.1f, 0.1f));

	// Begin render
	al_hold_bitmap_drawing(true);

	this->renderGame();
	this->renderUI();

	// Finish render
	al_hold_bitmap_drawing(false);

	// Draw the rendering
	al_flip_display();
}

void Game::renderGame() {
	this->renderer->useTransformation(IRenderer::CAMERA);

	// Draw the table area
	this->renderer->drawRectangle(Vector2(-net::MAX_FLOAT, -net::MAX_FLOAT), Vector2(net::MAX_FLOAT, net::MAX_FLOAT), Color(1.0f, 1.0f, 1.0f, 1.0f), 5.0f);

	for (auto& object : this->objectOrder) {
		object->draw(this->renderer, this->clients.find(this->localClient)->second);
	}
}

void Game::renderUI() {
	this->renderer->useTransformation(IRenderer::UI);

	int i = 0;
	for (auto& client : this->clients) {
		if (client.second->isJoined()) {
			std::string text;
			if (client.second->ping != 65535) {
				text = client.second->getColoredNick() + " (" + utils::toString(client.second->ping) + " ms)";
			} else {
				text = client.second->getColoredNick();
			}

			this->renderer->drawText(text, Vector2(0.0f, i * 20.0f));
		}
		++i;
	}

	for (std::vector<Message>::size_type i = this->messages.size(); i > 0; --i) {
		if (this->input != nullptr || al_get_time() < this->messages[i-1].time + MESSAGE_TIME) {
			this->renderer->drawText(this->messages[i-1].message, Vector2(0.0f, this->renderer->getDisplaySize().y / 2.0f + i * 20.0f - this->messages.size() * 20.0f));
		}
	}

	this->renderer->drawText("FPS: " + utils::toString( static_cast<int>(1.0 / this->deltaTime + 0.25)),
	                         Vector2(0.0f, this->renderer->getDisplaySize().y - 20.0f));

	for (auto& widget : this->widgets) {
		widget->draw(this->renderer);
	}

	if (this->input != nullptr) {
		input->draw(this->renderer);
	}
}
