#ifndef COLOR_H
#define COLOR_H

#include <iostream>

#include "irenderer.h"

class IRenderer;

class Color {
	friend std::ostream& operator<<(std::ostream &output, const Color &color);

public:
	float red;
	float green;
	float blue;
	float alpha;

	Color(const float red = 0.0f, const float green = 0.0f, const float blue = 0.0f, const float alpha = 0.0f);
	Color(IRenderer *renderer, unsigned int id);

	Color cpy(void) const;
	bool operator==(const Color &color) const;
	bool operator!=(const Color &color) const;

	void setHsv(IRenderer *renderer, float hue, float saturation, float value);
	void setFromId(IRenderer *renderer, unsigned int id);
};

inline Color Color::cpy() const {
	return Color(this->red, this->green, this->blue, this->alpha);
}

#endif
