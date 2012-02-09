#ifndef IRENDERER_H
#define IRENDERER_H

#include <string>

#include "vector2.h"
#include "coordinates.h"
#include "color.h"

class Color;

enum Transformation{CAMERA, CAMERA_INVERSE, UI};

class IRenderer{
public:
	virtual void updateTransformations(void) = 0;

	virtual void mulScreenZoom(float zoom) = 0;
	virtual void addScreenLocation(Vector2 location) = 0;
	virtual void setScreenSize(Coordinates screenSize) = 0;

	virtual void drawBitmap(std::string texture, Vector2 source_location,
	                        Vector2 source_size, Vector2 dest_location,
	                        Vector2 dest_size) = 0;

	virtual void drawBitmapTinted(std::string texture, Vector2 source_location,
	                        Vector2 source_size, Vector2 dest_location,
	                        Vector2 dest_size, Color color) = 0;

	virtual void drawRectangle(Vector2 pointA, Vector2 pointB, Color color, float thickness) = 0;

	virtual void drawText(std::string text, Color color, Vector2 location) = 0;

	virtual Coordinates getTextureSize(std::string texture) = 0;

	virtual void transformLocation(Transformation transformation, Vector2 &location) = 0;
	virtual void useTransform(Transformation transformation) = 0;

	virtual void hsvToRgb(float hue, float saturation, float value, Color *color) = 0;
};

#endif
