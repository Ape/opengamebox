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

	// Unix-like systems such as Linux _need_ to pass argv[0] from main() in here.
	if (PHYSFS_init(argv[0]) == 0) {
		std::cout << "Failed to initialize physfs" << std::cout;
		return EXIT_FAILURE;
	}

	PHYSFS_setWriteDir("data/");

	Game game;
	Game::gamePtr = &game;

	if (!game.init()) {
		return EXIT_FAILURE;
	}

	if (argc == 1) {
		if (game.connectMasterServer()) {
			return game.run();
		} else {
			return EXIT_FAILURE;
		}
	}

	address = std::string(argv[1]);

	if (argc >= 3) {
		std::istringstream stream(argv[2]);
		stream >> port;
	} else {
		port = 0;
	}

	if (game.connect(address, port)) {
		return game.run();
	} else {
		return EXIT_FAILURE;
	}
}

Game *Game::gamePtr;

void Game::catchSignal(int signal) {
	std::cout << std::endl << "Exiting.." << std::endl;

	Game::gamePtr->quit();
}

Game::Game() {
	this->settings = new Settings("opengamebox.cfg");
	this->state = State::INITIALIZING;
	this->connectionState = ConnectionState::NOT_CONNECTED;
	this->nextFrame = true;
	this->deltaTime = 0.0f;
	this->input = nullptr;
	this->localClient = net::MAX_CLIENTS;

	this->dragging = false;
	this->selecting = false;
	this->keyStatus = KeyStatus();

	this->loadingPackage = false;

}

Game::~Game() {
	delete this->settings;
}

bool Game::init() {
	// Catch SIGINT
	signal(SIGINT, catchSignal);

	// Initialize Allegro
	if (! al_init()) {
		std::cerr << "Error: Failed to initialize Allegro!" << std::endl;
		return false;
	}

	al_set_physfs_file_interface();
	PHYSFS_addToSearchPath(".", 1);

	// Reset frame timestamp
	this->previousTime = al_get_time();

	// Initialize input
	if (! al_install_keyboard() || ! al_install_mouse()) {
		std::cerr << "Error: Failed to initialize the input components!" << std::endl;
		return false;
	}

	// Set the window mode
	if (this->settings->getValue<bool>("display.fullscreen")) {
		al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
	} else {
		al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
	}

	// Initialize the renderer
	this->renderer = new Renderer(Coordinates(this->settings->getValue<int>("display.width"),
	                              this->settings->getValue<int>("display.height")),
	                              this->settings->getValue<int>("display.multisamples"));

	// Set window title
	this->renderer->setWindowTitle("OpenGamebox", "gfx/icon");

	// Create a timer for the main loop
	this->timer = al_create_timer(1.0f / this->settings->getValue<float>("display.fpslimit"));
	if (! this->timer) {
		std::cerr << "Error: Failed to create a timer!" << std::endl;
		return false;
	}

	// Create an event queue
	this->event_queue = al_create_event_queue();
	if (! this->event_queue) {
		std::cerr << "Error: Failed to create an event queue!" << std::endl;
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
		std::cerr << "Error: Failed to initialize the network components!" << std::endl;
		return false;
	}

	return true;
}

bool Game::connect(std::string address, int port) {
	if (this->connectionState == ConnectionState::CONNECTED) {
		this->addMessage("You are already connected!", MessageType::ERROR);
		return false;
	} else if (this->connectionState == ConnectionState::CONNECTING_MASTER_SERVER
	           || this->connectionState == ConnectionState::CONNECTED_MASTER_SERVER) {
		this->disconnectMasterServer();
	}

	this->connection = enet_host_create (nullptr,          // Create a client host
	                                     1,             // Only allow 1 outgoing connection
	                                     net::CHANNELS, // Number of channels
	                                     0,             // Unlimited downstream bandwidth
	                                     0);            // Unlimited upstream bandwidth

	if (enet_address_set_host(&(this->hostAddress), address.c_str()) < 0) {
		this->addMessage("Unknown host!", MessageType::ERROR);
		return false;
	}

	if (port == 0) {
		port = this->settings->getValue<int>("network.port");
	}

	this->hostAddress.port = port;

	if (this->connection == nullptr) {
		this->addMessage("Could not create a connection!", MessageType::ERROR);
		return false;
	}

	// Initialize the connection
	this->addMessage("Connecting to " + net::AddressToString(this->hostAddress) + "...");
	host = enet_host_connect(this->connection, &this->hostAddress, 1, 0);

	if (this->host == nullptr) {
		this->addMessage("Could not connect to the server!", MessageType::ERROR);
		return false;
	}

	// Set ping interval for the connection, this is only supported in ENet >= 1.3.4
	#if ENET_VERSION >= ENET_VERSION_CREATE(1, 3, 4)
	enet_peer_ping_interval(this->host, net::PING_INTERVAL);
	#endif

	this->connectionState = ConnectionState::CONNECTING;

	return true;
}

