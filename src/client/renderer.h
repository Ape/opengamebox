#ifndef RENDERER_H
#define RENDERER_H

#include <map>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "../irenderer.h"

class Renderer : public IRenderer{
public:
	Renderer();
	~Renderer();

	virtual void resize(Vector2 displaySize, float screenZoom);

	virtual void drawBitmap(std::string texture, Vector2 source_location,
	                        Vector2 source_size, Vector2 dest_location,
	                        Vector2 dest_size);
	virtual void drawBitmapTinted(std::string texture, Vector2 source_location,
                            Vector2 source_size, Vector2 dest_location,
                            Vector2 dest_size, float r, float g, float b, float alpha);
	virtual void drawRectangle(Vector2 pointA, Vector2 pointB, float r, float g, float b, float alpha, float thickness);

	virtual Coordinates getTextureSize(std::string texture);

	virtual void transformLocation(Transformation transformation, Vector2 &location);
	virtual void useTransform(Transformation transformation);

private:
    ALLEGRO_TRANSFORM camera;
    ALLEGRO_TRANSFORM camera_inverse;
    ALLEGRO_TRANSFORM cameraUI;

	std::map<std::string, ALLEGRO_BITMAP*> textures;

	void loadTexture(std::string);

	ALLEGRO_TRANSFORM* getTransformation(Transformation transformation);
};

#endif
