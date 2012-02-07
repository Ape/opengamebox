#include "main.h"

int main(int argc, char **argv){
	// Get host address and port from command line arguments
	std::string address;
	int port;

	if (argc >= 2){
		address = std::string(argv[1]);
	}else{
		address = "localhost";
	}

	if (argc >= 3){
		std::istringstream stream(argv[2]);
		stream >> port;
	}else{
		port = net::DEFAULT_PORT;
	}

	Game game;
	return game.run(address, port);
}

Game::Game(void){
	this->exiting = false;
	this->redraw = true;
	this->disconnecting = false;
	this->input = nullptr;

	this->dragging = false;
	this->movingScreen = false;

	this->localClient = net::MAX_CLIENTS;
}

int Game::run(std::string address, int port){
	// Initialize enet
	if (enet_initialize() != 0){
		std::cerr << "Failed to initialize the network components!" << std::endl;
		return EXIT_FAILURE;
	}

	this->connection = enet_host_create (NULL,          // Create a client host
	                                     1,             // Only allow 1 outgoing connection
	                                     net::CHANNELS, // Number of channels
	                                     100000,        // Downstream bandwidth limited to 100 kB/s
	                                     100000);       // Upstream bandwidth limited to 100 kB/s

	if (enet_address_set_host(&(this->hostAddress), address.c_str()) < 0){
		std::cerr << "Unknown host!" << std::endl;
		return EXIT_FAILURE;
	}

	if (port == 0){
		std::cerr << "Illegal port number!" << std::endl;
		return EXIT_FAILURE;
	}
	this->hostAddress.port = port;

	// Initialize Allegro
	if (! al_init()){
		std::cerr << "Failed to initialize Allegro!" << std::endl;
		return EXIT_FAILURE;
	}

	// Initialize input
	if (! al_install_keyboard() || ! al_install_mouse()){
		std::cerr << "Failed to initialize the input components!" << std::endl;
		return EXIT_FAILURE;
	}

	// Set the window mode
	if (FULLSCREEN){
		al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
	}else{
		al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
	}

	// Create the window
	this->display = al_create_display(SCREEN_W, SCREEN_H);
	if (! this->display){
		std::cerr << "Failed to create a display!" << std::endl;
		return EXIT_FAILURE;
	}
	al_acknowledge_resize(this->display);

	// Initialize the renderer
	this->renderer = new Renderer(Vector2(al_get_display_width(this->display), al_get_display_height(this->display)));

	// Initialize fonts
	al_init_font_addon();
	if (! al_init_ttf_addon()){
		std::cerr << "Failed to initialize fonts!" << std::endl;
		return EXIT_FAILURE;
	}

	this->font = al_load_ttf_font("res/LiberationSans-Regular.ttf", 16, 0);

	// Initialize image libraries
	if (! al_init_image_addon()){
		std::cerr << "Failed to initialize image libraries!" << std::endl;
		return EXIT_FAILURE;
	}

	if (!al_init_primitives_addon()){
		std::cerr << "Failed to initialize primitives_addon!" << std::endl;
		return EXIT_FAILURE;
	}

	// Create a timer for the main loop
	this->timer = al_create_timer(1.0f / FPS_LIMIT);
	if (! this->timer){
		std::cerr << "Failed to create a timer!" << std::endl;
		return EXIT_FAILURE;
	}

	// Create an event queue
	this->event_queue = al_create_event_queue();
	if (! this->event_queue){
		std::cerr << "Failed to create an event queue!" << std::endl;
		return EXIT_FAILURE;
	}

	// Register event sources
	al_register_event_source(this->event_queue, al_get_display_event_source(this->display));
	al_register_event_source(this->event_queue, al_get_timer_event_source(this->timer));
	al_register_event_source(this->event_queue, al_get_keyboard_event_source());
	al_register_event_source(this->event_queue, al_get_mouse_event_source());

	al_start_timer(this->timer);

	if (this->connection == NULL){
		std::cerr << "Could not create a network peer!" << std::endl;
		return EXIT_FAILURE;
	}

	// Initialize the connection
	this->addMessage("Connecting to " + net::AddressToString(this->hostAddress) + "...");
	host = enet_host_connect(connection, &this->hostAddress, 1, 0);

	if (this->host == NULL){
		std::cerr << "Could not connect to the server." << std::endl;
		return EXIT_FAILURE;
	}

	// Enter the main loop
	this->mainLoop();

	// Exit the application
	this->dispose();
	return EXIT_SUCCESS;
}

