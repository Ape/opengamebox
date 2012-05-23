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
	
}

void Widget::draw(IRenderer *renderer) {

}

bool Widget::onKey(ALLEGRO_KEYBOARD_EVENT keyboard) {
	return false;
}
