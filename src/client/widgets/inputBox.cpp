// Copyright 2012 Lauri Niskanen
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

#include "inputBox.h"

InputBox::InputBox(Game *game, void (Game::*send)(std::string), std::string caption, Vector2 location, float width, unsigned char maxLen, ALLEGRO_FONT *font)
: Widget(location, Vector2(width, 20.0f)),
	game(game),
	send(send),
	textWidget(new TextArea(location, size, caption + ": |", font)),
	caption(caption),
	text(al_ustr_new("")),
	inputLocation(0),
	maxLength(maxLen),
	historyIndex(0) {}

InputBox::~InputBox() {
	al_ustr_free(this->text);
	delete this->textWidget;
}

void InputBox::draw(IRenderer *renderer) {

	renderer->drawRectangleFilled(this->location, this->location + Vector2(this->size.x, this->textWidget->getLineCount() * 20), Color(0.15f, 0.15f, 0.15f));
	renderer->drawRectangle(this->location, this->location + Vector2(this->size.x, this->textWidget->getLineCount() * 20), Color(1.0f, 1.0f, 1.0f), 1.0f);

	textWidget->draw(renderer);

/*	std::ostringstream tmpText;

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

	renderer->drawText(tmpText.str(), Vector2(this->location));

*/
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
	} else if (keyboard.keycode == ALLEGRO_KEY_UP) {
		if (this->historyIndex < this->game->getSentMessageCount()) {
			++this->historyIndex;

			al_ustr_free(this->text);
			this->text = al_ustr_new(this->game->getSentMessage(this->historyIndex).c_str());
			this->inputLocation = al_ustr_length(this->text);
		}
	} else if (keyboard.keycode == ALLEGRO_KEY_DOWN) {
		if (this->historyIndex > 1) {
			--this->historyIndex;

			al_ustr_free(this->text);
			this->text = al_ustr_new(this->game->getSentMessage(this->historyIndex).c_str());
			this->inputLocation = al_ustr_length(this->text);
		} else if (this->historyIndex == 1) {
			--this->historyIndex;

			al_ustr_free(this->text);
			this->text = al_ustr_new("");
			this->inputLocation = 0;
		}
	} else if (keyboard.unichar > 0 && this->inputLocation < this->maxLength) {
		al_ustr_insert_chr(this->text, al_ustr_offset(this->text, this->inputLocation), keyboard.unichar);
		++this->inputLocation;
	}

	if (keyboard.keycode != ALLEGRO_KEY_ENTER && keyboard.keycode != ALLEGRO_KEY_DELETE) {

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

		textWidget->setText(tmpText.str());
	}
	return true;
}

void InputBox::resize(Vector2 multipler) {
	Widget::resize(multipler);
	this->size.y = 20;
}

std::string InputBox::getText() {
	return std::string(al_cstr(this->text));
}

ALLEGRO_USTR* InputBox::getTextUstr() {
	return this->text;
}
