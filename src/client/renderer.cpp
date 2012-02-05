#include "renderer.h"

Renderer::Renderer(){
	this->screenZoom = 2.0f;
	this->screenLocation = Vector2(0.0f, 0.0f);
	this->screenRotation = 0.0f;

	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

	// TODO: Do we want a system for preloading textures?
}

Renderer::~Renderer(){
	for (auto& texture : this->textures){
		al_destroy_bitmap(texture.second);
	}
}

void Renderer::resize(Vector2 displaySize){
	al_identity_transform(&this->camera);
    al_translate_transform(&this->camera, displaySize.x / this->screenZoom, displaySize.y / this->screenZoom);
    al_scale_transform(&this->camera, this->screenZoom / 2.0f, this->screenZoom / 2.0f);
    al_translate_transform(&this->camera, this->screenLocation.x, this->screenLocation.y);

    al_identity_transform(&this->camera_inverse);
    al_translate_transform(&this->camera_inverse, -this->screenLocation.x, -this->screenLocation.y);
    al_scale_transform(&this->camera_inverse, 2.0f / this->screenZoom, 2.0f / this->screenZoom);
    al_translate_transform(&this->camera_inverse, -displaySize.x / this->screenZoom, -displaySize.y / this->screenZoom);

    // TODO: Use al_invert_transform
    /*al_copy_transform(&this->camera, &this->camera_inverse);
    al_invert_transform(&this->camera_inverse);*/

    al_identity_transform(&this->cameraUI);
}

void Renderer::mulScreenZoom(float zoom){
	if (this->screenZoom * zoom > 0.1f && this->screenZoom * zoom < 30.0f)
	this->screenZoom *= zoom;
}

void Renderer::addScreenLocation(Vector2 location){
	this->screenLocation += location;
}

void Renderer::loadTexture(std::string texture){
	if (this->textures[texture] == nullptr){
		std::string path = "gfx/" + texture;

		this->textures[texture] = al_load_bitmap(path.c_str());
	}
}

void Renderer::drawBitmap(std::string texture, Vector2 source_location, Vector2 source_size,
                          Vector2 dest_location, Vector2 dest_size){
	if (this->textures[texture] == nullptr){
		this->loadTexture(texture);
	}

	al_draw_scaled_bitmap(this->textures[texture], source_location.x, source_location.y,
	                      source_size.x * this->getTextureSize(texture).x, source_size.y * this->getTextureSize(texture).y,
	                      dest_location.x, dest_location.y, dest_size.x, dest_size.y, 0);
}

void Renderer::drawBitmapTinted(std::string texture, Vector2 source_location, Vector2 source_size,
                          Vector2 dest_location, Vector2 dest_size, float r, float g, float b, float alpha){
	if (this->textures[texture] == nullptr){
		this->loadTexture(texture);
	}

	al_draw_tinted_scaled_bitmap(this->textures[texture], al_map_rgba_f(r, g, b, alpha), source_location.x, source_location.y,
	                      source_size.x * this->getTextureSize(texture).x, source_size.y * this->getTextureSize(texture).y,
	                      dest_location.x, dest_location.y, dest_size.x, dest_size.y, 0);
}

void Renderer::drawRectangle(Vector2 pointA, Vector2 pointB, float r, float g, float b, float alpha, float thickness){
	al_draw_rectangle(pointA.x, pointA.y, pointB.x, pointB.y, al_map_rgba_f(r, g, b, alpha), thickness);
}

Coordinates Renderer::getTextureSize(std::string texture){
	if (this->textures[texture] == nullptr){
		this->loadTexture(texture);
	}

	return Coordinates(al_get_bitmap_width(this->textures[texture]), al_get_bitmap_height(this->textures[texture]));
}

ALLEGRO_TRANSFORM* Renderer::getTransformation(Transformation transformation){
	switch (transformation){
		case CAMERA:{
			return &this->camera;
			break;
		}

		case CAMERA_INVERSE:{
			return &this->camera_inverse;
			break;
		}

		case UI:{
			return &this->cameraUI;
			break;
		}

		default:{
			return nullptr;
			break;
		}
	}
}

void Renderer::transformLocation(Transformation transformation, Vector2 &location){
	al_transform_coordinates(this->getTransformation(transformation), &location.x, &location.y);
}

void Renderer::useTransform(Transformation transformation){
	al_use_transform(this->getTransformation(transformation));
}
