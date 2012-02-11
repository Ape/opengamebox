#ifndef WIDGET_H
#define WIDGET_H

#include "../vector2.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include "../irenderer.h"

class Widget{
protected:
	Vector2 location;
	Vector2 size;

public:
	Widget(Vector2 location, Vector2 size);
	virtual void draw(IRenderer *renderer);
	virtual bool onKey(ALLEGRO_KEYBOARD_EVENT keyboard);
};

#endif
