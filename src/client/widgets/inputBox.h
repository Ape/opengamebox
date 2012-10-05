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

#ifndef INPUTBOX_H
#define INPUTBOX_H

#include <enet/enet.h>
#include <string>
#include <sstream>

#include "textarea.h"

class Game;

class InputBox : public Widget {
public:
	InputBox(Game *game, void (Game::*send)(std::string), std::string caption, Vector2 location, float width, unsigned char maxLen, ALLEGRO_FONT *font);
	virtual ~InputBox();

	virtual void draw(IRenderer *renderer);
	virtual bool onKey(ALLEGRO_KEYBOARD_EVENT keyboard);
	std::string getText();
	virtual void resize(Vector2 multipler);

	ALLEGRO_USTR* getTextUstr();

private:
	Game *game;
	void (Game::*send)(std::string);

	TextArea *textWidget;
	std::string caption;
	ALLEGRO_USTR *text;
	size_t inputLocation;
	unsigned char maxLength;
	size_t historyIndex;
};

#include "../main.h"


#endif
