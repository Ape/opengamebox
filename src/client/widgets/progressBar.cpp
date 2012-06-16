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

#include "progressBar.h"
#include <iostream>

ProgressBar::ProgressBar(Vector2 location, Vector2 size, float progress)
: Widget(location, size),
  progress(progress) {}

ProgressBar::~ProgressBar() {}

void ProgressBar::draw(IRenderer *renderer) {
	renderer->drawRectangleFilled(this->location, this->location + Vector2(this->progress * this->size.x, this->size.y), Color(0.15f, 0.85f, 0.15f));
	renderer->drawRectangleFilled(this->location + Vector2(this->progress * this->size.x, 0.0f), this->location + this->size, Color(0.15f, 0.15f, 0.15f));
}

void ProgressBar::setProgress(float progress) {
	this->progress = progress;
}
