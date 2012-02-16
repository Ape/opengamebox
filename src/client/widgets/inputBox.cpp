#include "inputBox.h"
#include "../main.h"

InputBox::InputBox(Game *game, void (Game::*send)(std::string), std::string caption, Vector2 location, float width, ALLEGRO_FONT *font, unsigned char maxLen)
                  : Widget(location, Vector2(width, 20.0f)) {
	this->game = game;
	this->send = send;
	this->caption = caption;
	this->font = font;
	this->text = al_ustr_new("");
	this->inputLocation = 0;
	this->maxLength = maxLen;
}
InputBox::~InputBox() {
	al_ustr_free(this->text);
}

void InputBox::draw(IRenderer *renderer) {
	renderer->drawRectangleFilled(this->location, this->location + this->size, Color(0.15f, 0.15f, 0.15f));
	renderer->drawRectangle(this->location, this->location + this->size, Color(1.0f, 1.0f, 1.0f), 1.0f);

	std::ostringstream tmpText;

	tmpText.str(std::string());

	ALLEGRO_USTR *tmpUstr = al_ustr_dup_substr(this->text, 0, al_ustr_offset(this->text, this->inputLocation));

	if (! this->caption.empty()) {
		tmpText << this->caption << ": ";
	}

	tmpText << al_cstr(tmpUstr) << "|";
	al_ustr_free(tmpUstr);

	tmpUstr = al_ustr_dup_substr(this->text, al_ustr_offset(this->text, this->inputLocation), al_ustr_size(this->text));
	tmpText << al_cstr(tmpUstr);
	al_ustr_free(tmpUstr);

	renderer->drawText(tmpText.str(), Vector2(this->location), Color(1.0f, 1.0f, 1.0f));
}

bool InputBox::onKey(ALLEGRO_KEYBOARD_EVENT keyboard) {
	if (keyboard.keycode == ALLEGRO_KEY_ENTER) {
		(this->game->*send)(std::string(al_cstr(this->text)));
	} else if (keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
		(this->game->*send)(std::string(""));
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
