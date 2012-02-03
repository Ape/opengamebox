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

	virtual void drawBitmap(std::string texture, Vector2 source_location,
	                        Vector2 source_size, Vector2 dest_location,
	                        Vector2 dest_size);
	virtual Coordinates getTextureSize(std::string texture);
};

#endif
