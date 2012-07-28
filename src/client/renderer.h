// Copyright 2012 Lauri Niskanen
// Copyright 2012 Antti Aalto
// Copyright 2012 Markus Mattinen
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

#ifndef RENDERER_H
#define RENDERER_H

#include <map>
#include <string>
#include <iostream>
#include <stdio.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>

#include "../irenderer.h"
#include "../utils.h"
#include "../vector2.h"
#include "../color.h"

class Renderer : public IRenderer {
public:
	Renderer(Coordinates screenSize, const int multisamplingSamples);
	virtual ~Renderer(void);

	ALLEGRO_DISPLAY* getDisplay(void) const;
	ALLEGRO_FONT* getFont(void) const;

	virtual Coordinates getDisplaySize(void) const;
	virtual void resize(void);
	virtual void setWindowTitle(std::string title, std::string icon);
	virtual void updateTransformations(void);

	virtual void zoomScreen(float zoom);
	virtual void scrollScreen(Vector2 translation);
	virtual void rotateScreen(float angle);
	virtual void setScreenSize(Coordinates screenSize);

	virtual void drawBitmap(std::string texture, Vector2 dest_location, Vector2 dest_size, float angle = 0);
	virtual void drawBitmapTinted(std::string texture, Vector2 dest_location, Vector2 dest_size, Color color, float angle = 0);

	virtual void drawLine(Vector2 pointA, Vector2 pointB, Color color, float thickness, Transformation transformation = Transformation::UI);

	virtual void drawRectangle(Vector2 pointA, Vector2 pointB, Color color, float thickness, Transformation transformation = Transformation::UI);
	virtual void drawRectangleFilled(Vector2 pointA, Vector2 pointB, Color color, Transformation transformation = Transformation::UI);

	virtual void drawCircle(Vector2 location, float radius, Color color, float thickness, Transformation transformation = Transformation::UI);
	virtual void drawCircleFilled(Vector2 location, float radius, Color color, Transformation transformation = Transformation::UI);

	virtual void drawText(std::string text, Vector2 location, Alignment alignment = Alignment::LEFT);

	virtual Coordinates getTextureSize(std::string texture);

	virtual void transformLocation(Transformation transformation, Vector2 &location);
	virtual void useTransformation(Transformation transformation);

	virtual void hsvToRgb(float hue, float saturation, float value, Color *color);

	void initRenderFont(void);

private:
	ALLEGRO_DISPLAY *display;
	ALLEGRO_FONT *font;
	ALLEGRO_FONT* rendFont;

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

	float getThicknessFactor(Transformation transformation);
};

#endif
