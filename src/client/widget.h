// Copyright 2012 Lauri Niskanen
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

#ifndef WIDGET_H
#define WIDGET_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include "../vector2.h"
#include "../irenderer.h"

#include <vector>

class Widget {
protected:
	Vector2 location;
	Vector2 size;
	std::vector<Widget*> widgets;

public:
	Widget(Vector2 location, Vector2 size);
	virtual ~Widget(void);

	virtual void draw(IRenderer *renderer);
	virtual bool onKey(ALLEGRO_KEYBOARD_EVENT keyboard);

	void addWidget(Widget *widget);

	virtual void resize(Vector2 multiplier);
	virtual void resize(Vector2 location, Vector2 size);

	virtual Vector2 getLocation(void);
	virtual Vector2 getSize(void);
};

#endif
