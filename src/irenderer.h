#ifndef IRENDERER_H
#define IRENDERER_H

#include <string>

#include "vector2.h"
#include "coordinates.h"

class IRenderer{
public:
	virtual void drawBitmap(std::string texture, Vector2 source_location,
	                        Vector2 source_size, Vector2 dest_location,
	                        Vector2 dest_size) = 0;

	virtual Coordinates getTextureSize(std::string texture) = 0;
};

#endif
