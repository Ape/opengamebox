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

#ifndef COLOR_H
#define COLOR_H

#include <iostream>
#include <sstream>

#include "irenderer.h"

class IRenderer;

class Color {
	friend std::ostream& operator<<(std::ostream &output, const Color &color);

public:
	float red;
	float green;
	float blue;
	float alpha;

	Color(const float red = 1.0f, const float green = 1.0f, const float blue = 1.0f, const float alpha = 1.0f);
	Color(IRenderer *renderer, unsigned int id);

	Color cpy(void) const;
	bool operator==(const Color &color) const;
	bool operator!=(const Color &color) const;

	void setHsv(IRenderer *renderer, float hue, float saturation, float value);
	void setFromId(IRenderer *renderer, unsigned int id);

	std::string encodedString() const;
};

inline Color Color::cpy() const {
	return Color(this->red, this->green, this->blue, this->alpha);
}

#endif