void Game::mainLoop(){
	while (! this->exiting){
		// Process network events
		this->networkEvents();

		// Handle local events
		this->localEvents();

		// Render the screen with limited FPS
		if (this->redraw && al_is_event_queue_empty(this->event_queue)){
			this->redraw = false;
			this->render();
		}
	}
}

void Game::quit(){
	if (! this->disconnecting){
		this->addMessage("Disconnecting...");
		this->disconnecting = true;
		enet_peer_disconnect(this->host, 0);
	}else{
		this->exiting = true;
	}
}

void Game::dispose(){
	enet_host_destroy(this->connection);
	enet_deinitialize();

	delete this->renderer;

	al_destroy_timer(this->timer);
	al_destroy_display(this->display);
	al_destroy_event_queue(this->event_queue);
}

void Game::localEvents(){
	ALLEGRO_EVENT event;
	al_wait_for_event(event_queue, &event);

	if (event.type == ALLEGRO_EVENT_TIMER){
		this->redraw = true;
	}else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
		this->quit();
	}else if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE){
		this->resize();
	}else if (event.type == ALLEGRO_EVENT_KEY_CHAR){
		if (input != nullptr){
			this->input->onKey(event.keyboard);
		}else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER){
			// Create a new chat input widget
			this->input = new InputBox(this, &Game::sendChat, Vector2(SCREEN_H - 40, 0), Vector2(24, 150), this->font, 255);
		}
	}else if (event.type == ALLEGRO_EVENT_KEY_DOWN){
		if (input == nullptr){
			if (event.keyboard.keycode == ALLEGRO_KEY_F10){
				this->quit();
			}else if (event.keyboard.keycode == ALLEGRO_KEY_SPACE){
				for (auto& object : this->selectedObjects){
					if (object->isOwnedBy(this->clients.find(localClient)->second)){
						object->own(nullptr);
					}else if (object->isOwnedBy(nullptr)){
						object->own(this->clients.find(localClient)->second);
					}
				}
			}else if (event.keyboard.keycode == ALLEGRO_KEY_C){
				this->chatCommand("create card_7c");
				this->chatCommand("create card_Kh");
				this->chatCommand("create card_As");
			}else if (event.keyboard.keycode == ALLEGRO_KEY_V){
				this->chatCommand("create chessboard");
				for (int i = 0; i < 12; ++i){
					this->chatCommand("create piece_red");
					this->chatCommand("create piece_blue");
				}
			}else if (event.keyboard.keycode == ALLEGRO_KEY_S){
				if (this->dragging){
					std::vector<Object*> objects;
					std::vector<Vector2> locations;
					for (auto& object : this->selectedObjects){
						objects.push_back(object);
						locations.push_back(object->getLocation());
					}

					std::random_shuffle(objects.begin(), objects.end());

					this->selectedObjects.clear();
					std::vector<Vector2>::size_type location = 0;
					for (auto& object : objects){
						object->setLocation(locations.at(location));
						++location;
						this->selectedObjects.push_back(object);

						net::removeObject(this->objectOrder, object);
						this->objectOrder.push_back(object);
					}

					this->checkObjectOrder();
				}
			}else if (event.keyboard.keycode == ALLEGRO_KEY_F){
				if (this->dragging){
					Vector2 nextLocation = this->selectedObjects.front()->getLocation();
					for (auto& object : this->selectedObjects){
						object->setLocation(nextLocation);
						nextLocation += object->getStackDelta();
					}
				}
			}else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT){
				this->renderer->addScreenLocation(Vector2(10.0f, 0.0f));
				this->renderer->updateTransformations();
			}else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT){
				this->renderer->addScreenLocation(Vector2(-10.0f, 0.0f));
				this->renderer->updateTransformations();
			}else if (event.keyboard.keycode == ALLEGRO_KEY_UP){
				this->renderer->addScreenLocation(Vector2(0.0f, 10.0f));
				this->renderer->updateTransformations();
			}else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN){
				this->renderer->addScreenLocation(Vector2(0.0f, -10.0f));
				this->renderer->updateTransformations();
			}
		}
	}else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1){
		Vector2 location(event.mouse.x, event.mouse.y);
		this->renderer->transformLocation(CAMERA_INVERSE, location);

		if (! this->dragging){
			for (auto& object : this->selectedObjects){
				object->select(nullptr);
			}
			this->selectedObjects.clear();

			std::string data;
			data.push_back(net::PACKET_SELECT);

			for (auto object = this->objectOrder.rbegin(); object != this->objectOrder.rend(); ++object){
				if ((*object)->isSelectedBy(nullptr) && (*object)->testLocation(location)){
					std::set<Object*> visited;
					this->selectedObjects = (*object)->getObjectsAbove(visited);

					if (this->selectedObjects.size() == 1 && this->selectedObjects.front() == nullptr){
						this->selectedObjects.clear();
					}

					for (auto& objectA : this->selectedObjects){
						net::dataAppendShort(data, objectA->getId());

						net::removeObject(this->objectOrder, objectA);
						this->objectOrder.push_back(objectA);
					}

					break;
				}
			}

			net::sendCommand(connection, data.c_str(), data.length());
		}

		if (!this->selectedObjects.empty()){
			for (auto& object : this->selectedObjects){
				if (object->testLocation(location)){
					this->dragging = true;
					this->draggingStart = location;

					break;
				}
			}
		}
	}else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 2){
		if (this->dragging){
			std::string data;
			data.push_back(net::PACKET_FLIP);

			for (auto& object : this->selectedObjects){
				net::dataAppendShort(data, object->getId());
			}

			net::sendCommand(connection, data.c_str(), data.length());
		}
	}else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 3){
		this->movingScreen = true;
		this->movingScreenStart = Vector2(event.mouse.x, event.mouse.y);
	}else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 1){
		if (this->dragging){
			Vector2 location(event.mouse.x, event.mouse.y);
			this->renderer->transformLocation(CAMERA_INVERSE, location);

			std::string data;
			data.push_back(net::PACKET_MOVE);

			for (auto& object : this->selectedObjects){
				net::dataAppendShort(data, object->getId());
				net::dataAppendVector2(data, object->getLocation());
			}

			net::sendCommand(connection, data.c_str(), data.length());

			this->dragging = false;
		}
	}else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 3){
		this->movingScreen = false;
	}else if (event.type == ALLEGRO_EVENT_MOUSE_AXES){
		if (event.mouse.dz != 0){
			this->renderer->mulScreenZoom(1 - 0.1f * event.mouse.dz);
			this->renderer->updateTransformations();
		}else if (this->dragging && (event.mouse.dx != 0 || event.mouse.dy != 0)){
			Vector2 location(event.mouse.x, event.mouse.y);
			this->renderer->transformLocation(CAMERA_INVERSE, location);

			for (auto& object : this->selectedObjects){
				Vector2 destination = object->getLocation() + (location - this->draggingStart);

				if (destination.x > net::MAX_FLOAT){
					destination.x = net::MAX_FLOAT;
				}
				if (destination.x < -net::MAX_FLOAT){
					destination.x = -net::MAX_FLOAT;
				}
				if (destination.y > net::MAX_FLOAT){
					destination.y = net::MAX_FLOAT;
				}
				if (destination.y < -net::MAX_FLOAT){
					destination.y = -net::MAX_FLOAT;
				}

				object->setLocation(destination);
			}
			this->draggingStart = location;
		}else if (this->movingScreen && (event.mouse.dx != 0 || event.mouse.dy != 0)){
			Vector2 location(event.mouse.x, event.mouse.y);

			this->renderer->addScreenLocation(location - this->movingScreenStart);
			this->movingScreenStart = location;

			this->renderer->updateTransformations();
		}
	}
}

