#ifndef RENDERER_H
#define RENDERER_H

#include <map>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "../irenderer.h"

class Renderer : public IRenderer{
private:
	std::map<std::string, ALLEGRO_BITMAP*> textures;

	void loadTexture(std::string);

public:
	Renderer();
	~Renderer();

	virtual void drawBitmap(std::string texture, Coordinates source_location,
	                        Coordinates source_size, Coordinates dest_location,
	                        Coordinates dest_size);
	virtual Coordinates getTextureSize(std::string texture);
};

#endif
