#include "inputBox.h"
#include "../main.h"

InputBox::InputBox(Game *game, void (Game::*send)(std::string), Vector2 location, Vector2 size, ALLEGRO_FONT *font, unsigned char maxLen) : Widget(location, size) {
	this->game = game;
	this->send = send;

	this->font = font;
	this->text = al_ustr_new("");
	this->inputLocation = 0;
	this->maxLength = maxLen;
}
InputBox::~InputBox() {
	al_ustr_free(this->text);
}

void InputBox::draw() {
	al_draw_rectangle(this->location.y, this->location.x, this->location.y + this->size.y, this->location.x + this->size.x,
	                  al_map_rgb_f(1.0f, 1.0f, 1.0f), 1.0f);

	std::ostringstream tmpText;

	tmpText.str(std::string());

	ALLEGRO_USTR *tmpUstr = al_ustr_dup_substr(this->text, 0, al_ustr_offset(this->text, this->inputLocation));
	tmpText << "> " << al_cstr(tmpUstr) << "|";

	tmpUstr = al_ustr_dup_substr(this->text, al_ustr_offset(this->text, this->inputLocation), al_ustr_size(this->text));
	tmpText << al_cstr(tmpUstr);

	al_ustr_free(tmpUstr);
	al_draw_text(font, al_map_rgb_f(1.0f, 1.0f, 1.0f), 0.0f, this->location.x - 2.0f, this->location.y, tmpText.str().c_str());
}

bool InputBox::onKey(ALLEGRO_KEYBOARD_EVENT keyboard) {
	if (keyboard.keycode == ALLEGRO_KEY_ENTER) {
		if (al_ustr_length(this->text) > 0) {
			(this->game->*send)(std::string(al_cstr(this->text)));
		}
		this->game->removeInput();
	} else if (keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
		this->game->removeInput();
	} else if (keyboard.keycode == ALLEGRO_KEY_DELETE) {
		if (this->inputLocation != al_ustr_length(this->text)) {
			al_ustr_remove_chr(this->text, al_ustr_offset(this->text, this->inputLocation));
		}
	} else if (keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
		if (this->inputLocation > 0) {
			al_ustr_remove_chr(this->text, al_ustr_offset(this->text, this->inputLocation - 1));
			--this->inputLocation;
		}
	} else if (keyboard.keycode == ALLEGRO_KEY_LEFT) {
		if (this->inputLocation > 0) {
			--this->inputLocation;
		}
	} else if (keyboard.keycode == ALLEGRO_KEY_RIGHT) {
		if (this->inputLocation < al_ustr_length(this->text)) {
			++this->inputLocation;
		}
	} else if (keyboard.unichar > 0 && this->inputLocation < this->maxLength) {
		al_ustr_insert_chr(this->text, al_ustr_offset(this->text, this->inputLocation), keyboard.unichar);
		++this->inputLocation;
	}

	return true;
}

std::string InputBox::getText() {
	return std::string(al_cstr(this->text));
}

ALLEGRO_USTR* InputBox::getTextUstr() {
	return this->text;
}
