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
	virtual void updateTransformations(void) = 0;

	virtual void mulScreenZoom(float zoom) = 0;
	virtual void addScreenLocation(Vector2 location) = 0;
	virtual void setScreenSize(Coordinates screenSize) = 0;
	virtual void rotateScreen(float angle) = 0;

	virtual void drawBitmap(std::string texture, Vector2 source_location,
	                        Vector2 source_size, Vector2 dest_location,
	                        Vector2 dest_size) = 0;

	virtual void drawBitmapTinted(std::string texture, Vector2 source_location,
	                        Vector2 source_size, Vector2 dest_location,
	                        Vector2 dest_size, Color color) = 0;

	virtual void drawRectangle(Vector2 pointA, Vector2 pointB, Color color, float thickness, Transformation transformation = Transformation::UI) = 0;
	virtual void drawRectangleFilled(Vector2 pointA, Vector2 pointB, Color color, Transformation transformation = Transformation::UI) = 0;

	virtual void drawText(std::string text, Vector2 location, Color color, Alignment alignment = Alignment::LEFT) = 0;

	virtual Coordinates getTextureSize(std::string texture) = 0;

	virtual void transformLocation(Transformation transformation, Vector2 &location) = 0;
	virtual void useTransformation(Transformation transformation) = 0;

	virtual void hsvToRgb(float hue, float saturation, float value, Color *color) = 0;
};

#endif