bool Game::connectMasterServer() {
	return this->connectMasterServer(this->settings->getValue<std::string>("network.masterserver"),
	                                 this->settings->getValue<int>("network.masterserverport"));
}

bool Game::connectMasterServer(std::string address, int port) {
	// Create a connection to the master server
	this->connection = enet_host_create (nullptr, 1, 1, 0, 0);

	if (enet_address_set_host(&(this->hostAddress), address.c_str()) < 0) {
		this->addMessage("Unknown master server host!", MessageType::ERROR);
		return false;
	}

	if (port == 0) {
		this->addMessage("Illegal master server port number!", MessageType::ERROR);
		return false;
	}
	this->hostAddress.port = port;

	if (this->connection == nullptr) {
		this->addMessage("Could not create a connection to the master server!", MessageType::ERROR);
		return false;
	}

	// Initialize the connection
	this->addMessage("Connecting to " + net::AddressToString(this->hostAddress) + " (master server)...");
	host = enet_host_connect(this->connection, &this->hostAddress, 1, 0);

	if (this->host == nullptr) {
		this->addMessage("Could not connect to the master server!", MessageType::ERROR);
		return false;
	}

	this->connectionState = ConnectionState::CONNECTING_MASTER_SERVER;

	return true;
}

int Game::run() {
	this->state = State::RUNNING;

	// Enter the main loop
	this->mainLoop();

	// Exit the application when returned from the main loop
	this->dispose();
	return EXIT_SUCCESS;
}

