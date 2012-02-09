#include "widget.h"

Widget::Widget(Vector2 location, Vector2 size) {
	this->location = location;
	this->size = size;
}

void Widget::draw() {

}

bool Widget::onKey(ALLEGRO_KEYBOARD_EVENT keyboard) {
	return false;
}
