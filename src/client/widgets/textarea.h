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

#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "../widget.h"
#include "../../utils.h"

class TextArea : public Widget{
public:
	TextArea(Vector2 location, Vector2 size, std::string text, ALLEGRO_FONT *font);

	virtual ~TextArea();

private:
	std::vector<std::string> text;
	ALLEGRO_FONT *font;

	virtual void draw(IRenderer *renderer);

};


#endif
