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
