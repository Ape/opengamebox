#include "client.h"

Client::Client(ENetPeer *peer, unsigned char id) {
	this->peer = peer;
	this->joined = false;
	this->id = id;
}

Client::Client(std::string nick, Color color, unsigned char id) {
	this->joined = true;
	this->nick = nick;
	this->color = color;
	this->peer = nullptr;
	this->id = id;
}

unsigned char Client::getId(){
	return this->id;
}

std::string Client::getNick() {
	return this->nick;
}

Color Client::getColor() {
	return this->color;
}

std::string Client::getColorCode() {
	std::ostringstream colorCode;
	colorCode << "^#" << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(this->id);

	return colorCode.str();
}

std::string Client::getColoredNick() {
	return this->getColorCode() + this->getNick() + "^fff";
}

bool Client::isJoined() {
	return this->joined;
}

void Client::setJoined() {
	this->joined = true;
}

Client::~Client() {
	if(this->peer != nullptr) {
		delete this->peer;
	}
}

void Client::setNick(std::string nick) {
	this->nick = nick;
}

ENetPeer* Client::getPeer() {
	return this->peer;
}
