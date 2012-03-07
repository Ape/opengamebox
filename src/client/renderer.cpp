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

#include "renderer.h"

const float Renderer::PI = 3.141592653589f;

Renderer::Renderer(Coordinates screenSize, const int multisamplingSamples) {
	this->screenSize = screenSize;

	if (multisamplingSamples > 1) {
		al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
		al_set_new_display_option(ALLEGRO_SAMPLES, multisamplingSamples, ALLEGRO_SUGGEST);
	}

	// Initialize display
	this->display = al_create_display(this->screenSize.x, this->screenSize.y);
    if (! this->display) {
        std::cerr << "Failed to create a display!" << std::endl;
    }
    al_acknowledge_resize(this->display);

	// Initialize fonts
    al_init_font_addon();
    if (! al_init_ttf_addon()) {
        std::cerr << "Failed to initialize fonts!" << std::endl;
    }

    this->font = al_load_ttf_font("res/LiberationSans-Regular.ttf", 16, 0);

    // Initialize additional rendering libraries
    if (! al_init_image_addon()) {
        std::cerr << "Failed to initialize image libraries!" << std::endl;
    }

    if (!al_init_primitives_addon()) {
        std::cerr << "Failed to initialize primitives addon!" << std::endl;
    }

	this->screenZoom = 2.0f;
	this->screenLocation = Vector2(0.0f, 0.0f);
	this->screenRotation = 0.0f;

	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

	this->updateTransformations();
}

Renderer::~Renderer() {
	for (auto& texture : this->textures) {
		if (texture.second != nullptr) {
			al_destroy_bitmap(texture.second);
			texture.second = nullptr;
		}
	}

	al_destroy_font(this->font);
	al_destroy_display(this->display);
}

ALLEGRO_DISPLAY* Renderer::getDisplay() const {
	return this->display;
}

ALLEGRO_FONT* Renderer::getFont() const {
	return this->font;
}

Coordinates Renderer::getDisplaySize() const {
	return this->screenSize;
}

void Renderer::resize() {
	al_acknowledge_resize(this->display);

	this->setScreenSize(Coordinates(al_get_display_width(this->display), al_get_display_height(this->display)));
    this->updateTransformations();
}

void Renderer::setWindowTitle(std::string title, std::string icon) {
	this->loadTexture(icon);

	al_set_window_title(this->display, title.c_str());
	al_set_display_icon(this->display, this->textures[icon]);
}

void Renderer::updateTransformations() {
	al_identity_transform(&this->camera);
	al_rotate_transform(&this->camera, this->screenRotation);
	al_translate_transform(&this->camera, this->screenSize.x / this->screenZoom, this->screenSize.y / this->screenZoom);
	al_translate_transform(&this->camera, this->screenLocation.x, this->screenLocation.y);
	al_scale_transform(&this->camera, this->screenZoom / 2.0f, this->screenZoom / 2.0f);

	al_identity_transform(&this->camera_inverse);
	al_scale_transform(&this->camera_inverse, 2.0f / this->screenZoom, 2.0f / this->screenZoom);
	al_translate_transform(&this->camera_inverse, -this->screenLocation.x, -this->screenLocation.y);
	al_translate_transform(&this->camera_inverse, -this->screenSize.x / this->screenZoom, -this->screenSize.y / this->screenZoom);
	al_rotate_transform(&this->camera_inverse, -this->screenRotation);

	// TODO: Use al_invert_transform
	/*al_copy_transform(&this->camera, &this->camera_inverse);
	al_invert_transform(&this->camera_inverse);*/

	al_identity_transform(&this->cameraUI);
}

void Renderer::zoomScreen(float factor) {
	if (this->screenZoom * factor > 0.1f && this->screenZoom * factor < 30.0f)
	this->screenZoom *= factor;
}

void Renderer::scrollScreen(Vector2 translation) {
	this->screenLocation += translation * 2.0f / this->screenZoom;
}

void Renderer::rotateScreen(float angle) {
	this->screenRotation += angle;
}

void Renderer::setScreenSize(Coordinates screenSize) {
	this->screenSize = screenSize;
}

void Renderer::loadTexture(std::string texture) {
	if (this->textures[texture] == nullptr) {
		std::string path = texture + ".png";
		this->textures[texture] = al_load_bitmap(path.c_str());

		if (this->textures[texture] == nullptr) {
			std::string path = texture + ".jpg";
			this->textures[texture] = al_load_bitmap(path.c_str());

			if (this->textures[texture] == nullptr) {
				if (texture != "gfx/error") {
					std::cout << "Error: Texture " << texture << ".{png|jpg}" << " could not be loaded." << std::endl;
					this->loadTexture("gfx/error");
					this->textures[texture] = this->textures["gfx/error"];
				} else {
					std::cout << "Fatal: Could not load fallback texture." << std::endl;
				}
			}
		}
	}
}

void Renderer::drawBitmap(std::string texture, Vector2 source_location, Vector2 source_size,
                          Vector2 dest_location, Vector2 dest_size) {
	this->loadTexture(texture);

	al_draw_scaled_bitmap(this->textures[texture], source_location.x, source_location.y,
	                      source_size.x * this->getTextureSize(texture).x, source_size.y * this->getTextureSize(texture).y,
	                      dest_location.x, dest_location.y, dest_size.x, dest_size.y, 0);
}

