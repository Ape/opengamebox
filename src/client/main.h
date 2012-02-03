#ifndef MAIN_H
#define MAIN_H

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iostream>

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

// Default screen configuration
const int SCREEN_W = 640;
const int SCREEN_H = 480;
const float FPS_LIMIT = 60.0f;
const bool FULLSCREEN = false;

class Game{
public:
	Game(void);

	int run(std::string address, int port);

	void removeInput(void);
	void sendChat(std::string text);
	void identifyToServer(std::string nick);

private:
	ALLEGRO_FONT *font;
	ALLEGRO_DISPLAY *display;
	ALLEGRO_EVENT_QUEUE *event_queue;
	ALLEGRO_TIMER *timer;
	ALLEGRO_TRANSFORM camera;
	ALLEGRO_TRANSFORM camera_inverse;
	ALLEGRO_TRANSFORM UIcamera;

	ENetAddress hostAddress;
	ENetHost *connection;
	ENetPeer *host;

	Renderer *renderer;

	bool exiting;
	bool redraw;
	bool disconnecting;

	float screenZoom;
	double previousTime;

	std::map<unsigned char, Object*> objects;
	std::map<unsigned char, net::Client*> clients;
	unsigned char localClient;

	struct Message{
		std::string message;
		double time;
	};

	std::vector<Message> messages;
	std::vector<Widget*> widgets;
	InputBox* input;

	unsigned char currentAction;

	void mainLoop(void);
	void quit(void);
	void dispose(void);

	void localEvents(void);

	void networkEvents(void);
	void receivePacket(ENetEvent event);

	void sendAction(unsigned char action, bool enable);

	void addMessage(std::string message);
	void chatCommand(std::string command);
	void askNick(void);

	void render(void);
	void renderGame(void);
	void renderUI(void);
	void resize(void);
};

#endif
