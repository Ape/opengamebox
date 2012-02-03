#ifndef IRENDERER_H
#define IRENDERER_H

#include <string>

#include "coordinates.h"

class IRenderer{
public:
	virtual void drawBitmap(std::string texture, Coordinates source_location,
	                        Coordinates source_size, Coordinates dest_location,
	                        Coordinates dest_siz) = 0;

	virtual Coordinates getTextureSize(std::string texture) = 0;
};

#endif
