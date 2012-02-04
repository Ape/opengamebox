#ifndef IRENDERER_H
#define IRENDERER_H

#include <string>

#include "vector2.h"
#include "coordinates.h"

enum Transformation{CAMERA, CAMERA_INVERSE, UI};

class IRenderer{
public:
	virtual void resize(Vector2 displaySize, float screenZoom) = 0;

	virtual void drawBitmap(std::string texture, Vector2 source_location,
	                        Vector2 source_size, Vector2 dest_location,
	                        Vector2 dest_size) = 0;

	virtual void drawBitmapTinted(std::string texture, Vector2 source_location,
	                        Vector2 source_size, Vector2 dest_location,
	                        Vector2 dest_size, float r, float g, float b, float alpha) = 0;

	virtual void drawRectangle(Vector2 pointA, Vector2 pointB, float r, float g, float b, float alpha, float thickness) = 0;

	virtual Coordinates getTextureSize(std::string texture) = 0;

	virtual void transformLocation(Transformation transformation, Vector2 &location) = 0;
	virtual void useTransform(Transformation transformation) = 0;
};

#endif
