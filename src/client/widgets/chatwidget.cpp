// Copyright 2012 Antti Aalto
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

#include "chatwidget.h"

ChatWidget::ChatWidget(Vector2 location, Vector2 size, float messagetime, ALLEGRO_FONT *font)
		  : Widget(location, size),
		    font(font),
			messageTime(messagetime) { }

void ChatWidget::addMessage(Message message) {
	std::pair<TextArea*, Message> pair(new TextArea(Vector2(this->location.x, this->location.y + this->size.y - 20), this->size,
										message.message, this->font), message);
	int messageSize = pair.first->getLineCount();
	pair.first->move(this->location + Vector2(0.0f, -20.0f * messageSize + this->size.y));
	for (auto &message : this->messages) {
		message.first->move(message.first->getLocation() + Vector2(0.0f, -20.0f * messageSize));
	}
	this->messages.push_back(pair);
}

void ChatWidget::draw(IRenderer *renderer, bool drawAll) {
float time = al_get_time();
	for (auto &pair : this->messages) {
		if (time < pair.second.time + this->messageTime || drawAll) {
			pair.first->draw(renderer);
		}
	}
}

ChatWidget::~ChatWidget() {

}
