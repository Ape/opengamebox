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

#include <list>
#include <set>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <csignal>
#include <tuple>
#include <thread>
#include <mutex>
#include <atomic>

#include <enet/enet.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_physfs.h>
#include <physfs.h>

#include "../net.h"
#include "../packet.h"
#include "../utils.h"
#include "../object.h"
#include "../objectClassManager.h"
#include "../objectClass.h"
#include "renderer.h"
#include "widgets/inputBox.h"
#include "widgets/textarea.h"
#include "widgets/chatwidget.h"
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

	void update(void);
	void render(void);

	std::thread *renderThread;

private:
	// Variables only for event thread
	ALLEGRO_EVENT_QUEUE *event_queue;
	ALLEGRO_TIMER *timer;

	ENetAddress hostAddress;
	ENetHost *connection;
	ENetPeer *host;

	Renderer *renderer;

	Settings *settings;

	enum class ConnectionState {NOT_CONNECTED, CONNECTING, CONNECTED, DISCONNECTING, CONNECTING_MASTER_SERVER, CONNECTED_MASTER_SERVER};
	ConnectionState connectionState;

	ObjectClassManager objectClassManager;

	std::vector<Object*> objectOrder;

	unsigned char localClient;

	std::vector<std::string> sentMessages;

	std::vector<std::tuple<std::string, Vector2, bool>> dCreateBuffer;
	std::list<Object*> selectedObjects;

	bool dragging;
	Vector2 draggingStart;

	Vector2 moveScreenStart;

	// Variables for both threads
	// TODO: better mutex handling
	std::mutex dataMutex;
	std::mutex displayMutex;
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

	std::mutex objectsMutex;
	std::map<unsigned short, Object*> objects;
	std::vector<Object*> uninitializedObjects;

	Vector2 selectingStart;

	std::mutex widgetsMutex;
	ChatWidget *chatWidget;
	ProgressBar *fileTransferProgress;
	std::vector<Widget*> widgets;
	std::atomic<InputBox*> input;

	std::mutex clientsMutex;
	std::map<unsigned char, Client*> clients;

	// Atomic variables for both threads

	struct KeyStatus {
		std::atomic<bool> screenZoomIn;
		std::atomic<bool> screenZoomOut;
		std::atomic<bool> screenMoveLeft;
		std::atomic<bool> screenMoveRight;
		std::atomic<bool> screenMoveUp;
		std::atomic<bool> screenMoveDown;
		std::atomic<bool> screenRotateClockwise;
		std::atomic<bool> screenRotateCClockwise;
		std::atomic<bool> snappingToGrid;
		std::atomic<bool> moveScreen;
	};

	KeyStatus keyStatus;

	std::atomic<bool> resize;
	std::atomic<bool> nextFrame;
	std::atomic<double> previousTime;
	std::atomic<double> deltaTime;

	std::atomic<bool> selecting;

	enum class State {INITIALIZING, RUNNING, EXITING, TERMINATED};
	std::atomic<State> state;

	// Functions
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
	std::string createObject(std::string object, Vector2 location = Vector2(0.0f, 0.0f), bool flipped = false);
	void checkObjectOrder(void);

	void askNick(void);
	void queryMasterServer(void);

	void* renderThreadFunc();

	void renderGame(void);
	void renderUI(void);
};

#endif