void Renderer::drawBitmapTinted(std::string texture, Vector2 source_location, Vector2 source_size,
                                Vector2 dest_location, Vector2 dest_size, Color color) {
	this->loadTexture(texture);

	al_draw_tinted_scaled_bitmap(this->textures[texture], al_map_rgba_f(color.red, color.green, color.blue, color.alpha), source_location.x, source_location.y,
	                      source_size.x * this->getTextureSize(texture).x, source_size.y * this->getTextureSize(texture).y,
	                      dest_location.x, dest_location.y, dest_size.x, dest_size.y, 0);
}

void Renderer::drawRectangle(Vector2 pointA, Vector2 pointB, Color color, float thickness, Transformation transformation) {
	Vector2 pointAB = Vector2(pointA.x, pointB.y);
	Vector2 pointBA = Vector2(pointB.x, pointA.y);

	float thicknessFactor;
	if (transformation == Transformation::CAMERA) {
		thicknessFactor = 2.0f / this->screenZoom;
	} else {
		thicknessFactor = 1.0f;
	}

	al_hold_bitmap_drawing(false);
	al_draw_line(pointA.x, pointA.y, pointAB.x, pointAB.y, al_map_rgba_f(color.red, color.green, color.blue, color.alpha), thicknessFactor * thickness);
	al_draw_line(pointA.x, pointA.y, pointBA.x, pointBA.y, al_map_rgba_f(color.red, color.green, color.blue, color.alpha), thicknessFactor * thickness);
	al_draw_line(pointB.x, pointB.y, pointAB.x, pointAB.y, al_map_rgba_f(color.red, color.green, color.blue, color.alpha), thicknessFactor * thickness);
	al_draw_line(pointB.x, pointB.y, pointBA.x, pointBA.y, al_map_rgba_f(color.red, color.green, color.blue, color.alpha), thicknessFactor * thickness);
	al_hold_bitmap_drawing(true);
}

void Renderer::drawRectangleFilled(Vector2 pointA, Vector2 pointB, Color color, Transformation transformation) {
	al_hold_bitmap_drawing(false);
	al_draw_filled_rectangle(pointA.x, pointA.y, pointB.x, pointB.y, al_map_rgba_f(color.red, color.green, color.blue, color.alpha));
	al_hold_bitmap_drawing(true);
}

void Renderer::drawText(std::string text, Vector2 location, Alignment alignment) {
	size_t position = 0;
	size_t oldposition = 0;
	float drawposition = 0.0f;
	Color color = Color(1.0f, 1.0f, 1.0f);
	bool loop = true;

	do {
		position = text.find("^", oldposition);

		if (position != std::string::npos && position + 5 <= text.length()) {
			al_draw_text(this->font, al_map_rgba_f(color.red, color.green, color.blue, color.alpha), location.x + drawposition, location.y,
			             this->getAlignment(alignment), text.substr(oldposition, position - oldposition).c_str());
			drawposition += al_get_text_width(this->font, text.substr(oldposition, position - oldposition).c_str());

			color.red   = util::hexStringToInt(text.substr(position + 1, 1)) / 16.0f;
			color.green = util::hexStringToInt(text.substr(position + 2, 1)) / 16.0f;
			color.blue  = util::hexStringToInt(text.substr(position + 3, 1)) / 16.0f;
		} else {
			al_draw_text(this->font, al_map_rgba_f(color.red, color.green, color.blue, color.alpha), location.x + drawposition, location.y,
			             this->getAlignment(alignment), text.substr(oldposition).c_str());
			drawposition += al_get_text_width(this->font, text.substr(oldposition).c_str());
			loop = false;
		}

		oldposition = position + 4;

		if (oldposition >= text.length()) {
			oldposition = text.length() -1;
		}
	} while (loop);
}

Coordinates Renderer::getTextureSize(std::string texture) {
	this->loadTexture(texture);

	return Coordinates(al_get_bitmap_width(this->textures[texture]), al_get_bitmap_height(this->textures[texture]));
}

ALLEGRO_TRANSFORM* Renderer::getTransformation(Transformation transformation) {
	switch (transformation) {
		case CAMERA: {
			return &this->camera;
			break;
		}

		case CAMERA_INVERSE: {
			return &this->camera_inverse;
			break;
		}

		case UI: {
			return &this->cameraUI;
			break;
		}

		default: {
			return nullptr;
			break;
		}
	}
}

int Renderer::getAlignment(Alignment alignment) {
	switch (alignment) {
		case Alignment::LEFT: {
			return ALLEGRO_ALIGN_LEFT;
			break;
		}

		case Alignment::CENTER: {
			return ALLEGRO_ALIGN_CENTRE;
			break;
		}

		case Alignment::RIGHT: {
			return ALLEGRO_ALIGN_RIGHT;
			break;
		}

		default: {
			return ALLEGRO_ALIGN_LEFT;
			break;
		}
	}
}

void Renderer::transformLocation(Transformation transformation, Vector2 &location) {
	al_transform_coordinates(this->getTransformation(transformation), &location.x, &location.y);
}

void Renderer::useTransformation(Transformation transformation) {
	al_use_transform(this->getTransformation(transformation));
}

void Renderer::hsvToRgb(float hue, float saturation, float value, Color *color) {
	al_color_hsv_to_rgb(hue, saturation, value, &color->red, &color->green, &color->blue);
}
