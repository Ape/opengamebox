#ifndef RENDERER_H
#define RENDERER_H

#include <map>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>

#include "../irenderer.h"
#include "../vector2.h"
#include "../color.h"

class Renderer : public IRenderer{
public:
	static const float PI;

	Renderer(Coordinates screenSize, const int multisamplingSamples);
	~Renderer(void);

	ALLEGRO_DISPLAY* getDisplay(void) const;
	ALLEGRO_FONT* getFont(void) const;

	virtual void resize(void);
	virtual Coordinates getDisplaySize(void) const;
	virtual void updateTransformations(void);

	virtual void zoomScreen(float zoom);
	virtual void scrollScreen(Vector2 translation);
	virtual void rotateScreen(float angle);
	virtual void setScreenSize(Coordinates screenSize);

	virtual void drawBitmap(std::string texture, Vector2 source_location,
	                        Vector2 source_size, Vector2 dest_location,
	                        Vector2 dest_size);
	virtual void drawBitmapTinted(std::string texture, Vector2 source_location,
                            Vector2 source_size, Vector2 dest_location,
                            Vector2 dest_size, Color color);

	virtual void drawRectangle(Vector2 pointA, Vector2 pointB, Color color, float thickness, Transformation transformation = Transformation::UI);
	virtual void drawRectangleFilled(Vector2 pointA, Vector2 pointB, Color color, Transformation transformation = Transformation::UI);

	virtual void drawText(std::string text, Vector2 location, Color color, Alignment alignment = Alignment::LEFT);

	virtual Coordinates getTextureSize(std::string texture);

	virtual void transformLocation(Transformation transformation, Vector2 &location);
	virtual void useTransformation(Transformation transformation);

	virtual void hsvToRgb(float hue, float saturation, float value, Color *color);

private:
	ALLEGRO_DISPLAY *display;
	ALLEGRO_FONT *font;

	ALLEGRO_TRANSFORM camera;
	ALLEGRO_TRANSFORM camera_inverse;
	ALLEGRO_TRANSFORM cameraUI;

	Coordinates screenSize;
	float screenZoom;
	Vector2 screenLocation;
	float screenRotation;

	std::map<std::string, ALLEGRO_BITMAP*> textures;

	void loadTexture(std::string);

	ALLEGRO_TRANSFORM* getTransformation(Transformation transformation);
	int getAlignment(IRenderer::Alignment alignment);
};

#endif
