// Copyright 2012 Lauri Niskanen
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

#include "widget.h"

Widget::Widget(Vector2 location, Vector2 size) {
	this->location = location;
	this->size = size;
}

Widget::~Widget() {
	for (auto widget : this->widgets) {
		delete widget;
	}
}

void Widget::draw(IRenderer *renderer) {
	for (auto &widget : this->widgets) {
		widget->draw(renderer);
	}
}

bool Widget::onKey(ALLEGRO_KEYBOARD_EVENT keyboard) {
	return false;
}

void Widget::addWidget(Widget *widget){
	this->widgets.push_back(widget);
}

void Widget::resize(Vector2 multiplier) {
	this->location.x *= multiplier.x;
	this->location.y *= multiplier.y;
	this->size.x *= multiplier.x;
	this->size.y *= multiplier.y;
}

void Widget::resize(Vector2 location, Vector2 size) {
	this->location = location;
	this->size = size;
}

Vector2 Widget::getLocation(void) {
	return this->location;
}

Vector2 Widget::getSize(void) {
	return this->size;
}