void Game::networkEvents(){
	ENetEvent event;

	while (enet_host_service(this->connection, &event, 0) > 0){
		switch (event.type){
			case ENET_EVENT_TYPE_CONNECT:{
				this->addMessage("Connected! Please enter your nick.");

				this->askNick();
				break;

			}

			case ENET_EVENT_TYPE_RECEIVE:{
				if (! this->disconnecting && (this->localClient != net::MAX_CLIENTS
				    || event.packet->data[0] == net::PACKET_HANDSHAKE || event.packet->data[0] == net::PACKET_NICK_TAKEN)){
					this->receivePacket(event);
				}

				enet_packet_destroy(event.packet);
				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT:{
				this->addMessage("Disconnected from the server.");

				this->exiting = true;
				break;
			}

			case ENET_EVENT_TYPE_NONE:{
				break;
			}
		}

	}
}

void Game::receivePacket(ENetEvent event){
	switch (event.packet->data[0]){
		case net::PACKET_HANDSHAKE:{
			if (event.packet->dataLength >= 2 && event.packet->dataLength <= 2 + 34*(net::MAX_CLIENTS - 1)){
				// Store the received client id
				this->localClient = event.packet->data[1];

				// Update the local client list
				size_t i = 2;
				while (i < event.packet->dataLength){
					net::Client *client = new net::Client(std::string((char*) event.packet->data + i + 2, event.packet->data[i + 1]));
					client->id = event.packet->data[i];
					this->clients[event.packet->data[i]] = client;

					i += 2 + event.packet->data[i + 1];
				}
			}

			break;
		}

		case net::PACKET_NICK_TAKEN:{
			if (event.packet->dataLength == 1){
				this->addMessage("That nick is already reserved!");

				this->askNick();
			}

			break;
		}

		case net::PACKET_JOIN:{
			if (event.packet->dataLength >= 3 && event.packet->dataLength <= 35){
				// Store the client information
				net::Client *client = new net::Client(std::string((char*) event.packet->data + 2, event.packet->dataLength - 2));
				this->clients[event.packet->data[1]] = client;

				this->addMessage(client->nick + " has joined the server!");
			}

			break;
		}

		case net::PACKET_LEAVE:{
			if (event.packet->dataLength == 2){
				this->addMessage(this->clients[event.packet->data[1]]->nick + " has left the server!");

				// Release selected and owned objects
				for (auto& object : this->objects){
					if (object.second->isSelectedBy(this->clients[event.packet->data[1]])){
						object.second->select(nullptr);
					}

					if (object.second->isOwnedBy(this->clients[event.packet->data[1]])){
						object.second->own(nullptr);
					}
				}

				// Clear the client information
				delete this->clients.find(event.packet->data[1])->second;
				this->clients.erase(event.packet->data[1]);
			}

			break;
		}

		case net::PACKET_CHAT:{
			if (event.packet->dataLength >= 1 + 1 + 1 && event.packet->dataLength <= 1 + 1 + 1 + 255){
				this->addMessage(this->clients[event.packet->data[1]]->nick + ": " + std::string((char*) event.packet->data + 2, event.packet->dataLength - 2));
			}

			break;
		}

		case net::PACKET_CREATE:{
			if (event.packet->dataLength >= 1 + 6 + 8 + 1 && event.packet->dataLength <= 1 + 6 + 8 + 255){
				net::Client *client;
				if (event.packet->data[1] != 255){
					client = this->clients.find(event.packet->data[1])->second;
				}

				unsigned short objId = net::bytesToShort(event.packet->data + 2);
				net::Client *selected = net::clientIdToClient(this->clients, event.packet->data[4]);
                net::Client *owner = net::clientIdToClient(this->clients, event.packet->data[5]);
				bool flipped = event.packet->data[6];
				Vector2 location = net::bytesToVector2(event.packet->data + 7);
				std::string objectId = std::string((char*) event.packet->data + 15, event.packet->dataLength - 15);

				Object *object = new Object(objectId, objId, location);
				object->select(selected);
				object->own(owner);
				object->setFlipped(flipped);

				this->objects.insert(std::pair<unsigned int, Object*>(objId, object));
				this->objectOrder.push_back(object);

				if (event.packet->data[1] != 255){
					this->addMessage(client->nick + " created a new " + object->getName());
				}

				this->checkObjectOrder();
			}

			break;
		}

		case net::PACKET_MOVE:{
			if (event.packet->dataLength >= 2 + 10){
				net::Client *client = this->clients.find(event.packet->data[1])->second;

				unsigned int numberObjects = 0;
				Object *lastObject;

				size_t i = 2;
				while (i < event.packet->dataLength){
					++numberObjects;

					unsigned short objId = net::bytesToShort(event.packet->data + i);
					Vector2 location = net::bytesToVector2(event.packet->data + i + 2);

					Object *object = this->objects.find(objId)->second;
					object->setLocation(location);

					net::removeObject(this->objectOrder, object);
					this->objectOrder.push_back(object);

					lastObject = object;
					i += 10;
				}

				if (numberObjects == 1){
					this->addMessage(client->nick + " moved " + lastObject->getName() + ".");
				}else if (numberObjects >= 2){
					std::ostringstream stream;
					stream << numberObjects;

					this->addMessage(client->nick + " moved " + stream.str() + " objects.");
				}

				this->checkObjectOrder();
			}

			break;
		}

		case net::PACKET_SELECT:{
			if (event.packet->dataLength >= 2){
				net::Client *client = this->clients.find(event.packet->data[1])->second;

				for (auto& object : this->objects){
					if (object.second->isSelectedBy(client)){
						object.second->select(nullptr);
					}
				}

				size_t i = 2;
				while (i < event.packet->dataLength){
					unsigned short objId = net::bytesToShort(event.packet->data + i);

					this->objects.find(objId)->second->select(client);

					i += 2;
				}
			}

			break;
		}

		case net::PACKET_FLIP:{
			if (event.packet->dataLength >= 2){
				net::Client *client = this->clients.find(event.packet->data[1])->second;

				unsigned int numberObjects = 0;
                Object *lastObject;

				size_t i = 2;
				while (i < event.packet->dataLength){
					++numberObjects;

					unsigned short objId = net::bytesToShort(event.packet->data + i);

					Object *object = this->objects.find(objId)->second;
					object->flip();

					lastObject = object;
					i += 2;
				}

				if (numberObjects == 1){
                    this->addMessage(client->nick + " flipped " + lastObject->getName() + ".");
                }else if (numberObjects >= 2){
                    std::ostringstream stream;
                    stream << numberObjects;

                    this->addMessage(client->nick + " flipped " + stream.str() + " objects.");
                }
			}


			break;
		}

		case net::PACKET_PINGS:{
			// TODO: Do we want this?
			/*if (event.packet->dataLength >= 4 && event.packet->dataLength <= 1 + 3 * net::MAX_CLIENTS){
				size_t i = 1;
				while (i < event.packet->dataLength){
					if (this->clients.count(event.packet->data[i]) > 0){
						this->clients[event.packet->data[i]]->ping = (event.packet->data[i + 1] + (event.packet->data[i + 2] << 8));
					}

					i += 3;
				}
			}*/

			break;
		}
	}
}

void Game::addMessage(std::string text){
	Message message;
	message.message = text;
	message.time = previousTime;

	this->messages.push_back(message);

	std::cout << message.message << std::endl;
}

// Send a chat packet
void Game::sendChat(std::string text){
	if (text.at(0) == '/'){
		this->chatCommand(text.substr(1));
	}else if (al_ustr_length(input->getTextUstr()) > 0){
		std::string data;
		data.push_back(net::PACKET_CHAT);
		data.append(this->input->getText());

		net::sendCommand(connection, data.c_str(), data.length());
	}

	delete input;
	this->input = nullptr;
}

void Game::chatCommand(std::string commandstr){
	std::istringstream command(commandstr);
	std::vector<std::string> parameters;
	std::copy(std::istream_iterator<std::string>(command), std::istream_iterator<std::string>(), std::back_inserter<std::vector<std::string>>(parameters));

	if (parameters.at(0) == "create"){
		if (parameters.size() == 2){
			this->createObject(parameters.at(1));
		}else if (parameters.size() == 4){
			Vector2 location;
			{
				std::istringstream stream(parameters.at(2));
				stream >> location.x;
			}
			{
				std::istringstream stream(parameters.at(3));
				stream >> location.y;
			}

			this->createObject(parameters.at(1), location);
		}else{
			this->addMessage("Usage: /" + parameters.at(0) + " object [x y]");
		}
	}else{
		this->addMessage(parameters.at(0) + ": command not found!");
	}
}

void Game::createObject(std::string objectId, Vector2 location){
	std::string data;
	data.push_back(net::PACKET_CREATE);
	data += 255; // Not selected
	data += 255; // Not owned
	bool flipped = false;
	data += flipped; // Not flipped
	net::dataAppendVector2(data, location);
	data.append(objectId);

	net::sendCommand(connection, data.c_str(), data.length());
}

void Game::checkObjectOrder(){
	for (auto& object : this->objectOrder){
		object->checkIfUnder(this->objectOrder);
	}
}

void Game::askNick(){
	this->input = new InputBox(this, &Game::identifyToServer, Vector2(SCREEN_H - 40, 0), Vector2(20, 100), this->font, 16);
}

void Game::removeInput(){
	delete input;
    this->input = nullptr;
}

void Game::identifyToServer(std::string nick){
	std::string data;
	data.push_back(net::PACKET_HANDSHAKE);
	data.append(nick, 0, 16); // Limit nick to 16 characters

	net::sendCommand(connection, data.c_str(), data.length());
}

void Game::render(){
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

void Game::renderGame(){
	this->renderer->useTransform(CAMERA);

	// Draw the table area
	this->renderer->drawRectangle(Vector2(-net::MAX_FLOAT, -net::MAX_FLOAT), Vector2(net::MAX_FLOAT, net::MAX_FLOAT), 1.0f, 1.0f, 1.0f, 1.0f, 5.0f);

	for (auto& object : this->objectOrder){
		object->draw(this->renderer, this->clients.find(this->localClient)->second);
	}
}

void Game::renderUI(){
	this->renderer->useTransform(UI);

	double deltaTime = al_get_time() - this->previousTime;
	this->previousTime = al_get_time();

	std::ostringstream tmpText;

	int i = 0;
	for (auto& client : this->clients){
		if (client.second->joined){
			tmpText.str(std::string());
			tmpText << client.second->nick << " (" << client.second->ping << " ms)";
			al_draw_text(font, al_map_rgb_f(1.0f, 1.0f, 1.0f), 0.0f, i * 20.0f, 0, tmpText.str().c_str());
		}

		++i;
	}

	for (std::vector<Message>::size_type i = this->messages.size(); i > 0; --i){
		if (al_get_time() < this->messages[i-1].time + 10.0){
			al_draw_text(font, al_map_rgb_f(1.0f, 1.0f, 1.0f), 0.0f, al_get_display_height(display) / 2.0f + i * 20.0f - this->messages.size() * 20.0f,
			             0, this->messages[i-1].message.c_str());
		}
	}

	tmpText.str(std::string());
	tmpText << "FPS: " << (int) (1.0 / deltaTime + 0.25);
	al_draw_text(font, al_map_rgb_f(1.0f, 1.0f, 1.0f), 0.0f, al_get_display_height(display) - 20.0f, 0, tmpText.str().c_str());

	for (auto& widget : this->widgets){
		widget->draw();
	}
	if (this->input != nullptr){
		input->draw();
	}
}

void Game::resize(){
	al_acknowledge_resize(this->display);

	this->renderer->setScreenSize(Vector2(al_get_display_width(this->display), al_get_display_height(this->display)));
	this->renderer->updateTransformations();
}
