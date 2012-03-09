#include "client.h"

Client::Client(ENetPeer *peer, unsigned char id) {
	this->peer = peer;
	this->joined = false;
	this->id = id;
}

Client::Client(std::string nick, Color color) {
	this->joined = true;
	this->nick = nick;
	this->color = color;
	this->peer = nullptr;
}

Color Client::getColor() {
	return this->color;
}

std::string Client::getNick() {
	return this->nick;
}

std::string Client::getColoredNick() {
	return this->getColor().encodedString() + this->getNick() + "^fff";
}

Client::~Client() {
	if(this->peer != nullptr) {
		delete this->peer;
	}
}

void Client::setNick(std::string nick) {
	this->nick = nick;
}
