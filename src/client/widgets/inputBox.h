#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <sstream>

#include "../widget.h"

class Game;

class InputBox : public Widget{
public:
	InputBox(Game *game, void (Game::*send)(std::string), std::string caption, Vector2 location, float width, ALLEGRO_FONT *font, unsigned char maxLen);
	~InputBox();

	virtual void draw(IRenderer *renderer);
	virtual bool onKey(ALLEGRO_KEYBOARD_EVENT keyboard);
	std::string getText();

	ALLEGRO_USTR* getTextUstr();

private:
	Game *game;
	void (Game::*send)(std::string);

	std::string caption;
	ALLEGRO_FONT *font;
	ALLEGRO_USTR *text;
	size_t inputLocation;
	unsigned char maxLength;
};

#endif
