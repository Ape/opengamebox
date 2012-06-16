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

#ifndef IRENDERER_H
#define IRENDERER_H

#include <string>

#include "vector2.h"
#include "coordinates.h"
#include "color.h"

class Color;

class IRenderer {
public:
	enum Transformation {CAMERA, CAMERA_INVERSE, UI};
	enum Alignment {LEFT, CENTER, RIGHT};

	virtual Coordinates getDisplaySize(void) const = 0;
	virtual void resize(void) = 0;
	virtual void setWindowTitle(std::string title, std::string icon) = 0;
	virtual void updateTransformations(void) = 0;

	virtual void zoomScreen(float zoom) = 0;
	virtual void scrollScreen(Vector2 translation) = 0;
	virtual void rotateScreen(float angle) = 0;
	virtual void setScreenSize(Coordinates screenSize) = 0;

	virtual void drawBitmap(std::string texture, Vector2 dest_location, Vector2 dest_size, float angle = 0) = 0;

	virtual void drawBitmapTinted(std::string texture, Vector2 dest_location, Vector2 dest_size, Color color, float angle = 0) = 0;

	virtual void drawLine(Vector2 pointA, Vector2 pointB, Color color, float thickness, Transformation transformation = Transformation::UI) = 0;

	virtual void drawRectangle(Vector2 pointA, Vector2 pointB, Color color, float thickness, Transformation transformation = Transformation::UI) = 0;
	virtual void drawRectangleFilled(Vector2 pointA, Vector2 pointB, Color color, Transformation transformation = Transformation::UI) = 0;

	virtual void drawCircle(Vector2 location, float radius, Color color, float thickness, Transformation transformation = Transformation::UI) = 0;
	virtual void drawCircleFilled(Vector2 location, float radius, Color color, Transformation transformation = Transformation::UI) = 0;

	virtual void drawText(std::string text, Vector2 location, Alignment alignment = Alignment::LEFT) = 0;

	virtual Coordinates getTextureSize(std::string texture) = 0;

	virtual void transformLocation(Transformation transformation, Vector2 &location) = 0;
	virtual void useTransformation(Transformation transformation) = 0;

	virtual void hsvToRgb(float hue, float saturation, float value, Color *color) = 0;
};

#endif
