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
	this->countLines();
}

void TextArea::countLines()
{
	int lines = 1;
	int width = 0;
	for (auto &word : this->text) {
		int textwidth = al_get_text_width(this->font, word.c_str());
		if (width + textwidth > this->size.x) {
			lines++;
			width = 0;
		} else {
			width += al_get_text_width(this->font, " ");
		}
		width += textwidth;
	}
	this->lineCount = lines;
}

TextArea::~TextArea() {

}

void TextArea::draw(IRenderer *renderer) {
	Vector2 drawPos(0.0f, 0.0f);
	std::string tempText;
	for (auto &word : this->text) {
		int width = al_get_text_width(this->font, word.c_str());
		if (width + drawPos.x > this->size.x) {

			renderer->drawText(tempText, this->location + Vector2(0.0f, drawPos.y));
			size_t colorPos = tempText.rfind('^');
			if (colorPos != std::string::npos) {
				tempText = tempText.substr(colorPos, 4);
			} else {
				tempText = "";
			}
			tempText += word;
			drawPos.y += 20;
			drawPos.x = 0;
		} else {
			drawPos.x += al_get_text_width(this->font, " ");
			tempText += word + " ";
		}
		drawPos.x += width;
	}
	renderer->drawText(tempText, this->location + Vector2(0.0f, drawPos.y));
}

int TextArea::getLineCount() {
	return this->lineCount;
}

void TextArea::setText(std::string text) {
	this->text = utils::splitString(text, ' ');
	this->countLines();
}

void TextArea::move(Vector2 location) {
	this->location = location;
}

Vector2 TextArea::getLocation() {
	return this->location;
}
