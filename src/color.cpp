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

#include "color.h"

std::ostream& operator<<(std::ostream &output, const Color &color) {
	output << "(" << color.red << ", " << color.green << ", " << color.blue << ", " << color.alpha << ")";
	return output;
}

Color::Color(const float red, const float green, const float blue, const float alpha) {
	this->red   = red;
	this->green = green;
	this->blue  = blue;
	this->alpha = alpha;
}

Color::Color(IRenderer *renderer, unsigned int id) {
	this->setFromId(renderer, id);
}

bool Color::operator==(const Color &color) const {
	return color.red == this->red && color.green == this->green && color.blue == this->blue && color.alpha == this->alpha;
}

bool Color::operator!=(const Color &color) const {
	return !(this->cpy() == color);
}

void Color::setHsv(IRenderer *renderer, float hue, float saturation, float value) {
	renderer->hsvToRgb(hue, saturation, value, this);
	this->alpha = 1.0f;
}

void Color::setFromId(IRenderer *renderer, unsigned int id) {
	// The magical float values below are based on the golden angle on the hue circle
	this->setHsv(renderer, 217.75608f + 137.50776f * id, 1.0f, 1 - ((id - id % 7) / 7) % 2 * 0.4f);
}

std::string Color::encodedString() const {
	std::ostringstream stream;

	stream << "^";
	stream << std::hex;
	stream << static_cast<int>(this->red * 15.0f + 0.5f);
	stream << static_cast<int>(this->green * 15.0f + 0.5f);
	stream << static_cast<int>(this->blue * 15.0f + 0.5f);

	return stream.str();
}