void Game::mainLoop() {
	while (this->state != State::TERMINATED) {
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
	if (this->connectionState == ConnectionState::CONNECTED) {
		this->state = State::EXITING;
		this->disconnect();
	} else {
		this->state = State::TERMINATED;
	}
}

void Game::disconnect() {
	if (this->connectionState == ConnectionState::CONNECTED
	    || this->connectionState == ConnectionState::CONNECTING) {
		this->addMessage("Disconnecting...");
		enet_peer_disconnect(this->host, 0);

		this->connectionState = ConnectionState::DISCONNECTING;
	} else if (this->connectionState == ConnectionState::CONNECTED_MASTER_SERVER
	           || this->connectionState == ConnectionState::CONNECTING_MASTER_SERVER) {
		this->addMessage("Disconnected from the master server.");
		this->disconnectMasterServer();
	} else {
		this->addMessage("You are not connected!", MessageType::ERROR);
	}
}

void Game::disconnectMasterServer() {
	enet_peer_disconnect_now(this->host, 0);
	enet_host_destroy(this->connection);
	this->connection = nullptr;

	this->connectionState = ConnectionState::NOT_CONNECTED;
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

						object->setAnimation(nextLocation, this->settings->getValue<float>("game.animationtime"));
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
		} else { //Boxselect
			for (auto& object : this->selectedObjects) {
				object->select(nullptr);
			}
			this->selectedObjects.clear();
			this->selecting = true;
			this->selectingStart = location;
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
		Vector2 location(event.mouse.x, event.mouse.y);
		this->renderer->transformLocation(IRenderer::CAMERA_INVERSE, location);
		if (this->dragging) {


			std::string data;
			data.push_back(net::PACKET_MOVE);

			for (auto& object : this->selectedObjects) {
				net::dataAppendShort(data, object->getId());
				net::dataAppendVector2(data, object->getLocation());

				object->setAnimation(object->getLocation(), 0.0f);
			}

			net::sendCommand(this->connection, data.c_str(), data.length());

			this->dragging = false;
		} else if (this->selecting) {
			this->selectedObjects.clear();
			this->selecting = false;
			Vector2 lower;
			Vector2 higher;
			if (this->selectingStart.x > location.x) {
				lower.x = location.x;
				higher.x = this->selectingStart.x;
			} else {
				lower.x = this->selectingStart.x;
				higher.x = location.x;
			}
			if (this->selectingStart.y > location.y) {
				lower.y = location.y;
				higher.y = this->selectingStart.y;
			} else {
				lower.y = this->selectingStart.y;
				higher.y = location.y;
			}
			for (auto& object : this->objects)
			{
				std::list<Vector2> corners = object.second->getCorners();
				for (auto corner : corners)
				{
					if (lower.x < corner.x && corner.x < higher.x && lower.y < corner.y && corner.y < higher.y) {
						object.second->select(this->clients.find(localClient)->second);
						this->selectedObjects.push_back(object.second);
						break;
					}
				}
			}
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

	while (this->connectionState != ConnectionState::NOT_CONNECTED && enet_host_service(this->connection, &event, 0) > 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT: {
				if (this->connectionState == ConnectionState::CONNECTING) {
					this->addMessage("Connected! Please enter your nick.");

					this->connectionState = ConnectionState::CONNECTED;
					this->askNick();
				} else if (this->connectionState == ConnectionState::CONNECTING_MASTER_SERVER) {
					this->addMessage("Connected to the master server.");

					this->connectionState = ConnectionState::CONNECTED_MASTER_SERVER;
					this->queryMasterServer();
				}

				break;

			}

			case ENET_EVENT_TYPE_RECEIVE: {
				if (this->connectionState == ConnectionState::CONNECTED_MASTER_SERVER) {
					this->receivePacket(event);
				} else if (this->connectionState == ConnectionState::CONNECTED) {
					if (this->localClient != net::MAX_CLIENTS || event.packet->data[0] == net::PACKET_HANDSHAKE ||
					    event.packet->data[0] == net::PACKET_NICK_TAKEN) {
						this->receivePacket(event);
					} else if (event.packet->data[0] == net::PACKET_MS_QUERY) {
						this->addMessage("Can't use a master server as a game server!", MessageType::ERROR);
						this->disconnectMasterServer();
					}
				}

				enet_packet_destroy(event.packet);

				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT: {
				this->addMessage("Disconnected from the server.");
				this->connectionState = ConnectionState::NOT_CONNECTED;

				if (this->state == State::EXITING) {
					this->state = State::TERMINATED;
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
	Packet packet(event.packet);

	try {
		switch (packet.readHeader()) {
			case Packet::Header::HANDSHAKE: {
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

			case Packet::Header::NICK_TAKEN: {
				if (event.packet->dataLength == 1) {
					this->addMessage("That nick is already reserved!");

					this->askNick();
				}

				break;
			}

			case Packet::Header::JOIN: {
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

			case Packet::Header::LEAVE: {
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

			case Packet::Header::CHAT: {
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

			case Packet::Header::CREATE: {
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
						ObjectClass *objectClass = this->objectClassManager.getObjectClass(objectData.at(0), objectData.at(1), &(this->missingPackages));
						std::cout<<this->missingPackages.size()<<std::endl;;
						if (!this->loadingPackage && !this->missingPackages.empty()){
							Packet packet(this->connection);
							packet.writeHeader(Packet::Header::PACKAGE_MISSING);
							packet.writeString(this->missingPackages.front());
							packet.send();
						}

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

			case Packet::Header::MOVE: {
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

						if (this->settings->getValue<float>("game.animationtime") == 0) {
							object->setLocation(location);
						} else {
							object->setAnimation(location, this->settings->getValue<float>("game.animationtime"));
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

			case Packet::Header::SELECT: {
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

			case Packet::Header::REMOVE: {
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

			case Packet::Header::FLIP: {
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

			case Packet::Header::OWN: {
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

			case Packet::Header::ROTATE: {
				unsigned short objId = net::bytesToShort(event.packet->data + 1);
				char rotation = event.packet->data[3];
				this->objects[objId]->rotate(rotation * utils::PI / 8);

				break;
			}

			case Packet::Header::PINGS: {
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

			case Packet::Header::MS_QUERY: {
				if (this->connectionState == ConnectionState::CONNECTED_MASTER_SERVER) {
					this->addMessage("Server list:");

					while (!packet.eof()) {
						std::string address = packet.readString();
						unsigned short port = packet.readShort();
						std::string name = packet.readString();
						unsigned short players = packet.readShort();

						std::ostringstream server;
						server << "\"" << name << "\" @ " << address << ":" << port << " (" << players << " players)";
						this->addMessage(server.str());
					}

					this->disconnectMasterServer();
				}

				break;
			}

			case Packet::Header::FILE_TRANSFER: {
				std::cout<<"filetransfer"<<std::cout;
				if (packet.readShort() == 0) {
					this->loadingfile.size = packet.readInt();
					this->loadingfile.data = new char[this->loadingfile.size];
					this->loadingfile.name = packet.readString();
					this->loadingfile.recieved = 0;
				} else {
					int ofset = packet.readInt();
					int size = packet.readInt();
					std::string tmp = packet.readString();
					for (unsigned int i = 0; i < tmp.size(); i++) {
						this->loadingfile.data[ofset + i] = tmp.at(i);
					}
					this->loadingfile.recieved += size;
				}
				if (this->loadingfile.size == this->loadingfile.recieved) {
					PHYSFS_File *file = PHYSFS_openWrite(this->loadingfile.name.c_str());
					PHYSFS_write(file, this->loadingfile.data, sizeof(char), this->loadingfile.size);
					PHYSFS_flush(file);
				}
				break;
			}

			default: {
				throw PacketException("Invalid packet header.");
			}
		}
	} catch (PacketException &e) {
		this->addMessage("Received an invalid packet from "
		                 + net::AddressToString(event.peer->address)
		                 + ": \"" + e.what() + "\"", MessageType::DEBUG);

		if (this->connectionState == ConnectionState::CONNECTED_MASTER_SERVER) {
			this->disconnectMasterServer();
		}
	}
}

void Game::addMessage(std::string text, MessageType type) {
	if (type == MessageType::WARNING) {
		text = std::string("^ff0Warning: ") + text;
	} else if (type == MessageType::ERROR) {
		text = std::string("^f00Error: ") + text;
	} else if (type == MessageType::DEBUG) {
		text = std::string("^0ffDebug: ") + text;
	}

	Message message;
	message.message = text;
	message.time = previousTime;
	std::cout << message.message << std::endl; // TODO: Remove colors
	this->messages.push_back(message);
}

// Send a chat packet
void Game::sendChat(std::string text) {
	delete input;
	this->input = nullptr;

	if (text.length() > 0) {
		if (text.at(0) == '/') {
			this->chatCommand(text.substr(1));
		} else if (this->connectionState == ConnectionState::CONNECTED) {
			std::string data;
			data.push_back(net::PACKET_CHAT);
			data.append(text);

			net::sendCommand(this->connection, data.c_str(), data.length());
		} else {
			this->addMessage("You are not connected to a server!", MessageType::ERROR);
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

	if (parameters.at(0) == "quit") {
		this->quit();
	} else if (parameters.at(0) == "connect") {
		if (parameters.size() == 1) {
			this->addMessage("Usage: /" + parameters.at(0) + " host [port]");
			return;
		}

		unsigned int port;

		if (parameters.size() == 3) {
			std::istringstream stream(parameters[2]);
			stream >> port;
		} else {
			port = this->settings->getValue<int>("network.port");
		}

		this->connect(parameters[1], port);
	} else if (parameters.at(0) == "disconnect") {
		this->disconnect();
	} else if (parameters.at(0) == "servers") {
		if (this->connectionState == ConnectionState::NOT_CONNECTED) {
			this->connectMasterServer();
		} else {
			this->addMessage("You are already connected to a server. Please disconnect first.");
		}
	} else if (parameters.at(0) == "load") {
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
		this->addMessage(parameters.at(0) + ": command not found!", MessageType::ERROR);
	}
}

void Game::loadScript(std::string script) {
	std::vector<std::string> scriptPath = utils::splitString(script, '.');

	if (scriptPath.size() == 1 || scriptPath.size() == 2) {
		this->addMessage("Running script '" + script + "'.");

		std::istringstream file;
		if (scriptPath.size() == 1) {
			std::ifstream scriptFile;
			scriptFile.open("scripts/" + scriptPath.at(0) + ".txt");
			file.str(std::string((std::istreambuf_iterator<char>(scriptFile)), std::istreambuf_iterator<char>()));
		} else { // scriptPath.size() == 2
			file.str(utils::getTextFile(scriptPath.at(0), "scripts/" + scriptPath.at(1) + ".txt"));
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

	} else {
		this->addMessage("Could not run script '" + script + "'.", MessageType::ERROR);
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
	Packet packet(this->connection);
	packet.writeHeader(Packet::Header::MS_QUERY);
	packet.send();
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

	if (this->selecting) {
		Vector2 location = this->selectingStart;
		ALLEGRO_MOUSE_STATE mouse;
		al_get_mouse_state(&mouse);
		this->renderer->transformLocation(IRenderer::CAMERA, location);
		this->renderer->drawRectangle(location, Vector2(mouse.x, mouse.y), Color(1.0f, 1.0f, 1.0f), 1.0f);
	}

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
		if (this->input != nullptr || al_get_time() < this->messages[i-1].time + this->settings->getValue<float>("game.messagetime")) {
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
