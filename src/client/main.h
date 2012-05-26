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

#ifndef MAIN_H
#define MAIN_H

#include <vector>
#include <map>
#include <list>
#include <set>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <enet/enet.h>

#include "../net.h"
#include "../utils.h"
#include "../objectClassManager.h"
#include "../objectClass.h"
#include "../object.h"
#include "../vector2.h"
#include "renderer.h"
#include "widget.h"
#include "widgets/inputBox.h"
#include "../client.h"

// Default screen configuration
const int SCREEN_W = 1024;
const int SCREEN_H = 768;
const float FPS_LIMIT = 60.0f;
const bool FULLSCREEN = false;
const int MULTISAMPLING_SAMPLES = 2;
const float ANIMATION_TIME = 0.5f;
const float MESSAGE_TIME = 20.0f;

class Game {
public:
	Game(void);
	bool init(void);

	bool connect(std::string address, int port);
	bool connectMasterServer(std::string address, int port);

	int run(void);

	void sendChat(std::string text);
	std::string getSentMessage(size_t index);
	size_t getSentMessageCount(void);
	void identifyToServer(std::string nick);

private:
	ALLEGRO_EVENT_QUEUE *event_queue;
	ALLEGRO_TIMER *timer;

	ENetAddress hostAddress;
	ENetHost *connection;
	ENetPeer *host;

	Renderer *renderer;

	enum class State {INITIALIZING, RUNNING, EXITING, TERMINATED};
	State state;

	enum class ConnectionState {NOT_CONNECTED, CONNECTING, CONNECTED, DISCONNECTING, CONNECTING_MASTER_SERVER, CONNECTED_MASTER_SERVER};
	ConnectionState connectionState;

	bool nextFrame;
	double previousTime;
	double deltaTime;

	ObjectClassManager objectClassManager;

	std::map<unsigned short, Object*> objects;
	std::vector<Object*> objectOrder;
	std::map<unsigned char, Client*> clients;
	unsigned char localClient;

	struct Message {
		std::string message;
		double time;
	};

	std::vector<Message> messages;
	std::vector<Widget*> widgets;
	InputBox* input;
	std::vector<std::string> sentMessages;

	std::vector<std::string> dCreateBuffer;
	std::list<Object*> selectedObjects;
	bool dragging;
	Vector2 draggingStart;

	struct KeyStatus {
		bool screenZoomIn;
		bool screenZoomOut;
		bool screenMoveLeft;
		bool screenMoveRight;
		bool screenMoveUp;
		bool screenMoveDown;
		bool screenRotateClockwise;
		bool screenRotateCClockwise;
		bool snappingToGrid;
		bool moveScreen;
	};

	KeyStatus keyStatus;
	Vector2 moveScreenStart;

	void mainLoop(void);
	void quit(void);
	void dispose(void);

	void localEvents(void);

	void networkEvents(void);
	void receivePacket(ENetEvent event);

	void addMessage(std::string message);
	void chatCommand(std::string commandstr);
	void loadScript(std::string script);
	void saveScript(std::string name);
	std::string createObject(std::string object, Vector2 location = Vector2(0.0f, 0.0f));
	void checkObjectOrder(void);

	void askNick(void);
	void queryMasterServer(void);

	void update(void);
	void render(void);
	void renderGame(void);
	void renderUI(void);
};

#endif
