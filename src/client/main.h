#ifndef MAIN_H
#define MAIN_H

#include <vector>
#include <map>
#include <list>
#include <set>
#include <string>
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <enet/enet.h>

#include "../net.h"
#include "../object.h"
#include "renderer.h"
#include "../vector2.h"
#include "widget.h"
#include "widgets/inputBox.h"

namespace net{
	struct Client;
}

// Default screen configuration
const int SCREEN_W = 1024;
const int SCREEN_H = 768;
const float FPS_LIMIT = 60.0f;
const bool FULLSCREEN = false;
const float ANIMATION_TIME = 0.5f;
const int MULTISAMPLING_SAMPLES = 2;

class Game{
public:
	Game(void);

	int run(std::string address, int port);

	void sendChat(std::string text);
	void identifyToServer(std::string nick);

private:
	ALLEGRO_EVENT_QUEUE *event_queue;
	ALLEGRO_TIMER *timer;

	ENetAddress hostAddress;
	ENetHost *connection;
	ENetPeer *host;

	Renderer *renderer;

	bool exiting;
	bool redraw;
	bool disconnecting;

	double previousTime;
	double deltaTime;

	std::map<unsigned short, Object*> objects;
	std::vector<Object*> objectOrder;
	std::map<unsigned char, net::Client*> clients;
	unsigned char localClient;

	struct Message{
		std::string message;
		double time;
	};

	std::vector<Message> messages;
	std::vector<Widget*> widgets;
	InputBox* input;

	std::list<Object*> selectedObjects;
	bool dragging;
	Vector2 draggingStart;

	bool movingScreen;
	Vector2 movingScreenStart;

	bool snappingToGrid;

	void mainLoop(void);
	void quit(void);
	void dispose(void);

	void localEvents(void);

	void networkEvents(void);
	void receivePacket(ENetEvent event);

	void sendAction(unsigned char action, bool enable);

	void addMessage(std::string message);
	void chatCommand(std::string commandstr);
	void createObject(std::string object, Vector2 location = Vector2(0.0f, 0.0f));
	void checkObjectOrder(void);

	void askNick(void);

	void render(void);
	void animate(void);
	void renderGame(void);
	void renderUI(void);
};

#endif
