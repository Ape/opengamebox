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

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "../widget.h"

class ProgressBar : public Widget {
public:
	ProgressBar(Vector2 location, Vector2 size, float progress);
	virtual ~ProgressBar();

	virtual void draw(IRenderer *renderer);

	void setProgress(float progress);

private:
	float progress;
};

#endif
