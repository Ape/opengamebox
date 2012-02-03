#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <sstream>

#include "../widget.h"

class Game;

class InputBox : public Widget{
private:
	Game *game;
	void (Game::*send)(std::string);

	ALLEGRO_FONT *font;
	ALLEGRO_USTR *text;
	size_t inputLocation;
	unsigned char maxLength;

public:
	InputBox(Game *game, void (Game::*send)(std::string), Vector2 location, Vector2 size, ALLEGRO_FONT *font, unsigned char maxLen);
	~InputBox();

	virtual void draw(void);
	virtual bool onKey(ALLEGRO_KEYBOARD_EVENT keyboard);
	std::string getText();
	ALLEGRO_USTR* getTextUstr();
};

#endif
