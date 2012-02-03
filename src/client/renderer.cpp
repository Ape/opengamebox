#include "renderer.h"

Renderer::Renderer(){
	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

	// TODO: Do we want a system for preloading textures?
}

Renderer::~Renderer(){
	for (auto& texture : this->textures){
		al_destroy_bitmap(texture.second);
	}
}

void Renderer::loadTexture(std::string texture){
	if (this->textures[texture] == nullptr){
		std::string path = "gfx/" + texture;

		this->textures[texture] = al_load_bitmap(path.c_str());
	}
}

void Renderer::drawBitmap(std::string texture, Coordinates source_location, Coordinates source_size,
                          Coordinates dest_location, Coordinates dest_size){
	if (this->textures[texture] == nullptr){
		this->loadTexture(texture);
	}

	al_draw_scaled_bitmap(this->textures[texture], source_location.x, source_location.y, source_size.x, source_size.y,
	                      dest_location.x, dest_location.y, dest_size.x, dest_size.y, 0);
}

Coordinates Renderer::getTextureSize(std::string texture){
	if (this->textures[texture] == nullptr){
		this->loadTexture(texture);
	}

	return Coordinates(al_get_bitmap_width(this->textures[texture]), al_get_bitmap_height(this->textures[texture]));
}
