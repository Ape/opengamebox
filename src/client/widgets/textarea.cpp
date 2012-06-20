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

#include "textarea.h"

TextArea::TextArea(Vector2 location, Vector2 size, std::string text, ALLEGRO_FONT *font)
		  : Widget(location, size),
		    font(font) {
	this->text = utils::splitString(text, ' ');
}

TextArea::~TextArea() {

}

void TextArea::draw(IRenderer *renderer) {
	Vector2 drawPos = this->location;
	for (auto &word : this->text) {
		int width = al_get_text_width(this->font, word.c_str());
		if (width + drawPos.x > this->size.x) {
			drawPos.y += 15;
		}
		drawPos.x = this->location.x;
		renderer->drawText(word, drawPos);
		drawPos.x += al_get_text_width(this->font, " ");
		drawPos.x += width;
		renderer->drawText(word, drawPos);
		drawPos.x += al_get_text_width(this->font, " ");
		drawPos.x += width;
	}
}
