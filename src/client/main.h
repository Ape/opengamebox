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
#include <csignal>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_physfs.h>
#include <enet/enet.h>
#include <physfs.h>

#include "../net.h"
#include "../packet.h"
#include "../utils.h"
#include "../objectClassManager.h"
#include "../objectClass.h"
#include "../object.h"
#include "../vector2.h"
#include "renderer.h"
#include "widget.h"
#include "widgets/inputBox.h"
#include "widgets/textarea.h"
#include "widgets/chatwidget.h"
#include "../client.h"
#include "../settings.h"

class ProgressBar;

// Workaround an issue with Windows specific preprocessor definitions
#undef ERROR

const std::string HISTORY_FILE = "history.txt";
const size_t HISTORY_SIZE = 20;

int main(int argc, char **argv);

class Game {
public:
	Game(void);
	~Game(void);
	bool init(void);

	static Game *gamePtr;
	static void catchSignal(int signal);

	bool connect(std::string address, int port);
	bool connectMasterServer(void);
	bool connectMasterServer(std::string address, int port);

	int run(void);
	void quit(void);

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

	Settings *settings;

	enum class State {INITIALIZING, RUNNING, EXITING, TERMINATED};
	State state;

	enum class ConnectionState {NOT_CONNECTED, CONNECTING, CONNECTED, DISCONNECTING, CONNECTING_MASTER_SERVER, CONNECTED_MASTER_SERVER};
	ConnectionState connectionState;

	bool nextFrame;
	double previousTime;
	double deltaTime;

	ObjectClassManager objectClassManager;

	bool loadingPackage;
	std::set<std::string> missingPackages;
	struct File {
		std::string name;
		int size;
		char* data;
		int received;
		double startTime;
	};
	File loadingfile;

	std::map<unsigned short, Object*> objects;
	std::vector<Object*> objectOrder;
	std::map<unsigned char, Client*> clients;
	unsigned char localClient;

	ChatWidget *chatWidget;

	std::vector<Widget*> widgets;
	InputBox *input;
	std::vector<std::string> sentMessages;

	ProgressBar *fileTransferProgress;

	std::vector<std::string> dCreateBuffer;
	std::list<Object*> selectedObjects;
	bool dragging;
	bool selecting;
	Vector2 selectingStart;
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

	void disconnect(void);
	void disconnectMasterServer(void);
	void dispose(void);
	void disposeGame(void);

	void loadHistory(void);
	void saveHistory(void);

	void localEvents(void);
	void endDragging(void);

	void networkEvents(void);
	void receivePacket(ENetEvent event);

	enum class MessageType {NORMAL, ERROR, WARNING, DEBUG};
	void addMessage(std::string message, MessageType type = MessageType::NORMAL);
	void chatCommand(std::string commandstr);

	void login(std::string password);
	void kick(std::string nick);
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
